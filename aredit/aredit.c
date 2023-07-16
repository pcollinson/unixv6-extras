#
/*
 * aredit - program to allow somewhat better editing of ar lib files
 *	    yes this is available in v7 ar but I decided not to engage
 *	    with the problems of backporting into this C compiler
 *
 * aredit cmd libfile.a marker.o append.o
 *
 * Cmds:
 * i	insert append.o before the marker.o file
 * a	append append.o after the marker.o file
 * ci	Insert append.o and change marker.o
 * ca	Replace marker.o and change append.o
 * p	Add to other commands to preserve temporary files
 *
 * The strategy is to completely unpack the libfile.a into a
 * temporary directory below the current one.
 * Then reconstruct the archive in a local file before copying
 * it into place
 *
 * Peter Collinson
 * July 2023
 */

/* number of entries we can deal with */
#define ENTRIES 200

/* name and order of entries */
char *entries[ENTRIES];
char **nexte;

/* string storage */
#define STRBLK 512	/* chunks of memory added */
char *cbuf;		/* content storage buffer */
char *cnext;		/* next location */

/* Names of the files */
char *dotarlib;		/* pointer to ../libname */
char *arlib;		/* pointer to the libname */
char *dotappnd;		/* pointer to ../append */
char *appnd;		/* pointer to append */
char *dotmarkr;		/* pointer to ../marker */
char *markr;		/* pointer to marker */
char *dotnllib;		/* pointer to temporary lib.a */
char *nllib;		/* name of temporary library */

/* directory for unpacking */
char *unpackd		"aredit.tmp";
/* File in unpackd for new archive */
#define TEMPLIB		"templib.a"

/* flags */
#define FINSERT 	01
#define FAPPEND		02
#define FMASK		03
#define FREPLACE        04
#define FPRESERVE	010
int flag;

struct {
	int	fdes;
	int	nleft;
	char	*nextc;
	char	buff[512];
} inf;

char *usage "Usage: aredit [c][ia][p] libfile marker append\n";

main(argc, argv)
int argc;
char **argv;
{
	register char *p;

	argc--;
	argv++;
	if (argc != 4) {
		printf(usage);
		exit(1);
	}
	/* command character */
	for (p = argv[0]; *p; p++) {
		switch (*p) {
		case 'c': flag =| FREPLACE;
			/* was r - but a mistype of ar */
			/* for artest could be bad */
			  break;
		case 'i': flag =| FINSERT;
			  break;
		case 'a': flag =| FAPPEND;
			  break;
		case 'p': flag =| FPRESERVE;
			  break;
		default:
			  printf(usage);
			  exit(1);
		}
	}
	if ((flag & FMASK) == 0
	    || (flag & FMASK) == FMASK) {
		printf(usage);
		exit(1);
	}
	/* set up names */
	dotarlib = namesav(argv[1]);
	arlib = dotarlib + 3;
	dotmarkr = namesav(argv[2]);
	markr = dotmarkr + 3;
	dotappnd = namesav(argv[3]);
	appnd = dotappnd + 3;
	dotnllib = namesav(TEMPLIB);
	nllib = dotnllib + 3;

	/*
	 * check all the files exist
	 */
	if (exists(arlib) < 0) {
		printf("Cannot open %s\n", arlib);
		exit(1);
	}
	if ((flag&FREPLACE) && exists(appnd) < 0) {
		printf("Cannot open %s\n", appnd);
		exit(1);
	}
	if (exists(markr) < 0) {
		printf("Cannot open %s\n", markr);
		exit(1);
	}
	/*
	 * now do the work
	 */
	process();
	exit(0);
}

process()
{
	/* Make the temporary directoory */
	mktmpdir();
	/* move into it */
	printf("Changing dir into %s\n", unpackd);
	if (chdir(unpackd) < 0) {
		syserr("Chdir failure");
		exit(1);
	}
	/* clean the directory */
	cleandir();
	/* extract the files and create the list */
	extract();

	/* copy the files into the directory */
	if (flag & FREPLACE)
		copyfile(dotmarkr, markr);
	copyfile(dotappnd, appnd);
	/* print all the names */
	/* prnames(); */
	/* rebuild the library */
	rebuild();
	/* copy over original file */
	copyfile(nllib, dotarlib);
	/* cleaning up */
	if ((flag&FPRESERVE) == 0) {
		printf("Removing temporary contents\n");
		cleandir();
		printf("Change dir to parent\n");
		if (chdir("..") < 0) {
			syserr("Chdir failure");
			exit(1);
		}
		printf("Removing %s\n", unpackd);
		if (rmdir(unpackd) < 0) {
			printf("Failed to remove %s\n", unpackd);
		}
	}
}

