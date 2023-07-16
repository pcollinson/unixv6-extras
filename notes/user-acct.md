# Creating a user account

When V6 is first installed, you can login with _root_ and get super-user status. However, you probably want somewhere to install files and run them. The V6 shell will run commands in the current directory, _/bin_ and _/usr/bin_ in that order. So having a home to put stuff is the way to go. You may want to install a simple shell script using _stty_ see [Using stty](using-stty.md) to set up the terminal for you.

To create a user account, first choose a name say _fred_, it needs to be lower case and should be a maximum of 8 characters. Now edit _/etc/passwd_ and add

```
fred::7:1::/usr/fred:
```

This gives you a uid of 7, and puts you in the 'other` group.

Now change to _/usr_ and type (there's a prompt before every line)

``` sh
mkdir fred
chown 7 fred
chgrp 1 fred
chmod 777 fred
```

you are set. Logout with ^d, and login as _fred_ with no password.


When you login you'll find yourself in your new home directory. You can still become root by typing _su_.  Beware that the root file system where your home directory lives doesn't have a lot of free space. The _df_ command tells you how many free 512 byte blocks are available on the filesystem. It can be a bad idea to run out of disk space. Running out of space on a root file system can become a huge problem. I am using an empty `RK05` mounted on _/dev/rk3_ for substantial  work. See [../mkfs](../mkfs/README.md) for help with formatting an RK05.
