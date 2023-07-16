: Script to edit the system files that require removal or replacing
: old files are placed in the appropriate orig directory
: For safety the script should be supplied with the name of the
: system file tree

: no way of testing for empty argument strings
: If empty this fails with cannot find

if -r $1 goto dirok
   echo "Usage: call this script with the name of the system directory - perhaps /usr/sys"
   exit

: dirok

echo '*** Step 1: make orig directories and copy backup files'
if -r $1/dmr/orig goto dmrok
   echo "Making "$1"/dmr/orig":
   mkdir $1/dmr/orig
   if -r $1/dmr/orig goto dmrok
      echo "Failed to make "$1"/dmr/orig - stopping"
   exit
: dmrok

if -r $1/ken/orig goto kenok
   echo "Making "$1"/ken/orig":
   mkdir $1/ken/orig
   if -r $1/ken/orig goto kenok
      echo "Failed to make "$1"/ken/orig - stopping"
   exit
: kenok

if -r $1/conf/orig goto confok
   echo "Making "$1"/conf/orig":
   mkdir $1/conf/orig
   if -r $1/conf/orig goto confok
      echo "Failed to make "$1"/conf/orig - stopping"
   exit
: confok

if -r $1/orig goto rootok
   echo "Making "$1"/orig":
   mkdir $1/orig
   if -r  $1/orig goto rootok
      echo "Failed to make "$1"/orig - stopping"
   exit
: rootok

: dmr/cat.c
if -r $1/dmr/orig/cat.c goto catend
   : no - is it in dmr? If not ignore
   if \! -r $1/dmr/cat.c goto catend
      : no copy needed
      cp $1/dmr/cat.c $1/dmr/orig/cat.c
      : did the copy work?
      if -r $1/dmr/orig/cat.c goto catok
	 echo "Failed: cp "$1"/dmr/cat.c "$1"/dmr/orig/cat.c"
	 goto catend
: catok
	echo "Success: cp "$1"/dmr/cat.c "$1"/dmr/orig/cat.c"
: catend


: dmr/dc.c
if -r $1/dmr/orig/dc.c goto dcend
   : no - is it in dmr? If not ignore
   if \! -r $1/dmr/dc.c goto dcend
      : no copy needed
      cp $1/dmr/dc.c $1/dmr/orig/dc.c
      : did the copy work?
      if -r $1/dmr/orig/dc.c goto dcok
	 echo "Failed: cp "$1"/dmr/dc.c "$1"/dmr/orig/dc.c"
	 goto dcend
: dcok
	echo "Success: cp "$1"/dmr/dc.c "$1"/dmr/orig/dc.c"
: dcend


: dmr/dh.c
if -r $1/dmr/orig/dh.c goto dhend
   : no - is it in dmr? If not ignore
   if \! -r $1/dmr/dh.c goto dhend
      : no copy needed
      cp $1/dmr/dh.c $1/dmr/orig/dh.c
      : did the copy work?
      if -r $1/dmr/orig/dh.c goto dhok
	 echo "Failed: cp "$1"/dmr/dh.c "$1"/dmr/orig/dh.c"
	 goto dhend
: dhok
	echo "Success: cp "$1"/dmr/dh.c "$1"/dmr/orig/dh.c"
: dhend


: dmr/dhdm.c
if -r $1/dmr/orig/dhdm.c goto dhdmend
   : no - is it in dmr? If not ignore
   if \! -r $1/dmr/dhdm.c goto dhdmend
      : no copy needed
      cp $1/dmr/dhdm.c $1/dmr/orig/dhdm.c
      : did the copy work?
      if -r $1/dmr/orig/dhdm.c goto dhdmok
	 echo "Failed: cp "$1"/dmr/dhdm.c "$1"/dmr/orig/dhdm.c"
	 goto dhdmend
: dhdmok
	echo "Success: cp "$1"/dmr/dhdm.c "$1"/dmr/orig/dhdm.c"
: dhdmend


: dmr/dp.c
if -r $1/dmr/orig/dp.c goto dpend
   : no - is it in dmr? If not ignore
   if \! -r $1/dmr/dp.c goto dpend
      : no copy needed
      cp $1/dmr/dp.c $1/dmr/orig/dp.c
      : did the copy work?
      if -r $1/dmr/orig/dp.c goto dpok
	 echo "Failed: cp "$1"/dmr/dp.c "$1"/dmr/orig/dp.c"
	 goto dpend
: dpok
	echo "Success: cp "$1"/dmr/dp.c "$1"/dmr/orig/dp.c"
: dpend


