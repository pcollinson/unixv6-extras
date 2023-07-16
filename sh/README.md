# Add _cd_ to the UNIX shell

The code here is the standard Unix V6 shell.

It has one change:  _cd_ is now a synonym for _chdir_.

Use _compile_ to compile and _install_ to install.

The _install_ script will preserve the old binary in _/usr/bin/sh.bak_, unless that file exists.

The _orig_ directory retains the original code, to aid with _diff_ should that be needed.

This was pretty much the first thing I did when I started this project.
