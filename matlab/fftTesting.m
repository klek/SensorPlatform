
% Plot the source vector as reference
[t1, t2] = testdata;
figure(2)
fftSpectrum(t1, 1);
title('FFT Spectrum (only FFT)');
ylabel('Magnitude [V]');
xlabel('Frequency [Hz]');

% Calculate the arctan taylor approx
phaseShiftApprox = (1:length(t2)/2);
for i = 1:length(t2)/2
    phaseShiftApprox(i) = atan2TaylorApprox(t2(2*i),t2(2*i - 1));
end
figure(3)
fftSpectrum(phaseShiftApprox, 1);
title('FFT Spectrum (TaylorApprox)');
ylabel('Magnitude [V]');
xlabel('Frequency [Hz]');

% Calculate the arctan reference signal
phaseShift = atan2(t2(2:2:end),t2(1:2:end));
figure(4)
fftSpectrum(phaseShift, 1);
title('FFT Spectrum (atan2)');
ylabel('Magnitude [V]');
xlabel('Frequency [Hz]');

% Calculate the arctan reference signal
%phaseShift = atan2(t2(2:2:end),t2(1:2:end));
figure(5)
fftSpectrum(t2, 1);
title('FFT Spectrum (atan2)');
ylabel('Magnitude [V]');
xlabel('Frequency [Hz]');