: dmr/kl.c
if -r $1/dmr/orig/kl.c goto klend
   : no - is it in dmr? If not ignore
   if \! -r $1/dmr/kl.c goto klend
      : no copy needed
      cp $1/dmr/kl.c $1/dmr/orig/kl.c
      : did the copy work?
      if -r $1/dmr/orig/kl.c goto klok
	 echo "Failed: cp "$1"/dmr/kl.c "$1"/dmr/orig/kl.c"
	 goto klend
: klok
	echo "Success: cp "$1"/dmr/kl.c "$1"/dmr/orig/kl.c"
: klend


: dmr/partab.c
if -r $1/dmr/orig/partab.c goto partabend
   : no - is it in dmr? If not ignore
   if \! -r $1/dmr/partab.c goto partabend
      : no copy needed
      cp $1/dmr/partab.c $1/dmr/orig/partab.c
      : did the copy work?
      if -r $1/dmr/orig/partab.c goto partabok
	 echo "Failed: cp "$1"/dmr/partab.c "$1"/dmr/orig/partab.c"
	 goto partabend
: partabok
	echo "Success: cp "$1"/dmr/partab.c "$1"/dmr/orig/partab.c"
: partabend


: dmr/tty.c
if -r $1/dmr/orig/tty.c goto ttyend
   : no - is it in dmr? If not ignore
   if \! -r $1/dmr/tty.c goto ttyend
      : no copy needed
      cp $1/dmr/tty.c $1/dmr/orig/tty.c
      : did the copy work?
      if -r $1/dmr/orig/tty.c goto ttyok
	 echo "Failed: cp "$1"/dmr/tty.c "$1"/dmr/orig/tty.c"
	 goto ttyend
: ttyok
	echo "Success: cp "$1"/dmr/tty.c "$1"/dmr/orig/tty.c"
: ttyend


: dmr/vs.c
if -r $1/dmr/orig/vs.c goto vssend
   : no - is it in dmr? If not ignore
   if \! -r $1/dmr/vs.c goto vssend
      : no copy needed
      cp $1/dmr/vs.c $1/dmr/orig/vs.c
      : did the copy work?
      if -r $1/dmr/orig/vs.c goto vsok
	 echo "Failed: cp "$1"/dmr/vs.c "$1"/dmr/orig/vs.c"
	 goto vssend
: vsok
	echo "Success: cp "$1"/dmr/vs.c "$1"/dmr/orig/vs.c"
: vssend


: dmr/vt.c
if -r $1/dmr/orig/vt.c goto vtend
   : no - is it in dmr? If not ignore
   if \! -r $1/dmr/vt.c goto vtend
      : no copy needed
      cp $1/dmr/vt.c $1/dmr/orig/vt.c
      : did the copy work?
      if -r $1/dmr/orig/vt.c goto vtok
	 echo "Failed: cp "$1"/dmr/vt.c "$1"/dmr/orig/vt.c"
	 goto vtend
: vtok
	echo "Success: cp "$1"/dmr/vt.c "$1"/dmr/orig/vt.c"
: vtend


: ken/sysent.c
if -r $1/ken/orig/sysent.c goto sysentend
   : no - is it in ken? If not ignore
   if \! -r $1/ken/sysent.c goto sysentend
      : no copy needed
      cp $1/ken/sysent.c $1/ken/orig/sysent.c
      : did the copy work?
      if -r $1/ken/orig/sysent.c goto sysentok
	 echo "Failed: cp "$1"/ken/sysent.c "$1"/ken/orig/sysent.c"
	 goto sysentend
: sysentok
	echo "Success: cp "$1"/ken/sysent.c "$1"/ken/orig/sysent.c"
: sysentend


: conf/m40.s
if -r $1/conf/orig/m40.s goto m40end
   : no - is it in conf? If not ignore
   if \! -r $1/conf/m40.s goto m40end
      : no copy needed
      cp $1/conf/m40.s $1/conf/orig/m40.s
      : did the copy work?
      if -r $1/conf/orig/m40.s goto m40ok
	 echo "Failed: cp "$1"/conf/m40.s "$1"/conf/orig/m40.s"
	 goto m40end
: m40ok
	echo "Success: cp "$1"/conf/m40.s "$1"/conf/orig/m40.s"
: m40end


: tty.h
if -r $1/orig/tty.h goto ttyhend
   : no - is it in none? If not ignore
   if \! -r $1/tty.h goto ttyhend
      : no copy needed
      cp $1/tty.h $1/orig/tty.h
      : did the copy work?
      if -r $1/orig/tty.h goto ttyhok
	 echo "Failed: cp "$1"/tty.h "$1"/orig/tty.h"
	 goto ttyhend
: ttyhok
	echo "Success: cp "$1"/tty.h "$1"/orig/tty.h"
: ttyhend
exit
