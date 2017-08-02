function output = butterworthFilter(Fs, input)
% Lowpass butterworth filter of 3 order
% Fs is the sampling frequency of the input data

% Setting up the filter
cutOffFreq = 50;

% Nyquist frequency 
nyqFreq = Fs / 2;
cutOffNorm = cutOffFreq / nyqFreq;

% Order of the filter
order = 3;

% Creating the filter coefficients
[z,p,k] = butter(order, cutOffNorm);

% Convert to SOS matrix
sos = zp2sos(z,p,k)

% Remove a0
coeffs = sos(:,[1 2 3 5 6])

% Sort into a linear buffer
coeffs = [coeffs(1,:) coeffs(2,:)]

% Verifying the filter
%freqz(b,a)

output = filter(z, p, input);

end