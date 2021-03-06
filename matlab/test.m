function Hd = test
%TEST Returns a discrete-time filter object.

% MATLAB Code
% Generated by MATLAB(R) 8.5 and the Signal Processing Toolbox 7.0.
% Generated on: 26-Jul-2017 14:40:30

% Butterworth Lowpass filter designed using FDESIGN.LOWPASS.

% All frequency values are in Hz.
Fs = 2000;  % Sampling Frequency

N  = 3;   % Order
Fc = 50;  % Cutoff Frequency

% Construct an FDESIGN object and call its BUTTER method.
h  = fdesign.lowpass('N,F3dB', N, Fc, Fs);
Hd = design(h, 'butter');

% [EOF]