/* debug code - print the list of names */
prnames()
{
	char **lscan;

	for (lscan = entries; *lscan; lscan++) {
		printf("%s\n", *lscan);
	}
}

/* make the working directory */
mktmpdir()
{
	int ret;

	printf("Making %s\n", unpackd);
	ret = mkdir(unpackd);
	if (ret < 0) {
		printf("Cannot make %s\n", unpackd);
		exit(1);
	}
}

/* remove all the files from the current directory */
cleandir()
{
	static struct {
		int ino;
		char name[14];
	} dentry;
	register char *p;
	register int j;
	char *fname;

	printf("Cleaning %s\n", unpackd);
	if (fopen(".", &inf) < 0) {
		printf("Cannot open %s\n", unpackd);
		exit(1);
	}
	for (;;) {
		p = &dentry;
		for (j = 0; j < 16; j++)
			*p++ = getc(&inf);
		if (dentry.ino == 0
		    || (dentry.name[0] == '.'
			&& (dentry.name[1] == '\0'
			    || (dentry.name[1] == '.'
				&& dentry.name[2] == '\0')
			    )
			)
		    ) continue;
		if (dentry.ino == -1)
			break;
		fname = makename(dentry.name);
		if (unlink(fname) < 0) {
			printf("Cannot unlink %s\n", fname);

			exit(1);
		}
	}
	close(inf.fdes);
}

/*
 * extract the files into the directory
 * and store the names
 */
extract()
{
	int i, status, pvec[2], fd0, c;
	char *bp;
	static char buf[64];

	printf("Extracting %s contents\n", dotarlib);
	pipe(pvec);
	i = fork();
	if (i == -1) {
		printf("Cannot fork\n");
		exit(1);
	}
	if (i == 0) {
		/* child - write to output side of pipe */
		close(1);
		dup(pvec[1]);
		close(0);
		close(pvec[0]);
		execl("/bin/ar", "ar", "xv", dotarlib, 0);
		exit(1);
	}
	/* save fd 0 */
	fd0 = dup(0);
	/* close the output side of the pipe */
	close(pvec[1]);
	/* use pipe 0 as stdin */
	close(0);
	dup(pvec[0]);
	bp = buf;
	nexte = entries;
	while ((c = getchar()) != 0) {
		if (c != '\n') {
			if ((bp - buf) > sizeof(buf)) {
				printf("Line too long from ar\n");
				exit(1);
			}
			*bp++ = c;
			*bp = '\0';
			continue;
		}
		if (bp == buf) continue;
		if (buf[0] == 'x' && buf[1] == ' ') {
			*nexte++ = strsav(&buf[2]);
			if (nexte >= &entries[ENTRIES]) {
				printf("Out of storage space for archive entries\n");
				printf("Recompile the code changing ENTRIES\n");
				exit(1);
			}
			*nexte = 0;
			bp = buf;
		}
	}
	/* put stdin back */
	close(0);
	dup(fd0);
	close(fd0);
	/* pick up the death of ar */
	while (wait(&status) != -1)
		continue;
}

rebuild()
{
	register char **lscan;

	printf("Rebuilding archive into %s\n", nllib);

	for (lscan = entries; *lscan; lscan++) {
		if (streq(*lscan, markr)) {
			printf("Found %s\n", markr);
			if (flag&FINSERT) {
				printf("Insert %s\n", appnd);
				aradd(appnd);
			}
			aradd(markr);
			if (flag&FAPPEND) {
				printf("Append %s\n", appnd);
				aradd(appnd);
			}
		} else if (streq(*lscan, appnd)) {
			printf("%s already found in %s, added by %s command\n",
			       appnd, arlib, visflag());
		} else
			aradd(*lscan);
	}
}

visflag()
{
	if (flag&FINSERT) return("insert");
	return("append");
}

