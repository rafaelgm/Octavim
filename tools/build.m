% Site...: https://github.com/rafaelgm/Octavim
% Author.: Rafael Monteiro

disp('Building OctaveHelper.dll...');
system('gcc OctaveHelper.c -m32 -shared -o OctaveHelper.dll');
disp('Finished!');
