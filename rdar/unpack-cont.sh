#!/bin/sh
# find all the cont.a files in a tree at .
# travel down to the directory, unpack it
# delete cont.a
find . -name cont.a -print | while read fname; do
     dname=$(dirname $fname)
     ( cd $dname; rdar cont.a -x; rm cont.a )
done