/* use ar to add a file */
aradd(file)
char *file;
{
	int i, status;

	i = fork();
	if (i == -1) {
		printf("Cannot fork\n");
		exit(1);
	}
	if (i == 0) {
		/* child run ar */
		execl("/bin/ar", "ar", "r", nllib, file, 0);
	}
	wait(&status);
	if (status != 0) {
		printf("ar insert fail\n");
		exit(1);
	}
}

copyfile(from, to)
char *from;
char *to;
{
	int fin, fout, ct;
	char buf[512];

	printf("Copying %s to %s\n", from, to);
	if ((fin = open(from, 0)) < 0) {
		printf("Cannot open: %s for reading\n", from);
		exit(1);
	}
	if ((fout = creat(to, 0664)) < 0) {
		printf("Cannot open: %s for writing\n", to);
		exit(1);
	}
	while ((ct = read(fin, buf, sizeof(buf))) != 0) {
		if (ct < 0) {
			printf("Read error on %s\n", from);
			exit(1);
		}
		if (write(fout, buf, ct) < 0)
			printf("Wrote error on %s\n", to);
	}
	close(fin);
	close(fout);
}


makename(file)
char *file;
{
	static char fname[16];
	register char *dp, *sp;
	register int i;

	dp = fname;
	sp = file;
	for (i = 0; i < 14; i++) {
		*dp++ = *sp++;
		if (*sp == '\0')
			break;
	}
	*dp = 0;
	return (fname);
}

exists(fname)
char *fname;
{
	int fd;

	if ((fd = open(fname, 0)) < 0) {
		return(0);
	}
	close(fd);
	return(1);
}


mkdir(dname)
char *dname;
{
	int pid;
	int retp;
	int status;

	if (exists(dname))
		return(0);

	pid = fork();
	if (pid < 0) {
		printf("Cannot fork for mkdir\n");
		exit(1);
	}
	if (pid == 0) {
		execl("/bin/mkdir", "mkdir", dname, 0);
	}
	retp = wait(&status);
	return (exists(dname));
}

rmdir(dname)
char *dname;
{
	int pid;
	int retp;
	int status;

	if (!exists(dname))
		return(0);

	pid = fork();
	if (pid < 0) {
		printf("Cannot fork for rmdir\n");
		exit(1);
	}
	if (pid == 0) {
		execl("/bin/rmdir", "rmdir", dname, 0);
	}
	retp = wait(&status);
	return (exists(dname));
}

/* manage string storage */
char *
strsav(src)
char *src;
{
	int need;
	register char *cstart;
	register char *cend;

	need = strlen(src) + 1;

	if (cbuf == 0 || (&entries[STRBLK] - cnext) <= need) {
		cbuf = alloc(STRBLK);
		cnext = cbuf;
	}
	cstart = cnext;
	strcpy(cstart, src);
	cnext = cstart + need;
	return (cstart);
}

/* store a filename - adding ../ to the start */
char *
namesav(src)
char *src;
{
	register int len;
	register char *d;
	char strbuf[64];

	len = strlen(src);
	d = strbuf;
	strcpy(d, "../");
	strcat(d, src);
	return(strsav(d));
}

/*
 * String management routines
 */
strcpy(to, from)
char *to, *from;
{
	register char *t,*f;

	t = to;
	f = from;
	while (*t++ = *f++)
		continue;
	*t = '\0';
}

strcat(after, with)
char *after, *with;
{
	register char *rafter;

	rafter = after;

	while (*rafter)
		rafter++;
	strcpy(rafter, with);
}

/*
 * perror has a strlen
 */
int
strlen(src)
char *src;
{
	register char *rsrc;
	register int len;

	rsrc = src;
	len = 0;
	while (*rsrc++) {
		len++;
	}
	return(len);
}

/*
 * simple string compare
 * returns 1 if equal
 *         0 if not
 */
int
streq(aa, bb)
char *aa, *bb;
{
	register char *a, *b;

	a = aa;
	b = bb;
	while (*b) {
		if (*a != *b++) {
			return(0);
		}
		a++;
	}
	if (*a)
		return(0);
	else	return(1);
}

/* Print system error */
syserr(str)
char *str;
{
	extern int errno;
	extern char *sys_errlist[];

	printf("%s - %s\n", str, sys_errlist[errno]);
}
