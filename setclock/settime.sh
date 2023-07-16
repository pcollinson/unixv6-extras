cat </dev/ppt > /tmp/setdate
if ! -r /tmp/setdate exit
sh /tmp/setdate
rm -f /tmp/setdate
