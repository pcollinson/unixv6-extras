#
/*
 * getty -- adapt to terminal speed on dialup, and call login
 * Rewritten for new teletype driver 5th June '77 by E. S. Jones
 * Swansea University
 *
 * Reformatted to be more K&R - sorry
 * Use centralised set of values from /sys/deftty.h
 * remove bell, insert ; before login:
 * remove line lengths, terminal emulator can handle those
 * make all logins into SCOPE
 * remove XHTAB and XVTAB from setups
 * remove delays for '-' and '0' entries in /etc/ttys
 *
 * Peter Collinson
 * July 2023
 *
 */

#include "/usr/sys/deftty.h"

/* Popular combinations of flags */
/* was #define	INITIAL		XHTAB| ECHO| ODDP| EVENP| INDCTL| USERBREAK| BREAKS */
#define	INITIAL		ECHO| INDCTL| USERBREAK| BREAKS| SCOPE

/* was #define	FINAL		XHTAB| ECHO| ODDP| EVENP| INDCTL| HOLD| XVTAB */
#define	FINAL		ECHO| INDCTL| HOLD| SCOPE

#define	FINALEVEN	XHTAB| ECHO| EVENP| INDCTL| HOLD| XVTAB

/* Delays and tab sizes */
#define	INITDELAYS	10, 20, 8, 6
#define	ZERODELAY	 0,  0, 8, 6
#define	CRDELAY		 0, 10, 8, 6
#define	NLCRDELAYS	 5, 10, 8, 6
#define	DATA100DELAY	 2,  0, 8, 6
/* added */
 #define NODELAY 	 0,  0, 0, 0

/*
 * speeds
 */
#define	B110	3
#define	B150	5
#define	B300	7
#define B1200	9
#define B2400	11
#define B4800	12
#define	B9600	13

#define	SIGINT	2
#define	SIGQIT	3

/* Structure for terms, stty, gtty */
struct	{
	char	t_ispeed;
	char	t_ospeed;
	char	t_erase;
	char	t_kill;
	int	t_flags;
	char	t_nldelay;
	char	t_crdelay;
	char	t_htdelay;
	char	t_vtdelay;
	char	t_width;
	char	t_length;
	char	t_breaktab[16];
	}  tty;


char	breaks[]  {
	0377,	0377,	0373,	0377,
	0,	0,	0,	0,
	0,	0,	0,	0,
	0,	0,	0,	0
	};



struct	tab {
	int	name;		/* this table name */
	int	successor;	/* successor table name */
	int	iflags;		/* initial flags */
	int	inldelay;	/* initial delays */
	int	icrdelay;	/*   ""   */
	int	ihtdelay;	/*   ""   */
	int	ivtdelay;	/*   ""   */
	int	flags;		/* final flags */
	int	nldelay;	/* final delays */
	int	crdelay;	/*   ""   */
	int	htdelay;	/*   ""   */
	int	vtdelay;	/*   ""   */
	int	ispeed;		/* input speed */
	int	ospeed;		/* output speed */
	int	width;
	int	length;
	char	*message;	/* login message */
	}  table[]  {

/* table '-'   Console DECwriter */
	'-',	'-',
	INITIAL,	NODELAY,
	FINAL,		NODELAY,
	B300,		B300,
	0,		0,		/* was 132 */
	"\r\n;login: ",

/* table '0'   Anything at 300 or 110 baud with delays */
	'0',		1,
	INITIAL,	NODELAY,
	FINAL,		NODELAY,
	B300,		B300,
        0,		0,		/* was 80 */
	"\r\n;login: ",

	1,		-1,
	INITIAL,	INITDELAYS,
	FINAL,		NLCRDELAYS,
	B110,		B110,
	80,		0,
	"\r\n;login: ",

/* table '1'   DECwriters */
	'1',		'0',
	INITIAL,	INITDELAYS,
	FINAL,		ZERODELAY,
	B300,		B300,
	132,		0,
	"\r\n;login: ",

/* table '2'   Teletypes */
	'2',		'0',
	INITIAL| CAPSONLY, INITDELAYS,
	FINAL| CAPSONLY, CRDELAY,
	B110,		B110,
	80,		0,
	"\n\r\033;login: ",

/* table '3'   Slow CDC VDUs */
	'3',		'0',
	INITIAL,	INITDELAYS,
	FINAL| SCOPE,	ZERODELAY,
	B300,		B300,
	0,		16,
	"\n\r;login: ",

/* table '4'   Fast CDC VDUs */
	'4',		'0',
	INITIAL,	INITDELAYS,
	FINAL| SCOPE,	ZERODELAY,
	B1200,		B1200,
	0,		16,
	"\n\r;login: ",

/* table '5'   Data100s */
	'5',			'0',
	INITIAL| DATA100,	INITDELAYS,
	FINAL| DATA100,		DATA100DELAY,
	B1200,			B1200,
	0,			24,
	"\n\r;login: ",

/* table '6'	Fast Dacoll VDUs */
	'6',			'0',
	INITIAL,		INITDELAYS,
	FINALEVEN| SCOPE,	ZERODELAY,
	B1200,			B1200,
	80,			24,
	"\n\r;login: "
	};

