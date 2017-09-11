function [output, coeffs] = butterworthFilter(Fs, input)
% Lowpass butterworth filter of 3 order
% Fs is the sampling frequency of the input data

% Setting up the filter
cutOffFreq = 45;

% Nyquist frequency 
nyqFreq = Fs / 2;
cutOffNorm = cutOffFreq / nyqFreq;

% Order of the filter
order = 3;

% Creating the filter coefficients
%[z,p,k] = butter(order, cutOffNorm);
[b a] = butter(order, cutOffNorm);

% Convert to SOS matrix
%sos = zp2sos(z,p,k)
sos = tf2sos(b, a);

% Remove a0
coeffs = sos(:,[1 2 3 5 6])

% Negate the two last columns
coeffs(:,4) = coeffs(:,4) * (-1);
coeffs(:,5) = coeffs(:,5) * (-1);

% Sort into a linear buffer
coeffs = [coeffs(1,:) coeffs(2,:)];

% Create a file to hold the filter coefficients
fd = fopen('../code/maths/butterFilterCoeff.h','wt');

% Format the header file with includes
fprintf(fd, '#include "arm_math.h"\n\n');
fprintf(fd, '/*\n');
fprintf(fd, ' * Filter coefficients for the butterworth filter\n');
fprintf(fd, ' */\n\n');

% Print the filter coefficients to the file
fprintf(fd, 'static const float32_t filterCoeff[%d] = {\n \t%0.15g', length(coeffs), coeffs(1));
fprintf(fd, ',\n \t%0.15g', coeffs(2:end));
fprintf(fd, '\n};\n');
fclose(fd);

% Verifying the filter
%freqz(b,a)

% Filter signal
output = filter(b, a, input);

end