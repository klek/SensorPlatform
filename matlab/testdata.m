function [superPosSignal, phaseShiftSignal] = testdata
% Creating two samples testvectors to verify workings of the FFT, 
% filter and arctangent calculation in the STM32F767Zi DSP

% Some defines
LOW_FREQ = 10;
MEDIUM_FREQ_1 = 40; % This should be lower than filter threshold
MEDIUM_FREQ_2 = 50; % This should be higher than filter threshold
HIGH_FREQ = 500;

% Samples per second
fs = 2048;
timePeriod = 1 / fs;

% I want a dataset for 10 sec
timeBase = 1;
myLen = timeBase / timePeriod;
t = (0:myLen-1) * timePeriod;

% Generating a superposition signal of the four frequencies
a = 2 * pi * t;
superPosSignal_orig = 0.5*sin(a*LOW_FREQ) + 0.5*sin(a*MEDIUM_FREQ_1) + 0.5*sin(a*MEDIUM_FREQ_2) + 0.5*sin(a*HIGH_FREQ);

% Plotting the inputsignal
%figure(1)
%plot(t,superPosSignal_orig)
%axis([0 1 -2 2])

% Now add zeroes to the imaginary parts
zero(1:2048) = zeros;
superPosSignal(1:2:4096) = superPosSignal_orig;
superPosSignal(2:2:4096) = zero;

% Create a c-header file
fd = fopen('../testing/superPosSignal.h', 'wt');

% Format the header file with includes
fprintf(fd, '#include "arm_math.h"\n\n');
fprintf(fd, '/*\n');
fprintf(fd, ' * Test vector for FFT function and filter. Test vector\n');
fprintf(fd, ' * is a super position of the four frequencies\n');
fprintf(fd, ' * %i Hz, %i Hz, %i Hz, %i Hz \n', LOW_FREQ, MEDIUM_FREQ_1, MEDIUM_FREQ_2, HIGH_FREQ);
fprintf(fd, ' */\n\n');

% Write the super position signal to header file
fprintf(fd, 'float32_t superPosSignal[%d] = {\n \t%.15g', length(superPosSignal), superPosSignal(1));
fprintf(fd, ',\n \t%.15g', superPosSignal(2:end));
fprintf(fd, '\n};\n');
fclose(fd);

% Generating the phase shift signal to test arctangent calculation
pskSignal = sin(superPosSignal_orig);

% Plotting the phase shifted signal
%figure(2)
%plot(t, pskSignal)
%axis([0 1 -2 2])

% The phaseShiftSignal needs to contain every other sample as a cosine of
% the same wave, that is a 90 degree phaseshift
hilbertSig = hilbert(pskSignal);
%phaseShiftSignal = real(hilbertSig) + li*imag(hilbertSig);
phaseShiftSignal(1:2:4096) = real(hilbertSig);
phaseShiftSignal(2:2:4096) = imag(hilbertSig);

% Create a c-header file
fd = fopen('../testing/phaseShiftSignal.h', 'wt');

% Format the header file with includes
fprintf(fd, '#include "arm_math.h"\n\n');
fprintf(fd, '/*\n');
fprintf(fd, ' * Test vector for arctangent function and FFT. Test vector\n');
fprintf(fd, ' * is a phase shift of the super position containing the four frequencies\n');
fprintf(fd, ' * %i Hz, %i Hz, %i Hz, %i Hz \n', LOW_FREQ, MEDIUM_FREQ_1, MEDIUM_FREQ_2, HIGH_FREQ);
fprintf(fd, ' * The signal is complex, with following look :\n');
fprintf(fd, ' * {real(0), imag(0), real(1), imag(1), real(2), imag(2), ...... } \n');
fprintf(fd, ' */\n\n');

% Write the super position signal to header file
fprintf(fd, 'float32_t phaseShiftSignal[%d] = {\n \t%.15g', length(phaseShiftSignal), 0); % The first sample is buggedphaseShiftSignal(1));
fprintf(fd, ',\n \t%.15g', phaseShiftSignal(2:end));
fprintf(fd, '\n};\n');
fclose(fd);

% Generation of the signal (not using 5,8 GHz signal atm)
%inputSignal =  0.3*sin(2*pi*beatsSec*t) + 0.7*sin(2*pi*respSec*t) + 0.5*sin(2*pi*BASE_SIGNAL_500HZ*t);
%inputSignal =  0.5*sin(2*pi*1*t) + 0.5*sin(2*pi*0.5*t);


end