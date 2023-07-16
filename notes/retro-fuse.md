# Using retro-fuse

Retro-fuse is a plug-in for the FUSE library which provides an API to access to filesystems from user space.  You can find _retro-fuse_ on [github](https://github.com/jaylogue/retro-fuse). Fuse is on [github](https://github.com/libfuse/libfuse) and for the Mac on [OSXFUSE](https://osxfuse.github.io).

It was very easy to install out of the box or two boxes - you need to install the FUSE library first. Once this is inplace (and there are some magic permission stuff to happen on recent MacOS), then in *retro_fuse*, you can type:

``` sh
make v6fs
```
and then use that as a _mount_ command to mount one of your _rk0_ disks onto a directory to get access to the files as a filesystem.

You use the standard _umount_ call to remove the linkage.

It's using the same code from the V6 filesystem to access the disk that you are running inside the simulator.

It provides easy access to V6 file systems. I've used it when I want to use modern tools to search for information on the V6 file system or to copy files out.

However, from a Mac there are some problems writing files. This is down MacOS and not the retro-fuse system. Mac will write a 'double' file called something like ```._filename```, there appears to be no way to stop this in the mount system or MacOS. I also had some problems with the MacOS extended attributes which are beginning to be used for lots of security checking.

Finally, I'm a little unsure whether it's wise to use a mounted _retro-fuse_  filesystem system at the same time as running the simulator with active V6 system.
