: Script to remove the files we need to remove
: ensuring that the backup files exist
if -r $1 goto dirok
   echo "Usage: call this script with the name of the system directory - perhaps /usr/sys"
   exit

: dirok
echo '*** Step 2: Removing files that are unwanted'

if \! -r $1/dmr/orig/cat.c goto catno
   if \! -r $1/dmr/cat.c goto catok
   echo "Removing "$1"/dmr/cat.c"
   rm -f $1/dmr/cat.c
   goto catok
: catno
echo "No backup found for "$1"/dmr/cat.c"
: catok

if \! -r $1/dmr/orig/dp.c goto dpno
   if \! -r $1/dmr/dp.c goto dpok
   echo "Removing "$1"/dmr/dp.c"
   rm -f $1/dmr/dp.c
   goto dpok
: dpno
echo "No backup found for "$1"/dmr/dp.c"
: dpok

if \! -r $1/dmr/orig/partab.c goto partabno
   if \! -r $1/dmr/partab.c goto partabok
   echo "Removing "$1"/dmr/partab.c"
   rm -f $1/dmr/partab.c
   goto partabok
: partabno
echo "No backup found for "$1"/dmr/partab.c"
: partabok

if \! -r $1/dmr/orig/vs.c goto vsno
   if ! -r $1/dmr/vs.c goto vsok
   echo "Removing "$1"/dmr/vs.c"
   rm -f $1/dmr/vs.c
   goto vsok
: vsno
echo "No backup found for "$1"/dmr/vs.c"
: vsok

if \! -r $1/dmr/orig/vt.c goto vtno
   if ! -r $1/dmr/vt.c goto vtok
   echo "Removing "$1"/dmr/vt.c"
   rm -f $1/dmr/vt.c
   goto vtok
: vtno
echo "No backup found for "$1"/dmr/vt.c"
: vtok
exit
