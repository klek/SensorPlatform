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
%data = 0;
% Flag to indicate if data is available
%dataAvailable = 0;

% Construct the UI
% Create the figure window
hFigure = figure(10);
hFigure.Visible = 'off';
hFigure.Name = 'Sensorplatform';
hFigure.Position = [360,500,750,585];
hFigure.MenuBar = 'none';
hFigure.ToolBar = 'none';
%figure('Name', 'Sensorplatform', 'Visible', 'off', 'Position', [360,500,750,585]);

% Get the handles
handles = guihandles(hFigure);

% The message struct
handles.message.head.type = char();
handles.message.head.nr = uint16(0);
handles.message.head.size = uint16(0);
%handles.message.head.size2 = 0;
handles.message.payload.spectrum = 0;
handles.message.payload.peakdata = 0;
handles.message.waiting = 0;
handles.message.prevIndex = 0;
handles.message.newData = 0;
handles.message.newPeakData = 0;

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

% Add button for plot
handles.plot = uicontrol('Style', 'pushbutton', ...
                    'String', 'Plot', ...
                    'Position', [590,480,150,30], ...
                    'Callback', {@plotButtonCallback, handles});

% Adding a drop-down menu, to choose decimation from
handles.decimationText = uicontrol('Style', 'text', ...
                    'String', 'Decimation', ...
                    'Position', [590,420,150,30]);
handles.decimationMenu = uicontrol('Style', 'popupmenu', ...
                    'String', {'1','2','4','8','16'}, ...
                    'Position', [590,400,150,30], ...
                    'Callback', {@decimationCallback, hFigure});

% Adding a text-box, to insert sample rate in
%handles.decimationText = uicontrol('Style', 'text', ...
%                    'String', 'Decimation', ...
%                    'Position', [590,440,150,30]);
%handles.decimationMenu = uicontrol('Style', 'popupmenu', ...
%                    'String', {'1','2','4','8','16'}, ...
%                    'Position', [590,400,150,30], ...
%                    'Callback', {@decimationCallback, hFigure});

                
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
function updatePlot(src, ~, hFigure)
    % We came here cause timer put us here
    %fprintf('Well, shit works?\n');
    
    % Local variable to indicate if datatips should be drawn
    drawDatatip = 0;
    
    % Get the figure data
    handles = guidata(hFigure);
    
    % Check if there is new data
    if ( handles.message.newData == 1 )
        % Reset the newData
        handles.message.newData = 0;
        %fprintf('The data received %f\n', handles.message.prevIndex);
        
        % Create the frequency vector
        sigLen = length(handles.message.payload.spectrum);
        resolution = (handles.fft.sampleRate / handles.fft.decimation) / handles.fft.fftLength;
        f = 0:resolution:((sigLen - 1)*resolution);
        
        % Plot the new data
        %plotSpectrum(handles.message.payload.spectrum, handles.plot.fftLength, (handles.plot.sampleRate/handles.plot.decimation));
        hPlot = plot(handles.axis, f, handles.message.payload.spectrum);
        handles.plot = hPlot;
        title('Frequency spectrum of vital signs');
        xlabel('Frequency [Hz]');
        ylabel('Magnitude [V]');
        %axis([-100 1050 0 (max(handles.message.payload.spectrum) + 0.1)])
        
        % Indicate new datatips should be drawn
        drawDatatip = 1;
        
        % We can also remove the data now
        handles.message.payload.spectrum = 0;
    end
    
    % Mark the peaks with data tips
    %makedatatip(hPlot, [41 51])
    if ( handles.message.newPeakData == 1 )
        handles.message.newPeakData = 0;
        % Mark the peaks with data tips specified from the MCU
        % Indexes of peaks is every second slots
        peakIndex = handles.message.payload.peakdata(2:2:end);
        % POTENTIAL PROBLEM HERE...THIRD AND FIFTH SLOT IS HALFED IN
        % VALUE???

        % Add 1 to each element, to match the array indexes between C and matlab
        peakIndex = peakIndex + 1;

        % Grab the peak values
        peakVal = handles.message.payload.peakdata(1:2:end);
        % POTENTIAL PROBLEM HERE...FOURTH SLOT IS HALFED IN VALUE???

        % Which peaks should be used?
        % Thresholds? Should be done in MCU?
        
        % Should we draw datatips?
        if ( drawDatatip == 1 )
            % Only mark the two highest peaks
            peakIndex = [peakIndex(1) peakIndex(2)];
            % Mark the peaks in the plot
            %makedatatip(hPlot, peakIndex);
        end
        
        % Remove the peak data
        handles.message.payload.peakdata = 0;
    end
    
    
    % Make sure we update the structure
    guidata(handles.figure, handles);
