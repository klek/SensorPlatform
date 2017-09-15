function filtering

fs = 2048;
timePeriod = 1 / fs;


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

% Second test
[t1, t2] = testdata;

% I want a dataset for 10 sec
timeBase = 1;
myLen = timeBase / timePeriod;
t = (0:myLen-1) * timePeriod;

% Plot super position signal
figure(3)
plot(t, t1(1:2:end))
title('Super position signal');
xlabel('Time (t)');
ylabel('Voltage (t)');

% Filter the signal
output = butterworthFilter(fs, t1);

% Plot result
figure(4)
plot(t, output(1:2:end))
title('Super position signal filtered');
xlabel('Time (t)');
ylabel('Voltage (t)');

end