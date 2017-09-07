function [phi] = atan2TaylorApprox(y_param, x_param)
% This is atan2 implemented with Taylor expansions. 
% This expansion is using a total of 5 coefficients.
% This atan rotates each angle into the first octant and
% then calculates the angle for the easier part

% The constants used here
ATAN_CONSTANT_1 = 1.00006;
ATAN_CONSTANT_2 = 0.33316;
ATAN_CONSTANT_3 = 0.19048;
ATAN_CONSTANT_4 = 0.09916;
ATAN_CONSTANT_5 = 0.02723;

% Rotating the angle into the first octant
octant = 0;
if ( y_param < 0 )
    % Y-value dictates angle is in octant 5-8
    % Rotate by 180 degrees
    x_param = -x_param;
    y_param = -y_param;
    octant = octant + 4;
end
if ( x_param <= 0 )
    % X-value dictates angle is in octant 3-6
    % Rotate by 90 degrees
    temp = -x_param;
    x_param = y_param;
    y_param = temp;
    octant = octant + 2;
end
if ( x_param <= y_param ) 
   % X-value dictates angle is in second octant
   % rotate by 45 degrees
   temp = y_param - x_param;
   x_param = x_param + y_param;
   y_param = temp;
   octant = octant + 1;
end

% Save the shift in angle for later
angleShift = octant * pi / 4;

% Input parameters should now be shifted for first octant
% start calculate angle
% NOTE(klek): Here the extra gain from the last if-statement gets removed
%             since it is applied to both coordinates.
angle = y_param / x_param;

% Calculate the squared angle for the taylor expansion (t²)
t_squared = -(angle * angle);

% Calculating the different steps of the arctan
phi = ATAN_CONSTANT_5;
phi = ATAN_CONSTANT_4 + (t_squared * phi);
phi = ATAN_CONSTANT_3 + (t_squared * phi);
phi = ATAN_CONSTANT_2 + (t_squared * phi);
phi = ATAN_CONSTANT_1 + (t_squared * phi);
phi = angle * phi;

% Add the shifted angle to the result
phi = angleShift + phi;
end