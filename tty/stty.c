#
/*
 *
 * set teletype modes  -  rewritten August 77 for the GUCS terminal handler
 *
 * Author	       -  W. Findlay
 *		       -  Computing Science Department, Glasgow University
 *
 * Reformatted to be more K&R - sorry
 * Removed parity options - kernel is no longer using them
 * Use centralised set of values from /sys/deftty.h
 * Peter Collinson
 * July 2023
 */


#include "/usr/sys/deftty.h"

struct	termsdata  {
	char	ispeed;
	char	ospeed;
	char	erase;
	char	kill;
	int	modes;
	char	p[6];
	char	breaktable[16];
} t;

char	*speeds[]  {
	"0",
	"50",
	"75",
	"110",
	"134",
	"150",
	"200",
	"300",
	"600",
	"1200",
	"1800",
	"2400",
	"4800",
	"9600",
	"ext-a",
	"ext-b",
	0
};

char	*rates []   {
	"input",
	"output",
	"speed",
	0
};

char	*params []   {
	"nldelay",
	"crdelay",
	"tabs",
	"skips",
	"line",
	"page",
	0
};

char	*noparams []   {
	"!nldelay",
	"!crdelay",
	"!tabs",
	"!skips",
	"!line",
	"!page",
	0
};

struct {
	int actset;
	int actreset;
	} actions [] {
	0,	0,
	0,	0,
	0,	XHTAB,
	0,	XVTAB,
	0,	0,
	0,	0,
	0
};

struct {
	int noactset;
	int noactreset;
	} noactions [] {
	0,	0,
	0,	0,
	XHTAB,	0,
	XVTAB,	0,
	0,	0,
	0,	0,
	0
};

struct	{
	char	*string;
	int	set;
	int	reset;
}  modes[]  {

	"data100",	DATA100,	SCOPE,
	"!data100",	0,		DATA100,
	"breaks",	BREAKS,		0,
	"!breaks",	0,		BREAKS,
	"hold",		HOLD,		0,
	"!hold",	0,		HOLD,
	"8-bit",	ALL8BITS,	0,
	"!8-bit",	0,		ALL8BITS,
	"!delimit",	0,		USERBREAK,
	"indicate",	INDCTL,		0,
	"!indicate",	0,		INDCTL,
	"vdu",		SCOPE,		0,
	"!vdu",		0,		SCOPE,
/* pc remove parity
 *	"even",		EVENP,		0,
 *	"!even",	0,		EVENP,
 *	"odd",		ODDP,		0,
 *	"!odd",		0,		ODDP, */
	"!raw",		0,		RAW,
	"!nl",		CRMOD,		0,
	"nl",		0,		CRMOD,
	"echo",		ECHO,		0,
	"!echo",	0,		ECHO,
	"UPPER",	CAPSONLY,	0,
	"!LOWER",	CAPSONLY,	0,
	"upper",	CAPSONLY,	0,
	"!lower",	CAPSONLY,	0,
	"!UPPER",	0,		CAPSONLY,
	"LOWER",	0,		CAPSONLY,
	"!upper",	0,		CAPSONLY,
	"lower",	0,		CAPSONLY,
	"hupcl",	HUPCL,		0,
	"!hupcl",	0,		HUPCL,
	0
};

char	*arg;

int	pos, width;

char	eraserep[8], killrep[8];

char	terminal[9]	"/dev/tty?";

main(argc, argv)
char *argv[];
{
	register int i, j, argval;
	int tty, argn;
	char mytty, quiet;

	quiet = pos = 0;
	argn = 1;
	tty = 2;
	mytty = terminal[8] = ttyn(tty);

	if (mytty == '?') {
		mytty = terminal[8] = '\0';
		tty = open(terminal, 2);
		if (tty < 0) {
			printf("stty : Cannot find your terminal\n");
			exit(-1);
		}
	}

	terms(tty, 2, &t);
	width =  truncated(t.p[4]);

	while (--argc > 0) {

		arg = *++argv;

		if (eq("quiet",argv)) quiet++;

		if (eq("!quiet",argv)) quiet = 0;

		if (eq("ek",argv))   {
			t.kill = CKILL;
			t.erase = CERASE;
		};

		if (eq("erase",argv))   {
			++argv;
			t.erase = **argv;
			argc--;
		};

		if (eq("kill",argv))   {
			++argv;
			t.kill = **argv;
			argc--;
		};

		if (eq("tty",argv))   {
			++argv;
			terminal[8] = **argv;
			if (argn > 1)   {
				printf("stty: tty %c must be first\n",
					  terminal[8]);
				exit(-1);
			}
			tty =  open(terminal, 2);
			if (tty >= 0)   {
				terms(tty, 2, &t);
				argc--;
				if (argc == 1)   {
					prmodes();
					exit(0);
				};
			}
			else 	{
				printf("stty: No access to %s\n",
							terminal);
				exit(-1);
			}
		}

		for (i= 0;  rates[i];  i++)
			if (eq(rates[i], argv))   {
				argc--;
				arg = *++argv;
				for (j= 0;  speeds[j];  j++)
					if (eq(speeds[j], argv))
						switch(i)   {
						case 0:
							t.ispeed = j;
							break;
						case 2:
							t.ispeed = j;
						case 1:
							t.ospeed = j;
							break;
						}
			};

		for (i= 0;  params[i];  i++)
			if (eq(params[i], argv))  {
				argc--;
				arg = *++argv;
				argval =  get_int(*argv);
				if (argval > 255)   {
					arg_err(params[i]);
					exit(-1);
				}
				else	{
					if (argval>=0) {
						t.p[i] = argval;
						t.modes =& ~actions[i].actreset;
						t.modes =|  actions[i].actset;
					}
					else	{
						num_err("must", *--argv);
						exit(-1);
					}
				}
			};

		for (i= 0;  noparams[i];  i++)
			if (eq(noparams[i], argv)) {
				argc--;
				arg = *++argv;
				argval = get_int(*argv);
				if (argval > 255) {
					arg_err(noparams[i]);
					exit(-1);
				}
				else	{
					if (argval < 0 || i == 2 || i == 3) {
						if (argval == -2) {
							argv--;
							argc++;
						};
						t.p[i] = (argval >= 0 ? argval : 0);
						t.modes =& ~noactions[i].noactreset;
						t.modes =|  noactions[i].noactset;
					}
					else	{
						num_err("must not", *--argv);
						exit(-1);
					}
				}
			};

		for (i= 0;  modes[i].string;  i++)
			if (eq(modes[i].string, argv))  {
				if (modes[i].set &ALL8BITS
						&&  terminal[8] == mytty)  {
					printf("stty: Won't set 8-bit mode %s",
						"on your own terminal\n");
					exit(-1);
				};
				t.modes =& ~modes[i].reset;
				t.modes =| modes[i].set;
				break;
			};

		if (arg)  {
			printf("stty : Unknown mode -  %s\n", arg);
			exit(-1);
		};
		argn++;
	}

	terms(tty, 3, &t);
	terms(tty, 2, &t);

	if (!quiet) {
		if (terminal[8] == mytty)
			width =  truncated(t.p[4]);
		prmodes();
	};

}

