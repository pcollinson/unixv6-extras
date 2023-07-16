# Using stty to change terminal control characters

With the vanilla system, you cannot use DELETE to delete characters because it's compiled into the kernel to generate an interrrupt signal. However, you can change the erase character to ^H, which many people can use to delete a character. This has the benefit(?) of echoing a backspace character, which can delete the character on the screen. So at the outset, there's mileage in creating a shell script and putting it somewhere that you can get at.

The script goes something like:

``` sh
stty erase ^H cr0 nl0
```

when you type this, ^H will echo a backspace, which can be confusing. Check that the file has what you want by using ```od -c``` on it.

The real solution to making the terminal work more like the ones you are used is to change add the new terminal driver to the kernel - see [../tty](../tty/README.md).
