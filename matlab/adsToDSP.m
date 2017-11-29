function adsToDSP(filename, rowNr, colNr, sampleRate, totalTime)
% Function to import data from a CSV-file generated with ADS in order to
% test and verify the output of the RF-frontend. This data is presented in
% a frequency spectrum in figure 1.
% The data is also re-written into a C header-file, containing a single row
% vector suitable for calculations with the DSP-backend. The output file
% and the row vector are called adsTestVector. Please not that to guarantee
% functionality with the DSP-backend, the sampleRate needs to be 2048 for
% the data generated in ADS. Furthermore, in order to keep a decent size on
% the output vector, the totalTime should be one of the following {1, 2, 4,
% 8, 16}
%
% Input parameters:
%
% FILENAME corresponds to the file containing the csv-table.
%
% ROWNR corresponds to the first row containing valid data in the
% csv-table
%
% COLNR corresponds to the first column containing valid data in the
% csv-table
%
% SAMPLERATE dictates the samplerate of the data in the csv-file. This
% samplerate is expected to be 2048 Sa/s to work with the DSP-backend
%
% TOTALTIME dictates the time used to collect the data. This time also
% corresponds to the decimation/delay used in the DSP-backend
%
% Further notes:
% In ADS it is good to extend the number of significant digits to 9+ in the
% plot options of the table. Otherwise, most of the data will be seen as
% zeros for MATLAB.


% Read the csv-file
inputSignal = csvread(filename, rowNr, colNr);

% Extract the I- and Q-data from the inputSignal
% I-data is expected to be in first column
inPhaseData = inputSignal(:,1);
% Q-data is expected to be in second column
quadPhaseData = inputSignal(:,2);

% Create a single row vector containing I- and Q-data in following way
% I1 Q1 I2 Q2 I3 Q3 ... Iend Qend
% First create a vector that can hold the total size
result = [inPhaseData ; quadPhaseData];
% Insert the I-data
result(1:2:end) = inPhaseData;
% Insert the Q-data
result(2:2:end) = quadPhaseData;


% Now this data is ready to be written into a header-file to be included in
% the DSP-compilation
writeToFile(result, totalTime);

% The result can also be shown in a frequency spectrum to verify the
% components of the signal
figure(1);
spectrum(result, sampleRate, totalTime);

end

function writeToFile(data, decimation)
% Create a c-header file
fd = fopen('adsTestVector.h', 'wt');

% Format the header file with includes
fprintf(fd, '#include "arm_math.h"\n\n');
fprintf(fd, '/*\n');
fprintf(fd, ' * Test vector for the entire algorithm apart from the ADC. Test vector\n');
fprintf(fd, ' * has been generated in ADS from simulation results of the RF-frontend\n');
fprintf(fd, ' */\n\n');

% Write the decimation used
fprintf(fd, '#define TIME_DEC \t %d\n\n', decimation);

% Write the result position signal to header file
fprintf(fd, 'float32_t adsTestVector[%d] = {\n \t%.15g', length(data), data(1)); % The first sample is buggedphaseShiftSignal(1));
fprintf(fd, ',\n \t%.15g', data(2:end));
fprintf(fd, '\n};\n');
fclose(fd);

end

function spectrum(inputSignal, sampleRate, tBase)
% Calculating the FFT spectrum of the input signal

% Samples per second
fs = sampleRate;
timePeriod = 1 / fs;

% I want a dataset for 10 sec
timeBase = tBase;
myLen = timeBase / timePeriod;
%t = (0:myLen-1) * timePeriod;

% Calculate the FFT
Y = fft(inputSignal);

% Calculate the magnitude of the spectrum and scale accordingly
P2 = abs(Y/myLen);

% Create the frequency axis
P1 = P2(1:myLen/2+1);
P1(2:end-1) = 2*P1(2:end-1);

f = fs * (0:(myLen/2))/myLen;

% Figure is already specified from parent file
plot(f,P1)
% Add a title and labels on the axis
title('Frequency spectrum of vital signs from ADS');
xlabel('Frequency [Hz]');
ylabel('Magnitude [mV]');

end