#define	TABLELNG	sizeof table/sizeof table[0]
#define	initial		1
#define	final		0

#define	NAMELNG	16
#define	STTY	('t' <<8)| 3
char		name[NAMELNG];
struct tab	*tabp;
int		crmod, upper, lower;
char		*newline	"\n";

main(argc, argv)
int argc;
char **argv;
{
	int tname, code;

	signal(SIGINT, 0);
	signal(SIGQIT, 0);
	argc--;    argv++;
	tname = '0';
	if (argc > 0)
		tname = **argv;
	for (;;)  {
		for (tabp= table;  tabp< &table[TABLELNG];  tabp++)
			if (tabp->name == tname)
				break;
		if (tabp >= &table[TABLELNG])
			tabp = table;
		set_tty(initial);
		for (;;)  {
			puts(tabp->message);
			set_tty(initial);
			code = getname();
			if (code < 0)
				break;
			if (code == 0)
				continue;
			set_tty(final);
			execl("/bin/login", "login", name, 0);

			exit(1);
		}
		tname = tabp->successor;
		if (tname == -1)
			tname = **argv;
	}
}

set_tty(mode)
int	mode;
{
	register int i, *ptab;
	register char *ptty;
	int delays, flags;

	tty.t_ispeed = tabp->ispeed;
	tty.t_ospeed = tabp->ospeed;
	tty.t_erase = CERASE;
	tty.t_kill = CKILL;
	tty.t_flags = tabp->flags;
	ptty = &tty.t_nldelay;
	ptab = &tabp->nldelay;
	if (mode == initial)  {
		tty.t_flags = tabp->iflags;
		ptab = &tabp->inldelay;
		}
	for (i=0; i< 4; i++)  {
		*ptty = *ptab;
		ptty++;    ptab++;
		/* avoids compiler bug */
		}
	if (crmod)
		tty.t_flags =| CRMOD;
	if (upper  &&  !lower)
		tty.t_flags =| CAPSONLY;
	tty.t_width = tabp->width;
	tty.t_length = tabp->length;
	if (mode == initial)
		for (i= 0; i< 16; i++)
			tty.t_breaktab[i] = breaks[i];
	terms(0, STTY, &tty);
}

getname()
{
	register char *np;
	register c;
	static cs;

	crmod = 0;
	upper = 0;
	lower = 0;
	np = name;
	while ((c= getchar()) >=  ' '  &&  np< &name[NAMELNG])  {
		if ('a' <= c  &&  c <= 'z')
			lower++;
		else	if ('A' <= c  &&  c <= 'Z')
			upper++;
		*np++ = c;
		}
	*np = '\0';
	if (c == CEOF)
		exit(-1);
	if (c != '\n'  &&  c != '\r')
		return(c==CINTR?  -1: 0);
	if (c == '\r')  {
		crmod++;
		write(1, newline, 1);
	}
	if (upper  &&  !lower)
		for (np= name;  *np;  np++)
			if ('A' <= *np  &&  *np <= 'Z')
				*np =+ 'a' - 'A';
}

puts(as)
char	*as;
{
	register char *s;
	register int count;

	for (s= as;  *s;  s++)
		count++;
	write(1, as, count);
}
