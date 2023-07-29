: Step 4 - Verify
: assuming we have a backup
if -r $1 goto dirok
   echo "Usage: call this script with the name of the system directory - perhaps /usr/sys"
   exit

: dirok

echo "Validating the copied file using cmp"
echo "if you get any output the something has failed"

if -r $1/dmr/dc.c goto dcdiff
   echo "$1/dmr/dc.c not installed"
   goto dcfin
: dcdiff
cmp dc.c $1/dmr/dc.c
: dcfin

if -r $1/dmr/dh.c goto dhdiff
   echo "$1/dmr/dh.c not installed"
   goto dhfin
: dhdiff
cmp dh.c $1/dmr/dh.c
: dhfin

if -r $1/dmr/dhdm.c goto dhdmdiff
   echo "$1/dmr/dhdm.c not installed"
   goto dhdmfin
: dhdmdiff
cmp dhdm.c $1/dmr/dhdm.c
: dhdmfin

if -r $1/dmr/kl.c goto kldiff
   echo "$1/dmr/kl.c not installed"
   goto klfin
: kldiff
cmp kl.c $1/dmr/kl.c
: klfin

if -r $1/dmr/tty.c goto ttydiff
   echo "$1/dmr/tty.c not installed"
   goto ttyfin
: ttydiff
cmp tty.c $1/dmr/tty.c
: ttyfin

if -r $1/ken/sysent.c goto sysentdiff
   echo "$1/ken/sysent.c not installed"
   goto sysentfin
: sysentdiff
cmp sysent.c $1/ken/sysent.c
: sysentfin

if -r $1/conf/m40.s goto m40diff
   echo "$1/conf/m40.s not installed"
   goto m40fin
: m40diff
cmp m40.s $1/conf/m40.s
: m40fin

if -r $1/tty.h goto ttyhdiff
   echo "$1/tty.h not installed"
   goto ttyhfin
: ttyhdiff
cmp tty.h $1/tty.h
: ttyhfin

if -r $1/deftty.h goto defttyhdiff
   echo "$1/deftty.h not installed"
   goto defttyhfin
: defttyhdiff
cmp deftty.h $1/deftty.h
: defttyhfin
