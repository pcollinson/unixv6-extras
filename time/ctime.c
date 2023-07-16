#
/*
 * This routine converts time as follows.
 * The epoch is 0000 Jan 1 1970 GMT.
 * The argument time is in seconds since then.
 * The localtime(t) entry returns a pointer to an array
 * containing
 *  seconds (0-59)
 *  minutes (0-59)
 *  hours (0-23)
 *  day of month (1-31)
 *  month (0-11)
 *  year-1970
 *  weekday (0-6, Sun is 0)
 *  day of the year
 *  daylight savings flag
 *
 * The routine corrects for daylight saving
 * time and will work in any time zone provided
 * "timezone" is adjusted to the difference between
 * Greenwich and local standard time (measured in seconds).
 * In places like Michigan "daylight" must
 * be initialized to 0 to prevent the conversion
 * to daylight time.
 * There is a table which accounts for the peculiarities
 * undergone by daylight time in 1974-1975.
 *
 * The routine does not work
 * in Saudi Arabia which runs on Solar time.
 *
 * asctime(tvec))
 * where tvec is produced by localtime
 * returns a ptr to a character string
 * that has the ascii time in the form
 *	Thu Jan 01 00:00:00 1970n0\\
 *	01234567890123456789012345
 *	0	  1	    2
 *
 * ctime(t) just calls localtime, then asctime.
 *
 * Reworked for UnixV6
 * Peter Collinson
 * July 2023
 *
 * Started off by trying to keep the V6 algorithms
 * but that proved difficult because the double
 * div routines in V6 - just didn't cope with the
 * large numbers 32 bit numbers we are getting in dates.
 * Decided to steal the v7 aldiv and alrem routines,
 * including  fixes from Henry Spencer. These are
 * in ctsup.s - and are renamed.
 *
 * The new code introduces some new routines that I
 * have named with an _ to maintain the same code
 * footprint as before.
 */
char	cbuf[26];
int	dmsize[12]
{
	31,
	28,
	31,
	30,
	31,
	30,
	31,
	31,
	30,
	31,
	30,
	31
};

/* USA
 * This value is subtracted
 * if you are east of Greenwich
 * put a -ve number here
int timezone	5*60*60;
int tzname[]
{
	"EST",
	"EDT",
};
*/
/* UK */
int timezone	0;
int tzname[]
{
	"GMT",
	"BST",
};
int	daylight 1;	/* Allow daylight conversion */
/*
 * The following table is used for 1974 and 1975 and
 * gives the day number of the first day after the Sunday of the
 * change.
 */
struct {
	int	daylb;
	int	dayle;
} daytab[] {
	5,	333,	/* 1974: Jan 6 - last Sun. in Nov */
	58,	303,	/* 1975: Last Sun. in Feb - last Sun in Oct */
};

#define	SEC	0
#define	MIN	1
#define	HOUR	2
#define	MDAY	3
#define	MON	4
#define	YEAR	5
#define	WDAY	6
#define	YDAY	7
#define	ISDAY	8

ctime(at)
int *at;
{
	return(asctime(localtime(at)));
}

localtime(tim)
int tim[];
{
	register int *t, *ct, dayno;
	int daylbegin, daylend;
	int copyt[2];

	t = copyt;
	t[0] = tim[0];
	t[1] = tim[1];
	if (timezone != 0)
		dpadd(t, -timezone);
	ct = gmtime(t);
	dayno = ct[YDAY];

	/* USA */
	/* daylbegin = 119;	/* last Sun in Apr */
	/* daylend = 303;	/* Last Sun in Oct */
	/* if (ct[YEAR]==74 || ct[YEAR]==75) {
		daylbegin = daytab[ct[YEAR]-74].daylb;
		daylend = daytab[ct[YEAR]-74].dayle;
	} */
	/* UK */
	daylbegin = 89;		/* last Sun in Mar 31 + 28 + 31 -1` */
	daylend = 303;		/* Last Sun in Oct */

	daylbegin = sunday(ct, daylbegin);
	daylend = sunday(ct, daylend);
	if (daylight &&
	    (dayno>daylbegin || (dayno==daylbegin && ct[HOUR]>=2)) &&
	    (dayno<daylend || (dayno==daylend && ct[HOUR]<1))) {
		dpadd(t, 1*60*60);
		ct = gmtime(t);
		ct[ISDAY]++;
	}
	return(ct);
}

/*
 * The argument is a 0-origin day number.
 * The value is the day number of the first
 * Sunday on or after the day.
 */
sunday(at, ad)
int *at;
{
	register int *t, d;

	t = at;
	d = ad;
	if (d >= 58)
		d =+ dysize(t[YEAR]) - 365;
	return(d - (d - t[YDAY] + t[WDAY] + 700) % 7);
}