end

function serialButton_Callback(src,~,handles)
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

function plotButtonCallback(src, ~, handles)
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

function decimationCallback(src,~,hFigure)
    % Grab the struct
    handles = guidata(hFigure);
    % Grab the event
    str = get(src, 'String');
    val = get(src, 'Value');
    % Decimation is per standard 1
    dec = 1;
    switch str{val}
        case '1'
            dec = 1;
        case '2'
            dec = 2;
        case '4'
            dec = 4;
        case '8'
            dec = 8;
        case '16'
            dec = 16;
    end
    handles.fft.decimation = dec;
    
    % Update the struct
    guidata(hFigure, handles);
end

function serialConnectEventHandler(src, ~, hFigure)
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
                % This is a debug message in text form
                % The payload contains chars in ascii
                % We write this data to console and to logfile
                % Grab packet nr
                nr = uint16( (out(3)*2^8) + (out(4)) );
                %fprintf('Nr = %16s\n',dec2bin(nr));
                %fprintf('Previous nr = %16s\n',dec2bin(handles.message.head.nr));
                if ( (handles.message.head.nr + 1) == nr )
                    handles.message.head.nr = nr;
                    % Do I need to check the size of payload too?
                    
                    % Check if this is last packet
                    morePackets = handles.message.head.size - 256*(nr-1);
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
                    for i = 1:(grabData);
                        slot = (i - 1);
                        handles.message.payload.text(i+prevIndex) = char(out(7+slot));
                    end
                    handles.message.prevIndex = prevIndex + i;
                    % Get the lenght of the buffer
                    bufLen = length(handles.message.payload.text);
                    % Fill rest of the buffer with whitespace
                    for i = (handles.message.prevIndex):bufLen;
                        handles.message.payload.text(i) = char(' ');
                    end
                    % Do we need to do something more?
                    
                    % If there is no more message
                    % Print it out to console and to file
                    if ( handles.message.waiting == 0 )
                        fprintf('%s\n',handles.message.payload.text);
                        % Maybe add timestamp here in future
                        fprintf(handles.outFile, '%s\n', handles.message.payload.text);
                    end
                else
                    % Well apparently we missed a packet??
                    fprintf('Missed a packet??\n');
                end

            elseif ( strcmp(type, 'B') )
                % Frequency spectrum data message
                nr = uint16( (out(3)*2^8) + (out(4)) );
                %fprintf('Nr = %16s\n',dec2bin(nr));
                %fprintf('Previous nr = %16s\n',dec2bin(handles.message.head.nr));
                if ( (handles.message.head.nr + 1) == nr )
                    handles.message.head.nr = nr;
                    % Do I need to check the size of payload too?
                    
                    % Check if this is last packet
                    morePackets = handles.message.head.size - 256*(nr-1);
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
                        handles.message.payload.spectrum(i+prevIndex) = unpackFloat([out(7+slot) out(8+slot) out(9+slot) out(10+slot)]);
                    end
                    handles.message.prevIndex = prevIndex + i;
                    % Do we need to do something more?
                    
                    % Was this the last data?
                    if ( handles.message.waiting == 0 )
                        % Indicate an update is needed
                        handles.message.newData = 1;
                        handles.message.prevIndex = 0;
                    end
                else
                    % Well apparently we missed a packet??
                    fprintf('Missed a packet??\n');
                end
            elseif ( strcmp(type, 'C') )
                % This is a peakData message
                % Frequency spectrum data message
                nr = uint16( (out(3)*2^8) + (out(4)) );
                %fprintf('Nr = %16s\n',dec2bin(nr));
                %fprintf('Previous nr = %16s\n',dec2bin(handles.message.head.nr));
                if ( (handles.message.head.nr + 1) == nr )
                    handles.message.head.nr = nr;
                    % Do I need to check the size of payload too?
                    
                    % Check if this is last packet
                    morePackets = handles.message.head.size - 256*(nr-1);
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
                        handles.message.payload.peakData(i+prevIndex) = unpackFloat([out(7+slot) out(8+slot) out(9+slot) out(10+slot)]);
                    end
                    handles.message.prevIndex = prevIndex + i;
                    % Do we need to do something more?
                    
                    % Was this the last data?
                    if ( handles.message.waiting == 0 )
                        % Indicate an update is needed
                        handles.message.newPeakData = 1;
                        handles.message.prevIndex = 0;
                    end
                else
                    % Well apparently we missed a packet??
                    fprintf('Missed a packet??\n');
                end
            end
        else
            % We have a problem :/
            fprintf('Packet types not equal...\n');
            % Set waiting variable to zero
            handles.message.waiting = 0;
        end
    else
        % This is a new packet
        % Check the first slot for type
        handles.message.head.type = char(out(1));
        if ( strcmp(handles.message.head.type, 'A') )
            % This is a debug message in text form
            % The payload contains chars in ascii
            % We write this data to console and to logfile
            % Grab packet nr
            handles.message.head.nr = uint16( (out(3)*2^8) + (out(4)) );
            % Check if there is more than one packet here
            if ( handles.message.head.nr > 0 )
                % Set the waiting flag
                handles.message.waiting = 1;
            end
            % Grab how much data we should expect, in bytes
            handles.message.head.size = uint16( (out(5)*2^8) + out(6) );
            
            if ( handles.message.head.size >= 256 )
                % Grab the payload which is chars
                % Starting at slot 7
                for i = 1:(256);
                    slot = (i - 1);
                    handles.message.payload.text(i) = char(out(7+slot));
                end
                handles.message.prevIndex = i;
            else
                % Grab the payload which is 4-byte floats
                % Starting at slot 7
                for i = 1:(handles.message.head.size);
                    slot = (i - 1);
                    handles.message.payload.text(i) = char(out(7+slot));
                end
                % Get the lenght of the buffer
                bufLen = length(handles.message.payload.text);
                % Fill rest of the buffer with whitespace
                for i = (handles.message.head.size):bufLen;
                    handles.message.payload.text(i) = char(' ');
                end
            end
            
            % If there is no more message
            % Print it out to console and to file
            if ( handles.message.waiting == 0 )
                fprintf('%s\n',handles.message.payload.text);
                % Maybe add timestamp here in future
                fprintf(handles.outFile, '%s\n', handles.message.payload.text);
            end
            
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
                    handles.message.payload.spectrum(i) = unpackFloat([out(7+slot) out(8+slot) out(9+slot) out(10+slot)]);
                end
                handles.message.prevIndex = i;
            else
                % Grab the payload which is 4-byte floats
                % Starting at slot 7
                for i = 1:(handles.message.head.size/4);
                    slot = (i - 1) * 4;
                    handles.message.payload.spectrum(i) = unpackFloat([out(7+slot) out(8+slot) out(9+slot) out(10+slot)]);
                end
            end
            
            % Do we need to do anything more?
        elseif ( strcmp(handles.message.head.type, 'C') )
            % This is a peakData message
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
                    handles.message.payload.peakdata(i) = unpackFloat([out(7+slot) out(8+slot) out(9+slot) out(10+slot)]);
                end
                handles.message.prevIndex = i;
            else
                % Grab the payload which is 4-byte floats
                % Starting at slot 7
                for i = 1:(handles.message.head.size/4);
                    slot = (i - 1) * 4;
                    handles.message.payload.peakdata(i) = unpackFloat([out(7+slot) out(8+slot) out(9+slot) out(10+slot)]);
                end
            end
            
            % Indicate we have new peakData
            if ( handles.message.waiting == 0 )
                % Indicate an update is needed
                handles.message.newPeakData = 1;
            end
        end
    end
    
    % print values
    %handles.message.payload.spectrum
    %handles.message.head.nr
    %handles.message.prevIndex
    
    % Store the data
    guidata(handles.figure, handles);
    
    % Anything left?
    %bytes = get(src, 'BytesAvailable');
    %fprintf('%d bytes still available\n', bytes);
end
%end

function res = unpackFloat(data)
% This function should unpack the incoming data and return it as a float
% Data is expected to be an array of bytes in BIG ENDIAN FORMAT

% Cast the array to uint8
data = uint8(data);
% Get the size of data
len = length(data);

% Length is either 4 or 8
if ( len == 4 )
    % Add the result together
    res = bitshift(uint32(data(1)),24) + bitshift(uint32(data(2)),16) + bitshift(uint32(data(3)),8) + uint32(data(4));
    % Change the type to single precision float
    res = typecast(res, 'single');
elseif (len == 8 )
    % Add the result together
    res_high = bitshift(uint64(data(1)),56) + bitshift(uint64(data(2)),48) + bitshift(uint64(data(3)),40) + bitshift(uint64(data(4)),32);
    res_low = bitshift(uint64(data(5)),24) + bitshift(uint64(data(6)),16) + bitshift(uint64(data(7)),8) + uint64(data(8));
    res = res_high + res_low;
    % Change the type to double precision float
    res = typecast(res, 'double');
else
    % Something is wrong
    fprintf('Unrecognized format...input needs to be either 4 or 8 bytes long\n');
    res = 0;
end

end
