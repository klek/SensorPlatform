function signal = readDSP(string)
% Function to read the textfile from the DSP
% Supports reading the FFT data and filtered data
% if these are enabled in the DSP

% Open and read the textfile
output = fopen('../testing/output/out.txt', 'rt'); 
words = textscan(output, '%s'); 
fclose(output); 
clear output;

index1 = 0;
index2 = 0;

switch(string)
    case 'Filter'
        index1 = find(strcmp(words{1}, '{')); 
        index2 = find(strcmp(words{1}, '};'));
    
    case 'FFT'
        index1 = find(strcmp(words{1}, '[')); 
        index2 = find(strcmp(words{1}, '];'));
        
    otherwise
        
end

% Grab the data
temp1 = [ words{1}{index1 + 1: index2 - 1} ];
% Remove the semicolon
temp2 = textscan(temp1, '%f', 'Delimiter', ';');

% Create the output signal
signal = temp2{1}';

end