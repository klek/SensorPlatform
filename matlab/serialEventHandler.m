function serialEventHandler(obj,~,outputFile)

%fd = fopen('testFile.txt', 'a');
bytes = get(obj, 'BytesAvailable');

if ( bytes > 0 )
    %data = fread(obj);
    data = fscanf(obj);
    fwrite(outputFile, data);
end



end