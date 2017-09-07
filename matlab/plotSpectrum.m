function plotSpectrum(inputSignal, fftLength, sampleRate)
% Plots the input signal with correct resolution 
% for the specified fftLength and sampleRate

% Get the length of the inputSignal
signalLength = length(inputSignal)

% We need to create an array with corresponding length to
% the inputSignal and ofc correct resolution.
% Calculate resolution as res = Fs / N, N = nr of BINS of the fft

% Nr of BINS in FFT = fftLength / 2
%nrOfBins = fftLength / 2;

% Calculate resolution
%res = sampleRate / nrOfBins;
res = sampleRate / fftLength;

% Create the frequency vector
f = (0:res:signalLength - 1)

% Plot the signal
plot(f, inputSignal)

end