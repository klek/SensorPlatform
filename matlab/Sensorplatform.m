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

% The message struct
handles.message.head.type = char();
handles.message.head.nr = uint16(0);
handles.message.head.size = uint16(0);
%handles.message.head.size2 = 0;
handles.message.payload = 0;
handles.message.waiting = 0;

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
hSerial.BytesAvailableFcn = {@serialConnectEventHandler, handles};

% Attach it to handles
handles.serial = hSerial;

% Construct the UI
% Create the figure window
handles.figure = figure(10);
handles.figure.Visible = 'off';
handles.figure.Name = 'Sensorplatform';
handles.figure.Position = [360,500,750,585];
%figure('Name', 'Sensorplatform', 'Visible', 'off', 'Position', [360,500,750,585]);

% Initialize the timer
handles.timer = timer('ExecutionMode', 'fixedRate',...
                'Period', 0.5,...
                'TimerFcn', {@updatePlot, handles});


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
handles.axis = axes('Units', 'pixels', 'Position', [50,50,500,485]);

% Add a menu to choose which data that should be used if subplot is used

% Initialize the UI by setting normalized components
handles.figure.Units = 'normalized';
handles.serialButton.Units = 'normalized';
handles.plot.Units = 'normalized';
handles.axis.Units = 'normalized';

% Center the GUI
movegui(handles.figure, 'center');

% Show the window
handles.figure.Visible = 'on';

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
function updatePlot(src,event,handles)
    % We came here cause timer put us here
    fprintf('Well, shit works?\n');
    
    % Check if there is new data
    if ( handles.message.waiting == 1 )
        % Do nothing xD
        fprintf('The data received %f\n', handles.message.payload);
    end
    % Grab new data from handle
    %plotSpectrum(handles.
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

function serialConnectEventHandler(src, event, handles)
    bytes = get(src, 'BytesAvailable');
    out = 0;
    % Should only reach here if we have a full message
    if ( bytes > 0 )
        out = fread(src, (256 + 6), 'uint8');
        %data = fscanf(src);
        %fwrite(outFile, data);
        %out = fscanf(src);
        fprintf('The value of out %s\n', out);
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
                nr = uint16( (out(3)*2^8) + out(4) );
                if ( (handles.message.head.nr + 1) == nr )
                    handles.message.head.nr = nr;
                    % Do I need to check the size of payload too?
                    
                    % Check if this is last packet
                    morePackets = handles.message.head.size - 256*nr;
                    if ( morePackets <= 256 )
                        handles.message.waiting = 0;
                    end
                    
                    % Now grab morePackets amount of data
                    % Nr cannot be equal to 1 here, it should always be
                    % bigger
                    for i = 1:(morePackets/4);
                        handles.message.payload(i*(256*(nr-1)/4)) = double( out(7*i)*2^24 + out(8*i)*2^16 + out(9*i)*2^8 + out(10*i) );
                    end
                    
                    % Do we need to do something more?
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
            handles.message.head.nr = uint16( (out(3)*2^8) + out(4) );
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
                    handles.message.payload(i) = double( out(7*i)*2^24 + out(8*i)*2^16 + out(9*i)*2^8 + out(10*i) );
                end
            else
                % Grab the payload which is 4-byte floats
                % Starting at slot 7
                for i = 1:(handles.message.head.size/4);
                    handles.message.payload(i) = double( out(7*i)*2^24 + out(8*i)*2^16 + out(9*i)*2^8 + out(10*i) );
                end
            end
            
            % Do we need to do anything more?
        end
    end
    
    % Anything left?
    bytes = get(src, 'BytesAvailable');
    if ( bytes > 0 )
        fprintf('%d bytes still available\n', bytes);
    else
        fprintf('%d bytes still available\n', bytes);
    end
    
    % Save data to UserData element of figure
    %handles.hFigure.UserData = dataAvailable;
    
%    if ( dataAvailable == 1 )
        % Convert the data        
%    end
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
