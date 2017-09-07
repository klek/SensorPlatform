function fftSpectrum(inputSignal, magFlag)
% Calculating the FFT spectrum of the input signal
% Specify magFlag = 1 if magnitude needs to be calculated
% otherwise magFlag = 0

% Samples per second
fs = 2048;
timePeriod = 1 / fs;

% I want a dataset for 10 sec
timeBase = 1;
myLen = timeBase / timePeriod;
%t = (0:myLen-1) * timePeriod;

% Calculate the FFT
Y = fft(inputSignal);

if ( magFlag == 1 )
    P2 = abs(Y/myLen);
else
    P2 = inputSignal;
end

P1 = P2(1:myLen/2+1);
P1(2:end-1) = 2*P1(2:end-1);

f = fs * (0:(myLen/2))/myLen;
%figure('Number', 1, 'Name', 'FFT', 'NumberTitle', 1)
%figure(4)
plot(f,P1)
%axis([0 800 0 0.8])

end