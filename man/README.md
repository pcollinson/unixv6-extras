# Display UNIX manual pages -_man_

The _man_ command is missing from the standard Unix V6 release.

The code here was extracted from the University of California's 1Bsd release. It was written by Bill Joy.

It uses a program also written by Bill Joy -_ ssp_ - single space output, that removes successive blank lines from its standard input. The program is used automatically when output is to the terminal, or not otherwise.

Changes are to the man.c code:

*  Used  a _#define_ to define the base path to the manual pages
*  comment out logging of man requests.
*  Fix declarations at the end of the code.

V6 man pages use different _nroff_ macros from later versions of the system. There is the problem of getting working macros that will render the page without some of the tools that understand backspacing and upward/downward motions. I've  adapted the standard _naa_ macro set into _naa-man_.

### Compilation

To compile the two programs separately use _compile_man_ that compiles _man.c_ command into _a.out_.  _compile_ssp_ compiles _ssp_ to _a.out_.

### Installation

To  compiles both programs and install them in _/bin_, use _install_. It will also install _naa-man_  in _/usr/doc/man/man0_.
Manual pages follows UCB practice and they are installed in section 6 of the manual.

### Change to ssp

I added some filtering into _ssp_ to remove the vertical movement characters that is output from some pages.

### Extra command

I found it useful to be able to run the macros over a file in the current directory, _manf_ will do this.

### Running on the host

It seems that current versions of _groff_ can be used with the macros in _/usr/doc/man0/taa_. However, the page size is built in and I failed to get them to work with an A4 page.