prmodes()
{
	register int i, m;

	if (t.ispeed != t.ospeed)  {
		prspeed("input (speed) ", t.ispeed);
		prspeed("output (speed) ", t.ospeed);
	}
	else	prspeed("speed ", t.ispeed);

	printf("\nerase %s;  kill %s;\n",  legible(t.erase, eraserep),
			legible(t.kill, killrep));

	m = t.modes;

	if (m &DATA100)
		prmode("data100 (tough)");
	if (m &SCOPE)
		prmode("vdu (erasing)");
	if (m &BREAKS)
		prmode("breaks (=^c)");
	if (m &ALL8BITS)  {
		prmode("8-bit (i/o path)");
		if (m &HOLD)
			prmode("hold (on ^s/^q)");
	};
	if (m &USERBREAK)
		prmode("delimit (user-specified)");
	if (m &RAW)
		prmode("raw");
	if (m &ECHO)
		prmode("echo");
	if (m &HUPCL)
		prmode("hupcl (hangup on close)");
	if (m &INDCTL)
		prmode("indicate (show ^ control chs)");
	newline();
	/* pc remove parity
	if (m &EVENP)
		prmode("even (parity allowed)");
	if (m &ODDP)
		prmode("odd (parity allowed)");
	newline();
	*/
	if (m &CRMOD)
		prmode("!nl (CR for newline)");
	if (m &CAPSONLY)
		prmode("!lower (soft lower-case)");
	prparam(t.p[2]&0377, (m &XHTAB ? "!tabs (every)" : "tabs (delay)"));
	prparam(t.p[3]&0377, (m &XVTAB ? "!skips (size)" : "skips (delay)"));
	newline();
	for (i= 0;  i < 6;  i++)
		if (i!=2 && i!=3)
			prparam(t.p[i]&0377, params[i]);
	newline();

}

prparam(m, s)
int m;
char *s;
{
	if (m != 0)  {
		position(7+ length(s));
		printf("%s %d;  ", s, m);
	};
}

prspeed(c, s)
{
	if (s < 0 ||  s > 15)
		s = 0;
	printf("%s %s baud;  ", c, speeds[s]);
}

prmode(s)
char *s;
{
	position(3+ length(s));
	printf("%s;  ", s);
}

position(c)
{
	if (pos +c > width)  {
		printf("\n");
		pos = 0;
	};
	pos =+ c;
}

truncated(w)
int w;
{
	w =& 0377;
	w =  (w==0 ? 80 : w);
	return(w<=80 ? w : 80);
}


length(as)
char *as;
{
	register char	*s;
	register int	l;

	s = as;
	l = 0;
	for (l= 0;  *s++ != 0;  ++l);
	return(l);

}

newline()
{
	if (pos != 0)
		printf("\n");
	pos = 0;
}

legible(c, s)
char c;
char *s;
{
	int	i;

	s[0] = '^';
	if (c < ' ')
		s[1] =  c | '\140';
	else  if (c == 0177)
		for (i= 0;  i < 7;  s[i]= "delete"[i++]);
	else 	{
		s[0] =  c &0177;
		s[1] = '\0';
		};
	return(s);
}

get_int(ap)
char *ap;
{

	register char *p;
	register int n;
	register char c;

	arg = 0;
	p = ap;
	if (p == -1) return(-1);
	if ((c = *p++)<'0' || c>'9') return(-2);
	n = 0;
	do 	{ n =  n*10 + (c-'0'); }
		while((c = *p++) >= '0'  &&  c <= '9');
	return(n);
}

arg_err(s)
char *s;
{
	printf("stty: Argument %s > 255\n", s);
	return;
}

num_err(i, s)
char *i, *s;
{
	printf("stty: %s %s have a numeric argument\n", s, i);
	return;
}

eq(string, argv)
char *string;
char **argv;
{

	register char	*p, *q;

	if (!arg)
		return(0);
	p = string;
	q = *argv;

	do
		if (*q != *p++)
			return(0);
	while (*q++ != '\0');

	arg = 0;
	return(1);
}
