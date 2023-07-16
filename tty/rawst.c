#
/*
 * print raw terms info
 */

struct {
	int		t_speeds;	/* output and input line speeds */
	char		t_erase;	/* the character delete */
	char		t_kill;		/* the line delete */
	int		t_flags;	/* modes, settable via spcl fcn call */
	char		t_nldelay;	/* delay for newline */
	char		t_crdelay;	/* delay for cr */
	char		t_htab;		/* delay or size of horizontal tab */
	char		t_vtab;		/* delay or size of vertical motion */
	char		t_width;	/* maximum line length for folding */
	char		t_length;	/* maximum screen length for paging */
	int		t_breaktab[8];	/* break character map */
} t;

struct modes {
	int mask;
	char *txt;
} mod[] {
	01, "HUPCL",
	02, "XHTAB",
	04, "CAPSONLY",
	010, "ECHO",
	020, "CRMOD",
	040, "RAW",
	0100, "ODDP",
	0200, "EVENP",
	0400, "SCOPE",
	01000, "INDCTL",
	02000, "USERBREAK",
	04000, "ALL8BITS",
	010000, "HOLD",
	020000, "BREAKS",
	040000, "XVTAB",
	0100000, "DATA100",
	0,	0,
};

main()
{
	terms(0, 2, &t);

	printf("t_speeds %o\n", t.t_speeds);
	printf("t_erase %o\n", t.t_erase);
	printf("t_kill %o\n", t.t_kill);
	printf("t_flags %o\n", t.t_flags);
	prmodes(t.t_flags);
	printf("t_nldelay %d\n", t.t_nldelay);
	printf("t_crdelay %d\n", t.t_crdelay);
	printf("t_htab %d\n", t.t_htab);
	printf("t_vtab %d\n", t.t_vtab);
	printf("t_width %d\n", t.t_width);
	printf("t_length %d\n", t.t_length);

	/*	int		t_breaktab[8];	/* break character map */
}

prmodes(flags)
{
	register int fl;
	register struct modes *mp;
	int i;

	fl = flags;

	i = 0;
	for (mp = mod; mp->txt; mp++) {
		if (fl&mp->mask) printf("%s ", mp->txt);
		i++;
		if (i >= 6) {
			printf("\n");
			i = 0;
		}
	}
	printf("\n");
}
