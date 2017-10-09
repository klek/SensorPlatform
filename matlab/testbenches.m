function [phaseShiftSignal, test, result, t] = testbenches(decimation)

% Creating two samples testvectors to verify workings of the FFT, 
% filter and arctangent calculation in the STM32F767Zi DSP

% Defining the frequencies
RESP_RATE = 16 / 60; % 0.26 Hz
HEART_RATE = 70 / 60; % 1.167 Hz
LOW_FREQ = 10;
MEDIUM_FREQ_1 = 40;
MEDIUM_FREQ_2 = 50; 
HIGH_FREQ = 500;
DC_OFFSET = 0;

% Samples per second
fs = 2048;
timePeriod = 1 / fs;

% I want a dataset for decimation sec
timeBase = decimation;       % Corresponds to decimation
myLen = timeBase / timePeriod;
t = (0:myLen-1) * timePeriod;

% Generating a superposition signal of the four frequencies
a = 2 * pi * t;
superPosSignal_orig = (0.3*sin(a*RESP_RATE) + 0.03*sin(a*HEART_RATE) + 0.1*sin(a*MEDIUM_FREQ_1) + 0.4*sin(a*HIGH_FREQ));
%superPosSignal_orig = (0.5*sin(a*LOW_FREQ) + 0.5*sin(a*MEDIUM_FREQ_1) + 0.5*sin(a*MEDIUM_FREQ_2) + 0.5*sin(a*HIGH_FREQ));

% Generating the phaseshifted signal
%pskSignal_real = cos(superPosSignal_orig) + DC_OFFSET;
%pskSignal_imag = sin(superPosSignal_orig) + DC_OFFSET;
pskSignal = sin(superPosSignal_orig);

% Generating the complex signal
hilbertSignal = hilbert(pskSignal);

% Putting it together 
phaseShiftSignal(1:2:length(pskSignal)*2) = real(hilbertSignal) + DC_OFFSET;
phaseShiftSignal(2:2:length(pskSignal)*2) = imag(hilbertSignal) + DC_OFFSET;

%offsetBy = min(phaseShiftSignal);
%mean(phaseShiftSignal)
%max(phaseShiftSignal)

% Now we need to convert values to integers of 12-bits size
% Amplify the signal to the upper part of 12-bits
% We expect the signal to be between 0-3.0 V
% 12-bits = 4096 => we should be able to multiply by 4096/3000 = 1.36
% To get to the highest range in Volts. Also converting to millivolt
% results in multiplication by 1360

%whos phaseShiftSignal
% Down convert the double floating point precision vector to single in
% order to half the number of bytes used. This removes information!
phaseShiftSignal = single(phaseShiftSignal);
test = phaseShiftSignal;
%whos phaseShiftSignal

% Find a value to offset the signal by
offsetBy = min(phaseShiftSignal);
%test = test + (-offsetBy);

% Cast the floating values to unsigned 32-bit ints. This shouldn't remove
% any information, as in change any bits of the signal.
phaseShiftSignal = typecast((phaseShiftSignal*1360), 'uint32');
[maxVal index] = max(phaseShiftSignal);
%fprintf('Value %d in binary is %16s\n', phaseShiftSignal(index), dec2bin(phaseShiftSignal(index)))

% Check if we are in bounds of 12-bits
while ( maxVal >= 4096 )
    % Then we need rightshift the values in the vector
    phaseShiftSignal = bitshift(phaseShiftSignal, -1);
    % Calculate max again
    [maxVal index] = max(phaseShiftSignal);
end
%fprintf('Value %d in binary is %16s\n', phaseShiftSignal(index), dec2bin(phaseShiftSignal(index)))

% Now we need to pack the vector into half the size, with real (I-data) in
% the lower 16-bits and the imag-data (Q-data) in the upper 16-bits of a
% 32-bit unsigned value
result = uint32(bitshift(phaseShiftSignal(2:2:end),16) + phaseShiftSignal(1:2:end));
[maxVal index] = max(result);
%fprintf('Value %d in binary is %16s\n', result(index), dec2bin(result(index)))

% Create a c-header file
fd = fopen('../testing/testbench.h', 'wt');

% Format the header file with includes
fprintf(fd, '#include "arm_math.h"\n\n');
fprintf(fd, '/*\n');
fprintf(fd, ' * Test vector for the entire algorithm apart from the ADC. Test vector\n');
fprintf(fd, ' * is a phase shift of a super position containing the four frequencies\n');
fprintf(fd, ' * %.3i Hz, %.3i Hz, %i Hz, %i Hz\n', RESP_RATE, HEART_RATE, MEDIUM_FREQ_2, HIGH_FREQ);
fprintf(fd, ' * The signal is quadrature, and packed in the following way:\n');
fprintf(fd, ' * Element 0 = {QQQQ QQQQ QQQQ QQQQ IIII IIII IIII IIII}, where\n');
fprintf(fd, ' * each letter corresponds to a bit.\n');
fprintf(fd, ' */\n\n');

% Write the decimation used
fprintf(fd, '#define TIME_DEC \t %d\n\n', decimation);

% Write the result position signal to header file
fprintf(fd, 'uint32_t testBenchSignal[%d] = {\n \t%d', length(result), result(1)); % The first sample is buggedphaseShiftSignal(1));
fprintf(fd, ',\n \t%d', result(2:end));
fprintf(fd, '\n};\n');
fclose(fd);

% Create a c-header file
fd = fopen('../testing/testbench_floats.h', 'wt');

% Format the header file with includes
fprintf(fd, '#include "arm_math.h"\n\n');
fprintf(fd, '/*\n');
fprintf(fd, ' * Test vector for the entire algorithm apart from the ADC. Test vector\n');
fprintf(fd, ' * is a phase shift of a super position containing the four frequencies\n');
fprintf(fd, ' * %.3i Hz, %.3i Hz, %i Hz, %i Hz\n', RESP_RATE, HEART_RATE, MEDIUM_FREQ_2, HIGH_FREQ);
fprintf(fd, ' * The signal is quadrature, and packed in the following way:\n');
fprintf(fd, ' * Element 0 = {QQQQ QQQQ QQQQ QQQQ IIII IIII IIII IIII}, where\n');
fprintf(fd, ' * each letter corresponds to a bit.\n');
fprintf(fd, ' */\n\n');

% Write the decimation used
fprintf(fd, '#define TIME_DEC \t %d\n\n', decimation);

% Write the result position signal to header file
fprintf(fd, 'float32_t testBenchSignal_floats[%d] = {\n \t%.15g', length(test), 0); % The first sample is buggedphaseShiftSignal(1));
fprintf(fd, ',\n \t%.15g', test(2:end));
fprintf(fd, '\n};\n');
fclose(fd);


end