function [phi1, phi2, phi3] = atan2TaylorClean(y_param, x_param)
% The Taylor expansion of arctan

% Calculate t
t = y_param/x_param;

phi1 = t - (t^3)/3 + (t^5)/5 - (t^7)/7;
phi2 = t - (t^3)/3 + (t^5)/5 - (t^7)/7 + (t^9)/9;
phi3 = t - (t^3)/3 + (t^5)/5 - (t^7)/7 + (t^9)/9 - (t^11)/9;


end