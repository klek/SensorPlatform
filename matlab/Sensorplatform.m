function Sensorplatform
% A GUI for representing the data receive from the Sensorplatform 
% implemented on a Nucleo-STM32F767i.
% This program displays the received FFT-spectrum from the device
% aswell as the two highest peaks in the spectrum.
%
% Future update: 
% Add possibility to show a subplot, with reference data.

% Define the data files
% For save data
outputFile = fopen('../testing/output/output.txt', 'wt');
% Vector to store data we received in
data = 0;
% Flag to indicate if data is available
dataAvailable = 0;

% Construct the UI
% Create the figure window
hFigure = figure(10);
hFigure.Visible = 'off';
hFigure.Name = 'Sensorplatform';
hFigure.Position = [360,500,750,585];
%figure('Name', 'Sensorplatform', 'Visible', 'off', 'Position', [360,500,750,585]);

% Get the handles
handles = guihandles(hFigure);

% The message struct
handles.message.head.type = char();
handles.message.head.nr = uint16(0);
handles.message.head.size = uint16(0);
%handles.message.head.size2 = 0;
handles.message.payload = 0;
handles.message.waiting = 0;
handles.message.prevIndex = 0;
handles.message.newData = 0;

% The fft struct
handles.fft.fftLength = 2048;
handles.fft.decimation = 1;
handles.fft.sampleRate = 2048;

% Assign the output file to handles
handles.outFile = outputFile;

% Then serial connection configuration
hSerial = serial('/dev/ttyACM0');   % Maybe add this as a config in UI
hSerial.BaudRate = 115200;%9600;
hSerial.Parity = 'odd';
hSerial.StopBits = 1;
hSerial.DataBits = 7;
hSerial.Timeout = 10;
hSerial.InputBufferSize = 2048;     % This should be larger than needed

% Configure the callback
hSerial.BytesAvailableFcnMode = 'byte';%'terminator';
hSerial.BytesAvailableFcnCount = (256+6);
hSerial.BytesAvailableFcn = {@serialConnectEventHandler, hFigure};

% Attach it to handles
handles.serial = hSerial;

% Initialize the timer
handles.timer = timer('ExecutionMode', 'fixedRate',...
                'Period', 0.5,...
                'TimerFcn', {@updatePlot, hFigure});


% Add button for connecting to the serial
handles.serialButton = uicontrol('Style', 'pushbutton', ...
                    'String', 'Serial Connect', ...
                    'Position', [590,520,150,30], ...
                    'Callback', {@serialButton_Callback,handles});

% Add button for subplot
handles.plot = uicontrol('Style', 'pushbutton', ...
                    'String', 'Plot', ...
                    'Position', [590,480,150,30], ...
                    'Callback', {@plotButtonCallback, handles});

