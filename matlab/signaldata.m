%function [inputSignal_psk, iData, qData] = signaldata
function signaldata
% This should generate:
%       A 5,8 GHz base signal
%       A 1 kHz base signal
%       A 500 Hz base signal
%       A heartbeat signal 100 beats/min
%       A respiration rate of 16 breaths/min
%       And then mix these signals together
% Some defines
HEARTBEATS = 100;
RESP_RATE = 16;
BASE_SIGNAL_500HZ = 500;
%BASE_SIGNAL_1KHZ = 1000;
BASE_SIGNAL_5_8_GHZ = 5800000000;
DOWN_SAMPLE_FACTOR = 16;

% Calculating the frequency of the heartbeat
beatsSec =  HEARTBEATS / 60;

% Calculating the frequency of respiration
respSec = RESP_RATE / 60;

% Samples per second
fs = 2048;
timePeriod = 1 / fs;

% I want a dataset for 20 sec
timeBase = 20;
myLen = timeBase / timePeriod;
t = (0:myLen-1) * timePeriod;

% Generation of the signal (not using 5,8 GHz signal atm)
%inputSignal =  0.3*sin(2*pi*beatsSec*t) + 0.7*sin(2*pi*respSec*t) + 0.5*sin(2*pi*BASE_SIGNAL_500HZ*t);
inputSignal =  0.7*sin(2*pi*respSec*t) + 0.5*sin(2*pi*BASE_SIGNAL_500HZ*t);

% Plotting the inputsignal
figure(1)
plot(t,inputSignal)
axis([0 20 -2 2])

% Creating a PSK signal as we expect to receive at the antenna
% Test signals
modulResp = 1.8 * sin(2 * pi * respSec * t);
modulBeat = 0.3 * sin(2 * pi * beatsSec * t);
%noice
inputSignal_psk = 1.0 * sin((2 * pi * BASE_SIGNAL_500HZ * t) + modulResp + modulBeat);

% Plotting the inputsignal PSK
figure(2)
plot(t,inputSignal_psk)
axis([0 0.3 -3.5 3.5])

% Use Hilbert transform to get I and Q components of the signal
hilbertSignal = hilbert(inputSignal_psk);
iData = real(hilbertSignal);
qData = imag(hilbertSignal);

% Plotting the I and Q data
figure(3)
plot(t, iData, t, qData)
%plot(t, hilbertSignal)
axis([0 0.1 -2 2])

% Calculating the FFT spectrum of the modulated signal
Y = fft(inputSignal_psk);

P2 = abs(Y/myLen);
P1 = P2(1:myLen/2+1);
P1(2:end-1) = 2*P1(2:end-1);
f = fs * (0:(myLen/2))/myLen;
%figure('Number', 1, 'Name', 'FFT', 'NumberTitle', 1)
figure(4)
plot(f,P1)
axis([480 520 0 0.5])

% Testing
%plot(t, sqrt(2 * iData .* iData))
%axis([0 20 -1 5 ])

% Powerdetection on I and Q signals to remove the 500 Hz carrier
iDataSquared = sqrt(downsample(2 * iData .* iData, DOWN_SAMPLE_FACTOR));
qDataSquared = sqrt(downsample(2 * qData .* qData, DOWN_SAMPLE_FACTOR));

testSignal = sqrt(2 * inputSignal_psk .* inputSignal_psk);

myLen = myLen/DOWN_SAMPLE_FACTOR;
t = t(1:DOWN_SAMPLE_FACTOR:end);

% Ok, up until here we have made the signals that we can expect at
% the input of the ADC, that is, we have created the I and Q signals
% The following is gonna happen in supposed to be implemented with code in
% MCU

% Pre-processing
% I and Q data is sampled simultaneously from 2 separate ADCs
% When enough data has been sampled, this should be low-passed filtered
% with breaking frequency of 50 Hz, using Butterworth-implementation

% Assemble the signal
inputSignal = iDataSquared + 1i*qDataSquared;

% Calculating the FFT spectrum of the modulated signal
Y = fft(testSignal);

P2 = abs(Y/myLen);
P1 = P2(1:myLen/2+1);
P1(2:end-1) = 2*P1(2:end-1);
f = fs * (0:(myLen/2))/myLen;
%figure('Number', 1, 'Name', 'FFT', 'NumberTitle', 1)
figure(5)
plot(f,P1)

% Plot it
%plot(t, inputSignal)

% Filter the hilbert signal
%hilbert_filtered = sqrt(butterworthFilter(fs, downsample(2 * hilbertSignal .* hilbertSignal, DOWN_SAMPLE_FACTOR)));
%hilbert_filtered = sqrt(butterworthFilter(fs, 2 * hilbertSignal .* hilbertSignal));

% Filter data 
%inputSignalFiltered = butterworthFilter(fs, inputSignal);

% Downsample data
inputSignalFiltered = butterworthFilter(fs, inputSignal(1:DOWN_SAMPLE_FACTOR:end));

% Remove the offset
inputSignalFiltered = inputSignalFiltered - 0.9 - 1i*0.9;

% Amplify the signal
inputSignalFiltered = inputSignalFiltered * 30;

% Downsample t since we have done it with our input data
t = t(1:DOWN_SAMPLE_FACTOR:end); %downsample(t, DOWN_SAMPLE_FACTOR);

% Plot the new filtered signals
figure(6)
plot(t, real(inputSignalFiltered), t, imag(inputSignalFiltered))
%plot(t2, hilbert_filtered)
axis([0 20 -3 3])

% Calculate the phase of the input signal
inputPhase = atan2(imag(inputSignalFiltered), real(inputSignalFiltered));

% Plot the inputsignals phase
figure(7)
plot(t, inputPhase)
axis([0 20 -2 2])

myLen = myLen / DOWN_SAMPLE_FACTOR;

% Plot the frequency spectrum
% Calculating the FFT spectrum of the modulated signal
Y = fft(inputPhase);

P2 = abs(Y/myLen);
P1 = P2(1:myLen/2+1);
P1(2:end-1) = 2*P1(2:end-1);
f = fs * (0:(myLen/2))/myLen;
%figure('Number', 1, 'Name', 'FFT', 'NumberTitle', 1)
figure(8)
plot(f,P1)
%axis([-0.5 3 0 0.8])

end