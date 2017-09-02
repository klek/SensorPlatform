function signal = sensorRead

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

% Wait 30 sec for the output to be done
% NOTE(klek): This can be done in a prettier way?
pause(30)

% Close the serial connection
fclose(obj);

% Close the outputfile
fclose(outputFile);
clear outputFile;

% We want to open the textfile in readonly mode now to grab the data
% we want only
outputFile = fopen('../testing/output/out.txt', 'rt');

% Now we want the data section of the file only
% Start by reading each "word"
words = textscan(outputFile, '%s');

% Close the file
fclose(outputFile);
clear outputFile;

% Now find where the matrix starts and ends
index1 = find(strcmp(words{1}, '['))
index2 = find(strcmp(words{1}, '];'))

% We now want all elements in between here
temp1 = [ words{1}{index1 + 1: index2 - 1} ];

% Parse this new vector 
temp2 = textscan(temp1, '%f', 'Delimiter', ';');

% Put it into a vector
signal = temp2{1}';

% Display the data using fftSpectrum
figure(1)
fftSpectrum(signal, 0);
title('FFT spectrum');
ylabel('Magnitude [V]');
xlabel('Frequency [Hz]');

% Plot the source vector as reference
[t1, t2] = testdata;
figure(2)
fftSpectrum(t1, 1);
title('FFT Spectrum (only FFT)');
ylabel('Magnitude [V]');
xlabel('Frequency [Hz]');

% Calculate the arctan reference signal
phaseShift = atan2(t2(2:2:end),t2(1:2:end))
figure(3)
fftSpectrum(t2, 1);
title('FFT Spectrum (arctan demodulation)');
ylabel('Magnitude [V]');
xlabel('Frequency [Hz]');
end