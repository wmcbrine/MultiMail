cd mmail
for %%x in (*.cc) do cl -nologo -c -O2 -D__WIN32__ -DMM_MAJOR=0 -DMM_MINOR=46 -DUSE_QWK -DUSE_BW -DUSE_OMEN -DUSE_OPX -DUSE_SOUP -Tp %%x
cd ../interfac
for %%x in (*.cc) do cl -c -O2 -D__WIN32__ -DMM_MAJOR=0 -DMM_MINOR=46 -DCURS_INC=\"/msvc/pdcurs26/curses.h\" -DPDC_STATIC_BUILD -Tp %%x
cd ..
link -out:mm.exe mmail\*.obj interfac\*.obj user32.lib \msvc\pdcurs26\win32\pdcurses.lib
