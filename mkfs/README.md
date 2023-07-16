# Create a filesystem on an RKO5 disk

The _rk.proto_ file is a proto file for _mkfs_ to create a file system on an RK05.

``` sh
/etc/mkfs /dev/rkXX rk.proto

```
The $ at the end of the file is important.

Note that it's intended to make a system disk that uses blocks from 0 to 3999 inclusive. The 872 blocks starting at 4000 are used for swapping.