/*
 * gmtime reworked
 * NB V6 didn't support times before 1970, so
 * I have not implemented that feature
 */
#define lcp(a, b) a[0] = b[0]; a[1] = b[1]

gmtime(tim)
int tim[];
{
	register int d0, d1;
	register *tp;
	static int xtime[9];
	/* buffers for doing double arithmetic */
	int days[2];
	int hms[2];
	/* used for working out day of the week */
	int y, m, d;

	lcp(days, tim);		/* create a copy */
	lcp(hms, tim);		/* create a copy */

	/* Convert seconds into days and compute the remainder
 	 * divide by 86400 is (1, 20864)
	 */
	v7aldiv(days, 1, 20864);
	v7alrem(hms, 1, 20864);

	tp = &xtime[0];
	/*
	 * generate hours:minutes:seconds
	 * from remainder in hms
	 */
	*tp++ = _dblrem(hms, 60);	/* seconds */
	v7aldiv(hms, 0, 60);
	d1 = hms[1];			/* can start using ints */
	*tp++ = d1%60;			/* mins */
	d1 = d1/60;
	*tp++ = d1;			/* hours */

	/*
	 * year number - this counts the
	 * number of days from epoch to now
	 * max value here is 24855 - so
	 * can set d0 to the lo value in days
	 */
	d0 = days[1];
	for(d1=70; d0 >= dysize(d1); d1++)
		d0 =- dysize(d1);

	xtime[YEAR] = d1;
	xtime[YDAY] = d0+1;	/* Jan 1 is day 1 */


	/*
	 * generate month - change the dmsize
	 * table if needed, and put it back
	 * afterwards
	 */
	if (dysize(d1)==366)
		dmsize[1] = 29;
	for(d1=0; d0 >= dmsize[d1]; d1++)
		d0 =- dmsize[d1];
	dmsize[1] = 28;
	*tp++ = d0+1;		/* Day of the month 1->31 */
	*tp++ = d1;		/* Month 0->11 */
	xtime[ISDAY] = 0;
	/* nice fast dow algorithm */
	xtime[WDAY] = _dayofweek(xtime[YEAR] + 1900, d1+1, d0+1);
	return(xtime);
}

/* remainder of a double value - with an int result */
_dblrem(dbl, quot)
int dbl[];
int quot;
{
	int tmp[2];

	lcp(tmp, dbl);
	v7alrem(tmp, 0, quot);
	return(tmp[1]);
}

asctime(t)
int *t;
{
	register char *cp, *ncp;
	register int *tp;
	int yr;

	cp = cbuf;
	for (ncp = "Day Mon 00 00:00:00 1900\n"; *cp++ = *ncp++;);
	ncp = &"SunMonTueWedThuFriSat"[3*t[6]];
	cp = cbuf;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	cp++;
	tp = &t[4];
	ncp = &"JanFebMarAprMayJunJulAugSepOctNovDec"[(*tp)*3];
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	cp = ct_numb(cp, *--tp);
	cp = ct_numb(cp, *--tp+100);
	cp = ct_numb(cp, *--tp+100);
	cp = ct_numb(cp, *--tp+100);
	yr = t[YEAR];
	if (yr >= 100) {
		cp[1] = '2'; /* skip the space */
		cp[2] = '0';
		yr =- 100;
	}
	cp =+ 2;
	cp = ct_numb(cp, yr);
	return(cbuf);
}

/*
 * leap years - reworked
 * Originally it was when y is divisible by 4
 * now but NOT when the year is divisible by 100 but is divisible by 400
 * this needs fixing in v7 too - 2000 was a leap year
 *
 * can be called with a real year 1970
 * or a number which is year-1900
 */
dysize(yr)
{
	register y;

	y = yr;
	/* called internally */
	if (y < 500) y =+ 1900;

	if (((y%4) == 0 && (y%100) != 0) || (y%400) == 0)
		return(366);
	return(365);
}

/* from https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week
 * 1 <= m <= 12,  y > 1752 (in the U.K.) */
int _dowtab[] {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};

_dayofweek(yr, mo, d)
int yr, mo, d;
{
	/*  The compiler doesn't like this
	 *  static int t[]  {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
	 */
	register y, m;

	y = yr;
	m = mo;
	if (m < 3)
		y =- 1;
	return (y + y/4 - y/100 + y/400 + _dowtab[m-1] + d) % 7;
}

ct_numb(acp, n)
{
	register char *cp;

	cp = acp;
	cp++;
	if (n>=10)
		*cp++ = (n/10)%10 + '0';
	else
		*cp++ = ' ';
	*cp++ = n%10 + '0';
	return(cp);
}
