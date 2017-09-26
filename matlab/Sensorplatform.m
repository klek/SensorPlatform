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


% Then serial connection configuration
hSerial = serial('/dev/ttyACM0');   % Maybe add this as a config in UI
hSerial.BaudRate = 9600;
hSerial.Parity = 'odd';
hSerial.StopBits = 1;
hSerial.DataBits = 7;
hSerial.Timeout = 20;

% Configure the callback
%hSerial.BytesAvailableFcnMode = 'byte';
%hSerial.BytesAvailableFcnCount = 1;
%hSerial.BytesAvailableFcn = {@serialConnectEventHandler, outputFile};


% Construct the UI
% Create the figure window
hFigure = figure(10);
hFigure.Visible = 'off';
hFigure.Name = 'Sensorplatform';
hFigure.Position = [360,500,750,585];
%figure('Name', 'Sensorplatform', 'Visible', 'off', 'Position', [360,500,750,585]);

% Add button for connecting to the serial
hSerialButton = uicontrol('Style', 'pushbutton', ...
                    'String', 'Serial Connect', ...
                    'Position', [590,520,150,30], ...
                    'Callback', {@serialButton_Callback,hSerial});

% Add button for subplot
hPlot = uicontrol('Style', 'pushbutton', ...
                    'String', 'Plot', ...
                    'Position', [590,480,150,30], ...
                    'Callback', @plot_Callback);

% Adding a drop-down menu, to choose data from
%hMenu = uicontrol('Style', 'dropdownmenu', ...
%                    'Position', [

% Add axis for the plot
hAxis = axes('Units', 'pixels', 'Position', [50,50,500,485]);

% Add a menu to choose which data that should be used if subplot is used

% Initialize the UI by setting normalized components
hFigure.Units = 'normalized';
hSerialButton.Units = 'normalized';
hPlot.Units = 'normalized';
hAxis.Units = 'normalized';

% Show the window
hFigure.Visible = 'on';

% Callbacks

end

% Callbacks
function serialButton_Callback(src,event,obj)
    % Check if connection is open
    %if ( obj.Status == 'closed' )
    if ( strcmp(obj.Status, 'closed') )
        % Open the serial connection
        fopen(obj);
        % Check again if connection is now open
        if ( strcmp(obj.Status, 'open') )
            % Change text for button
            src.String = 'Serial disconnect';
        end
    else
        % Connection is already open
        % lets close it
        fclose(obj);
        % Change text for button
        src.String = 'Serial connect';
    end
end

function plot_Callback(src, event)
    % We should plot the window here
    
    % Setup time vector
    
    % Find what kind of plot
    % if ( reference data != none ) 
    %       then we do a subplot
    % else
    %       then we do a normal plot
    plot();
end

function serialConnectEventHandler(src,event,outFile)
    bytes = get(src, 'BytesAvailable');

    if ( bytes > 0 )
        %data = fread(obj);
        data = fscanf(obj);
        fwrite(outFile, data);
    end

end
