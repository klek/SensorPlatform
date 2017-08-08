function filtering

fs = 2048;
timePeriod = 1 / fs


timeBase = 20;
myLen = timeBase / timePeriod;
t = (0:myLen-1) * timePeriod;

% Creating test signal
x_sin = sin(2 * pi * 2 * t) + sin(2 * pi * 5000 * t);

% Plot test signal
figure(1)
plot(t, x_sin)
axis([0 20 -2 2])

% Filter test signal
output = butterworthFilter(fs, x_sin);

% Plot result
figure(2)
plot(t, output)
axis([0 20 -2 2])

end