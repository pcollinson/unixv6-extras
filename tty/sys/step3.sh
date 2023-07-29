: Step 3 - install the files
: assuming we have a backup
if -r $1 goto dirok
   echo "Usage: call this script with the name of the system directory - perhaps /usr/sys"
   exit

: dirok

echo '*** Step 3: Installing replacement files'

if -r $1/dmr/orig/dc.c goto dcinst
echo "No backup for "$1"/dmr/dc.c - not installed"
goto dcend
: dcinst
echo cp dc.c $1/dmr/dc.c
cp dc.c $1/dmr/dc.c
: dcend

if -r $1/dmr/orig/dh.c goto dhinst
echo "No backup for "$1"/dmr/dh.c - not installed"
goto dhend
: dhinst
echo cp dh.c $1/dmr/dh.c
cp dh.c $1/dmr/dh.c
: dhend

if -r $1/dmr/orig/dhdm.c goto dhdminst
echo "No backup for "$1"/dmr/dhdm.c - not installed"
goto dhdmend
: dhdminst
echo cp dhdm.c $1/dmr/dhdm.c
cp dhdm.c $1/dmr/dhdm.c
: dhdmend

if -r $1/dmr/orig/kl.c goto klinst
echo "No backup for "$1"/dmr/kl.c - not installed"
goto klend
: klinst
echo cp kl.c $1/dmr/kl.c
cp kl.c $1/dmr/kl.c
: klend

if -r $1/dmr/orig/tty.c goto ttyinst
echo "No backup for "$1"/dmr/tty.c - not installed"
goto ttyend
: ttyinst
echo cp tty.c $1/dmr/tty.c
cp tty.c $1/dmr/tty.c
: ttyend

if -r $1/ken/orig/sysent.c goto sysentinst
echo "No backup for "$1"/ken/sysent.c - not installed"
goto sysentend
: sysentinst
echo cp sysent.c $1/ken/sysent.c
cp sysent.c $1/ken/sysent.c
: sysentend

if -r $1/conf/orig/m40.s goto m40inst
echo "No backup for "$1"/conf/m40.s - not installed"
goto m40end
: m40inst
echo cp m40.s $1/conf/m40.s
cp m40.s $1/conf/m40.s
: m40end

if -r $1/orig/tty.h goto ttyhinst
echo "No backup for "$1"/tty.h - not installed"
goto ttyhend
: ttyhinst
echo cp tty.h $1/tty.h
cp tty.h $1/tty.h
: ttyhend
exit
