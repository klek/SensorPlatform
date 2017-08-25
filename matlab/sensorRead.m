function sensorRead

% Create a serial connection
obj = serial('/dev/ttyACM0');

% Open an output file to save data
outputFile = fopen('../testing/output/out.txt', 'wt');

% Setup settings for the device
obj.BaudRate = 9600;
obj.Parity = 'odd';
obj.StopBits = 1;
obj.DataBits = 7;
obj.Timeout = 20;

% Configure the callback
obj.BytesAvailableFcnMode = 'byte';
obj.BytesAvailableFcnCount = 1;
obj.BytesAvailableFcn = {@serialEventHandler, outputFile};

% Now open the serial connection
fopen(obj);

pause(30)

% Close the serial connection
fclose(obj);

% Close the outputfile
fclose(outputFile);

end