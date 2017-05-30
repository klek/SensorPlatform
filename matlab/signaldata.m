function [inputSignal, beatsSec, respSec] = signaldata
% This should generate:
%       A 5,8 GHz base signal
%       A 1 kHz base signal
%       A 500 Hz base signal
%       A heartbeat signal 100 beats/min
%       A respiration rate of 16 breaths/min
%       And then mix these signals together1
% Some defines
HEARTBEATS = 100;
RESP_RATE = 16;
BASE_SIGNAL_500HZ = 500;
%BASE_SIGNAL_1KHZ = 1000;
%BASE_SIGNAL_5_8_GHZ = 5800000000;

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
% Test signal
testSignal = 0.7 * sin(2 * pi * respSec * t);
inputSignal_psk = 0.5 * sin((2 * pi * BASE_SIGNAL_500HZ * t) + testSignal);

% Plotting the inputsignal
figure(1)
plot(t,inputSignal)
axis([0 20 -2 2])

% Plotting the inputsignal PSK
figure(2)
plot(t,inputSignal_psk)
axis([0 20 -2 2])


% FFT
Y = fft(inputSignal);

P2 = abs(Y/myLen);
P1 = P2(1:myLen/2+1);
P1(2:end-1) = 2*P1(2:end-1)
f = fs * (0:(myLen/2))/myLen;
%figure('Number', 1, 'Name', 'FFT', 'NumberTitle', 1)
figure(3)
plot(f,P1)
end