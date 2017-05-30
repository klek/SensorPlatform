function output = fir_filter(Fs, input)

% Setting up the filter
cutOffFreq = 50;

% Nyquist frequency 
nyqFreq = Fs / 2;
cutOffNorm = cutOffFreq / nyqFreq;

% Order of the filter
order = 28;

% Creating the filter coefficients
firCoeff = fir1(order, cutOffNorm)

% Verifying the filter
%fvtool(firCoeff, 'Fs', fs)

output = filter(firCoeff, 1, input);

end