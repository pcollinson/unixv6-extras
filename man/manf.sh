: Script to run the man macros on a file
if -r $1 goto havearg
   echo "Usage: manf file"
exit
: havearg
if -r /usr/bin/ssp goto withssp
   nroff /usr/doc/man/man0/naa-man $1
   exit
: withssp
   nroff /usr/doc/man/man0/naa-man $1 | ssp
   exit
