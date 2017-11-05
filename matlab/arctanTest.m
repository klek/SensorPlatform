function arctanTest
% This script is supposed to show the difference
% between the homemade arctan and atan2 from MATLAB
% in the intervall t = (0 , 1.1)



% Create two vectors
y_param = (0:0.01:1.1);
x_param = ones(size(y_param));

% Create the t-vector
t =  (0:0.01:1.1);

% Reference function from MATLAB is atan2
ref_result = atan2(y_param, x_param);

% Result from Taylor arctan
homemade_result1 = zeros(length(x_param),3);
for i = 1:length(x_param)
    [homemade_result1(i,1) homemade_result1(i,2) homemade_result1(i,3)] = atan2TaylorClean(y_param(i), x_param(i));
end

% Result from homemade arctan
homemade_result2 = (1:length(x_param));
for i = 1:length(x_param)
    homemade_result2(i) = atan2TaylorApprox(y_param(i),x_param(i));
end

% Create values for the UNIT circle
theta = linspace(-4*pi, 4*pi, 8000);
y = sin(theta);
x = cos(theta);

% Reference function from MATLAB is atan2
ref_result_unit = atan2(y, x);

% Result from homemade arctan
homemade_result_unit = (1:length(x));
for i = 1:length(x)
    homemade_result_unit(i) = atan2TaylorApprox(y(i),x(i));
end

% Plot the result
figure(1)
plot(t, ref_result, t, homemade_result1(:,1), '--', t, homemade_result1(:,2), '--', t, homemade_result1(:,3), '--')
legend('atan2', 'Taylor O(2)', 'Taylor O(3)', 'Taylor O(4)', 'Location', 'northwest')
title('Comparing atan2 to Taylor expansion')
xlabel('t')
ylabel('phi [rad]')

% Plot the result
figure(2)
plot(t, ref_result, t, homemade_result1(:,1), '--', t, homemade_result1(:,2), '--', t, homemade_result1(:,3), '--')
legend('atan2', 'Taylor O(2)', 'Taylor O(3)', 'Taylor O(4)', 'Location', 'northwest')
title('Comparing atan2 to Taylor expansion [zoomed]')
xlabel('t')
ylabel('phi [rad]')
axis([0.65 1.1 0.6 1])

figure(3)
plot(t, ref_result, t, homemade_result2, '--', t, homemade_result1(:,3), '--')
legend('atan2', 'TaylorApprox', 'Taylor O(4)', 'Location', 'northwest')
title('Comparison of atan2 to Taylor expansion and Taylor approximation')
xlabel('t')
ylabel('phi [rad]')
%axis([0.5 1 0 0.9])

figure(4)
plot(theta, ref_result_unit, theta, homemade_result_unit, '--')
legend('atan2', 'TaylorApprox')
title('Comparing arctan implementations')
xlabel('t')
ylabel('phi [rad]')

end