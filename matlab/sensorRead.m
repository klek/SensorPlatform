function [signal] = sensorRead

% Create a serial connection
obj = serial('/dev/ttyACM0');

% Open an output file to save data
outputFile = fopen('../testing/output/out.txt', 'wt');

% Setup settings for the device
obj.BaudRate = 115200;%9600;
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
pause(10)

% Close the serial connection
fclose(obj);

% Close the outputfile
fclose(outputFile);
clear outputFile;

% We want to open the textfile in read-only mode now to grab the data
% we want only
outputFile = fopen('../testing/output/out.txt', 'rt');

% Now we want the data section of the file only
% Start by reading each "word"
words = textscan(outputFile, '%s');

% Close the file
fclose(outputFile);
clear outputFile;

% Now find where the matrix starts and ends
index1 = find(strcmp(words{1}, '['));
index2 = find(strcmp(words{1}, '];'));

% We now want all elements in between here
temp1 = [ words{1}{index1 + 1: index2 - 1} ];

% Parse this new vector 
temp2 = textscan(temp1, '%f', 'Delimiter', ';');

% Put it into a vector
signal = temp2{1}';

% Display the data using fftSpectrum
figure(1)
%fftSpectrum(signal, 0);
plotSpectrum(signal, 2048, 2048)
title('FFT spectrum (DSP)');
ylabel('Magnitude [V]');
xlabel('Frequency [Hz]');

% Plot the source vector as reference
[t1, t2] = testdata;
figure(2)
fftSpectrum(t1, 1);
title('FFT Spectrum (only FFT)');
ylabel('Magnitude [V]');
xlabel('Frequency [Hz]');

% Calculate the arctan taylor approx
phaseShiftApprox = (1:length(t2)/2);
for i = 1:length(t2)/2
    phaseShiftApprox(i) = atan2TaylorApprox(t2(2*i),t2(2*i - 1));
end
figure(3)
fftSpectrum(phaseShiftApprox, 1);
title('FFT Spectrum (TaylorApprox)');
ylabel('Magnitude [V]');
xlabel('Frequency [Hz]');

% Calculate the arctan reference signal
phaseShift = atan2(t2(2:2:end),t2(1:2:end));
figure(4)
fftSpectrum(phaseShift, 1);
title('FFT Spectrum (atan2)');
ylabel('Magnitude [V]');
xlabel('Frequency [Hz]');
end