% Adding a drop-down menu, to choose data from
%hMenu = uicontrol('Style', 'dropdownmenu', ...
%                    'Position', [

% Add axis for the plot
handles.axis = axes('Units', 'pixels', 'Position', [60,50,500,485]);

% Add a menu to choose which data that should be used if subplot is used

% Initialize the UI by setting normalized components
handles.figure.Units = 'normalized';
handles.serialButton.Units = 'normalized';
handles.plot.Units = 'normalized';
handles.axis.Units = 'normalized';

% Center the GUI
movegui(hFigure, 'center');

% Show the window
hFigure.Visible = 'on';

% Store the figure handle in the handles struct
handles.figure = hFigure;

% Store all data
guidata(hFigure, handles);

% Start the timer
%start(handles.timer);

%while true
%    dataAvailable = handles.figure.UserData;
%    if ( dataAvailable == 1 )
%       % We have received new data
%       fprintf('Testing\n');
%    end
%    pause(1);
%    fprintf('%d dataAvailable\n', dataAvailable);
%end

end

% Callbacks
function updatePlot(src, event, hFigure)
    % We came here cause timer put us here
    fprintf('Well, shit works?\n');
    
    % Get the figure data
    handles = guidata(hFigure);
    
    % Check if there is new data
    if ( handles.message.newData == 1 )
        % Reset the newData
        handles.message.newData = 0;
        %fprintf('The data received %f\n', handles.message.prevIndex);
        
        % Create the frequency vector
        sigLen = length(handles.message.payload);
        resolution = (handles.fft.sampleRate / handles.fft.decimation) / handles.fft.fftLength;
        f = 0:resolution:(sigLen - 1);
        
        % Plot the new data
        %plotSpectrum(handles.message.payload, handles.plot.fftLength, (handles.plot.sampleRate/handles.plot.decimation));
        hPlot = plot(handles.axis, f, handles.message.payload);
        title('Frequency spectrum of vital signs');
        xlabel('Frequency [Hz]');
        ylabel('Magnitude [V]');
        
        % Mark the peaks with data tips
        makedatatip(hPlot, [41 51])
    end
    
    % Make sure we update the structure
    guidata(handles.figure, handles);
end

function serialButton_Callback(src,event,handles)
    % Check if connection is open
    %if ( obj.Status == 'closed' )
    if ( strcmp(handles.serial.Status, 'closed') )
        % Open the serial connection
        fopen(handles.serial);
        % Check again if connection is now open
        if ( strcmp(handles.serial.Status, 'open') )
            % Change text for button
            src.String = 'Serial disconnect';
        end
    else
        % Connection is already open
        % lets close it
        fclose(handles.serial);
        % Change text for button
        src.String = 'Serial connect';
    end
end

function plotButtonCallback(src, event, handles)
    % User wants us to plot
    % Get the state of the button
    buttonState = src.String;
    if ( strcmp(buttonState, 'Plot') )
        % Start timer, to poll for data
        start(handles.timer);
        % Change string
        src.String = 'Stop plot';
    else
        % Stop timer
        stop(handles.timer);
        % Change string
        src.String = 'Plot';
    end
end

function serialConnectEventHandler(src, event, hFigure)
    bytes = get(src, 'BytesAvailable');
    out = 0;
    
    % Get the handles
    handles = guidata(hFigure);
    %gcbo
    
    % Should only reach here if we have a full message
    if ( bytes > 0 )
        out = fread(src, (256 + 6), 'uint8');
        %data = fscanf(src);
        %fwrite(outFile, data);
        %out = fscanf(src);
        %fprintf('The value of out %s\n', out);
        %bytes = get(src, 'BytesAvailable');
    end

    % Was this the first time we got data?
    % Check the message
    if ( handles.message.waiting == 1 )
        % We should append this packet to already existing data
        type = char(out(1));
        if ( strcmp(handles.message.head.type, type) )
            % Check which type it is
            if ( strcmp(type, 'A') )
                % We have to deal with this
            elseif ( strcmp(type, 'B') )
                % Data type message
                nr = uint16( (out(3)*2^8) + (out(4)) );
                fprintf('Nr = %16s\n',dec2bin(nr));
                fprintf('Previous nr = %16s\n',dec2bin(handles.message.head.nr));
                if ( (handles.message.head.nr + 1) == nr )
                    handles.message.head.nr = nr;
                    % Do I need to check the size of payload too?
                    
                    % Check if this is last packet
                    morePackets = handles.message.head.size - 256*(nr-1)
                    if ( morePackets <= 256 )
                        handles.message.waiting = 0;
                        grabData = morePackets;
                    else
                        grabData = 256;
                    end
                    
                    % Now grab more amount of data
                    % Nr cannot be equal to 1 here, it should always be
                    % bigger
                    prevIndex = handles.message.prevIndex;
                    for i = 1:(grabData/4);
                        slot = (i - 1) * 4;
                        tmp1 = uint32(bitshift(out(7+slot),24));
                        tmp2 = uint32(bitshift(out(8+slot),16));
                        tmp3 = uint32(bitshift(out(9+slot),8));
                        tmp4 = uint32(out(10+slot));
                        handles.message.payload(i+prevIndex) = typecast( (tmp1 + tmp2 + tmp3 + tmp4), 'single');
                        %handles.message.payload(i+prevIndex) = double( out(7+slot)*2^24 + out(8+slot)*2^16 + out(9+slot)*2^8 + out(10+slot) );
                    end
                    handles.message.prevIndex = prevIndex + i;
                    % Do we need to do something more?
                    
                    % Was this the last data?
                    if ( handles.message.waiting == 0 )
                        % Indicate an update is needed
                        handles.message.newData = 1;
                    end
                else
                    % Well apparently we missed a packet??
                    fprintf('Missed a packet??\n');
                end
            end
        else
            % We have a problem :/
            fprintf('Packet types not equal...\n');
        end
    else
        % This is a new packet
        % Check the first slot for type
        handles.message.head.type = char(out(1));
        if ( strcmp(handles.message.head.type, 'A') )
            % This is a debug message in text form
            % The payload contains chars in ascii
        elseif ( strcmp(handles.message.head.type, 'B') )
            % This is a data message
            % The payload contains uint8 that should be casted to 4-byte floats
            % Grab the packet nr
            handles.message.head.nr = uint16( (out(3)*2^8) + (out(4)) );
            % Check if there is more than one packet here
            if ( handles.message.head.nr > 0 )
                % Set the waiting flag
                handles.message.waiting = 1;
            end
            % Grab how much data we should expect, in bytes
            handles.message.head.size = uint16( (out(5)*2^8) + out(6) );

            if ( handles.message.head.size >= 256 )
                % Grab the payload which is 4-byte floats
                % Starting at slot 7
                for i = 1:(256/4);
                    slot = (i - 1) * 4;
                    % Creating a byte array
                    %A = uint8([ out(7+slot) out(8+slot) out(9+slot) out(10+slot) ]);
                    tmp1 = uint32(bitshift(out(7+slot),24));
                    tmp2 = uint32(bitshift(out(8+slot),16));
                    tmp3 = uint32(bitshift(out(9+slot),8));
                    tmp4 = uint32(out(10+slot));
                    handles.message.payload(i) = typecast( (tmp1 + tmp2 + tmp3 + tmp4), 'single');
                    %handles.message.payload(i) = double( out(7+slot)*2^24 + out(8+slot)*2^16 + out(9+slot)*2^8 + out(10+slot) );
                end
                handles.message.prevIndex = i;
            else
                % Grab the payload which is 4-byte floats
                % Starting at slot 7
                for i = 1:(handles.message.head.size/4);
                    slot = (i - 1) * 4;
                    tmp1 = uint32(bitshift(out(7+slot),24));
                    tmp2 = uint32(bitshift(out(8+slot),16));
                    tmp3 = uint32(bitshift(out(9+slot),8));
                    tmp4 = uint32(out(10+slot));
                    handles.message.payload(i) = typecast( (tmp1 + tmp2 + tmp3 + tmp4), 'single');
                    %handles.message.payload(i) = double( out(7+slot)*2^24 + out(8+slot)*2^16 + out(9+slot)*2^8 + out(10+slot) );
                end
            end
            
            % Do we need to do anything more?
        end
    end
    
    % print values
    handles.message.payload;
    handles.message.head.nr
    handles.message.prevIndex
    
    % Store the data
    guidata(handles.figure, handles);
    
    % Anything left?
    bytes = get(src, 'BytesAvailable');
    fprintf('%d bytes still available\n', bytes);
end
%end

function unpackUART(data)
% This function should unpack the data received
% into its header and payload component

% Header is total of 4 bytes long
% Containing:
% Type NULL Size Size

% Payload is the data in size long
% This should match up the length of the remaining vector in bytes

end
