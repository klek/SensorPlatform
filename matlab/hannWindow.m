function hannWindow
% Creates Hann coefficients for a 64 sample vector
% and writes half of them to the file hannCoeff.h
% They are only for smoothing the edges of each data set

% Defines the length
L = 64;

% The hann function is given by hann-matlab function
hannCoeff = hann(L)'

% Create a c-header file
fd = fopen('../code/maths/hannCoeff.h', 'wt');

% Format the header file with includes
fprintf(fd, '#include "arm_math.h"\n\n');
fprintf(fd, '/*\n');
fprintf(fd, ' * Test vector containing Hann coefficients that should be applied\n');
fprintf(fd, ' * to the samples before FFT calculation. Since the hann window\n');
fprintf(fd, ' * is symmetrical only half of the values are here\n');
fprintf(fd, ' */\n\n');

% Write the super position signal to header file
fprintf(fd, 'static const float32_t hannCoeff[%d] = {\n \t%.15g', length(hannCoeff)/2, hannCoeff(1));
fprintf(fd, ',\n \t%.15f', hannCoeff(2:end/2));
fprintf(fd, '\n};\n');
fclose(fd);


end