#
/*
 * This version of tty.c started life in UCLA and made its was in into
 * various UK universities, all of whom hacked it for local use.
 * This version is from the University of Glasgow, chosen because
 * it fits in somewhat better with current usage
 *
 * I have somewhat reformatted it to make it more K&R - apologies
 * to the original authors.
 *
 * I've also removed parity checking and setting
 * Peter Collinson
 * July  2023
 */
#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../tty.h"
#include "../deftty.h"
#include "../proc.h"
#include "../inode.h"
#include "../file.h"
#include "../reg.h"
#include "../conf.h"

/* Maptab serves two functions :-
 *  maptab[c] =   parity bit for c |  lower case representation of c
 */
char maptab[] {
   0000+000, 0200+000, 0200+000, 0000+000, 0200+000, 0000+000, 0000+000, 0200+000,
   0200+000, 0000+000, 0000+000, 0200+000, 0000+000, 0200+000, 0200+000, 0000+000,
   0200+000, 0000+000, 0000+000, 0200+000, 0000+000, 0200+000, 0200+000, 0000+000,
   0000+000, 0200+000, 0200+000, 0000+000, 0200+000, 0000+000, 0000+000, 0200+000,
   0200+000, 0000+'|', 0000+000, 0200+000, 0000+000, 0200+000, 0200+000, 0000+'`',
   0000+'{', 0200+'}', 0200+000, 0000+000, 0200+000, 0000+000, 0000+000, 0200+000,
   0000+000, 0200+000, 0200+000, 0000+000, 0200+000, 0000+000, 0000+000, 0200+000,
   0200+000, 0000+000, 0000+000, 0200+000, 0000+000, 0200+000, 0200+000, 0000+000,
   0200+000, 0000+000, 0000+000, 0200+000, 0000+000, 0200+000, 0200+000, 0000+000,
   0000+000, 0200+000, 0200+000, 0000+000, 0200+000, 0000+000, 0000+000, 0200+000,
   0000+000, 0200+000, 0200+000, 0000+000, 0200+000, 0000+000, 0000+000, 0200+000,
   0200+000, 0000+000, 0000+000, 0200+000, 0000+0134, 0200+000, 0200+'~', 0000+000,
   0000+000, 0200+'A', 0200+'B', 0000+'C', 0200+'D', 0000+'E', 0000+'F', 0200+'G',
   0200+'H', 0000+'I', 0000+'J', 0200+'K', 0000+'L', 0200+'M', 0200+'N', 0000+'O',
   0200+'P', 0000+'Q', 0000+'R', 0200+'S', 0000+'T', 0200+'U', 0200+'V', 0000+'W',
   0000+'X', 0200+'Y', 0200+'Z', 0000+000, 0200+000, 0000+000, 0000+000, 0200+000
};

/* A Clist block.  All character queues are linked lists
 * of Clist blocks.  Each block contains a pointer to the
 * next block and six characters */
struct cblock {
	struct cblock *c_next;
	char  characters[6];
};

/* Header for the freelist of Cblocks */
struct cblock *cfreelist;
/* The array of Cblocks */
struct cblock cfree[NCLIST];

/* Wordtable is used by the CDWORD function to determine
 * whether a character is alphanumeric, and therefore part
 * of a word */
char wordtable[] {
	0000,  0000,  0000,  0000,  0200,  0000,  0377,  0003,
	0376,  0377,  0377,  0007,  0376,  0377,  0377,  0007
};

/* The indtable is used to  determine which characters
 * are non-printing (CBELL is printing).  Non-printing
 * characters are echoed as a pair of printing characters */
char indtable[] {
	0177,  0300,  0377,  0377,  0000,  0000,  0000,  0000,
	0000,  0000,  0000,  0000,  0000,  0000,  0000,  0200
};

/* Structure of the KL-11 registers.  Used in ttstart.  */
struct {
	int ttrcsr;
	int ttrbuf;
	int tttcsr;
	int tttbuf;
};

/* KL Hardware bits */
#define DONE 0200
#define IENABLE 0100

#define ECHOING 1
#define WRITING 0

/*
 * Ttyopen performs functions common to tty open routines,
 * it is called by klopen and dhopen  etc.
 */

ttyopen(atp)
struct tty *atp;
{
	register struct proc *pp;
	register struct tty *tp;

	tp = atp;

	/* If the process has no controlling terminal, set it */
	if (u.u_procp->p_ttyp == 0)
		u.u_procp->p_ttyp = tp;

	/* Ah declare this terminal open */
	tp->t_state =& ~WOPEN;
	tp->t_state =| ISOPEN;
}

/* Common tty read routine, called by the users read
 * system call, via klread, dhread etc. and the cdevsw table
 */
ttread(atp)
struct tty *atp;
{
	register struct tty *tp;
	register int c, cnext;

	tp = atp;

	/* Switch off any ^o */
	tp->t_state =& ~XDELETE;

	/* Wait until there is data waiting in inq */
	spl5();
	while (tp->t_delct == 0) {
		if ((tp->t_state &CARR_ON) == 0)
			return;
		sleep(&tp->t_inq, TTIPRI);
	}
	spl0();

	/* 8 bit transmission uses delct as a data count */
	if (tp->t_flags &ALL8BITS)  {
		while (tp->t_delct--
			&&  passc(getc(&tp->t_inq)) >= 0);
		return;
	}

	/* Pass data until a CDELIM, or no more data required */
	while ((c= getc(&tp->t_inq)) != CDELIM
	       &&  passc(c) >= 0);

	/* Check for a trailing CDELIM after no more data
	 * required, and clobber it */
	cnext =  *(tp->t_inq.c_cf) &0377;
	if (c != CDELIM  &&  cnext == CDELIM)
		c = getc(&tp->t_inq);

	/* Usually, delct is a count of the number of CDELIM
	 * characters in the inq, update it */
	if (c == CDELIM)
		tp->t_delct--;

	/* Reset the page line to 0 */
	tp->t_line = 0;
}

/* Common tty write routine, called by the users write
 * system call, via klwrite, dhwrite etc. and the cdevsw table.
 */
ttwrite(atp)
struct tty *atp;
{
	register struct tty *tp;
	register int  c;

	tp = atp;

	/* If the terminal is not connected, quit */
	if ((tp->t_state &CARR_ON) == 0)
		return;

	while ((c= cpass()) >= 0)  {

		/* Output queue > Hiwater, go to sleep until
		 * it empties by being printed */
		spl5();
		while (tp->t_outq.c_cc > TTHIWAT
				||  tp->t_state &(XHOLD|XSTOP))  {
			ttstart(tp);
			tp->t_state =| ASLEEP;
			sleep(&tp->t_outq, TTOPRI);
		}
		spl0();


		/* Put data on the outq */
		if ((tp->t_state &XDELETE) == 0)
			ttyoutput(c, tp, WRITING);
	}

	ttstart(tp);
}

/*
 * Ttyinput is called by receiver interrupt routines
 * to process data input, and put it on the inq for
 * ttread to collect later.
 */
ttyinput(ac, atp)
int  ac;
struct tty *atp;
{
	register int c, t_flags;
	register struct tty *tp;
	int delimit;
	char *p;

	tp = atp;
	t_flags = tp->t_flags;

	/* Data from the interrupt routine may have garbage
	 * in its hibyte */
	c =  ac &0377;

	/* 8 bits mode is entirely different from character data */
	if (t_flags &ALL8BITS)  {

		/* Hold mode gives ^s ^q. State Xhold causes
		 * output to be temporarily suspended */
		if (t_flags &HOLD)  {
			if (c == CXON)  {
				tp->t_state =& ~XHOLD;
				goto start;
			}
			if (c == CXOFF)  {
				tp->t_state =| XHOLD;
				return;
			}
		}

		/* Put the data on the inq, count it, and wake up
		 * any ttread routine that is waiting for data */
		putc(c, &tp->t_inq);
		tp->t_delct++;
		wakeup(&tp->t_inq);
		return;
	}

	/* Crmod, <return> becomes <newline> */
	if (c == '\r'  &&  t_flags &CRMOD)
		c = '\n';

	/* Fools paradise */
	if (t_flags &(CAPSONLY|DATA100))  {
		if ('A' <= c  &&  c <= 'Z')
			c =+ 'a' - 'A';
		else if (t_flags &DATA100  &&
				'a' <= c  &&  c <= 'z')
					c =+ 'A' - 'a';
	}

	/* Are we sitting held up at the end of a page of output */
	if ( (tp->t_state &(XHOLD|XSTOP)) == (XHOLD|XSTOP)
	   && (c>=' ' && c<'\177'  ||  c==CNEXTLINE) )	{

		/* If the character is CNEXTLINE, allow just
		 * one line of output before we stop again,
		 * otherwise set line in page to 0 */
		tp->t_line =  (c==CNEXTLINE? tp->t_length-1: 0);
		tp->t_state =& ~(XHOLD|XSTOP);
		goto start;
	}

	/* Check for a delimit character */
	delimit = 0;
	if (t_flags &USERBREAK)  {
		/* c is a delimit if bit c of breaktab is set */
		if (bitmap(tp->t_breaktab, c))  {
			delimit++;
			goto stashchar;
		}
	}
	/* Otherwise, in normal mode only <newline> delimits.
	 * In RAW mode, they all do */
	else  if (c == '\n'  ||  t_flags &RAW)  {
		delimit++;
		goto stashchar;
	}

	/* As all characters delimit in RAW mode,
	 * none of them are special */
	if (t_flags &RAW)
		goto stashchar;

	/* We are in Literal state if the previous character
	 * was a Cliteral.  Literal state nobbles any special
	 * meaning that c usually has */
	if (tp->t_state &LITERAL)  {
		tp->t_state =& ~LITERAL;
		/* Fools paradise */
		if (t_flags &CAPSONLY  &&  maptab[c] &0177)
			c =  maptab[c] &0177;
		goto stashchar;
	}

	if (c == tp->t_erase)  {
		eraseprint(zapc(&tp->t_inq), tp);
		goto start;
	}

	if (c == tp->t_kill)  {
		echo(c, tp);
		echo('\n', tp);
		/* Empty inq up to the last CDELIM */
		while (zapc(&tp->t_inq) >= 0);
		goto start;
	}

	if (c == CLITERAL)  {
		tp->t_state =| LITERAL;
		/* Cliteral is echoed as '\' */
		echo('\\', tp);
		goto start;
	}

	switch (c)  {
	case CINTR:
		/* Send an interrupt signal */
		t_flags = SIGINT;
		goto setsig;
	case CQUIT:
		/* Send a quit signal */
		t_flags = SIGQIT;
	setsig: flushtty(tp);
		echo(c, tp);
		echo('\n', tp);
		signal(tp, t_flags);
		goto start;
	case CDWORD:
		/* Delete the last word input, or the last character.
		 * We delete until the first non-alpanumeric
		 * character */
		eraseprint((c= zapc(&tp->t_inq)), tp);
		if (bitmap(wordtable, c))  {
			while (bitmap(wordtable,
					(c= zapc(&tp->t_inq))))
				eraseprint(c, tp);
			/* If c is a character, put it back */
			if (c >= 0)
				if (putc(c, &tp->t_inq))
					goto echobell;
		}
		goto start;
	case CXOFF:
		/* Temporarily suspend output */
		tp->t_state =| XHOLD;
		return;
	case CXON:
		/* Restore output */
		tp->t_state =& ~(XHOLD|XSTOP);
		goto start;
	case CEOF:
		/* End of file, put a CDELIM on the inq.
		 * Immediately after another delimit character
		 * this will cause ttread to return zero bytes read,
		 * system convention for end of file */
		echo(c, tp);
		echo('\n', tp);
		delimit++;
		goto delimchar;
	case CRETYPE:
		/* Re-echo the current line */
		echo(c, tp);
		echo('\n', tp);

		/* Start at the front of inq, trog
		 * down it counting CDELIMs until none left,
		 * print remaining characters in queue */
		delimit =  tp->t_delct;
		c =  tp->t_inq.c_cc;
		p =  tp->t_inq.c_cf;
		while (c--)  {
			if (delimit)   {
				if (((*p++) &0377) == CDELIM)
					delimit--;
			}
			else	echo(*p++, tp);
			/* Contents of p ends in 0 => block end,
			 * so set p to next block in the queue */
			if ((p &07) == 0)
				p =  ((p-8)->integ) + 2;
		}
		goto start;
	case CXDELETE:
		/* Delete output */
		if ((tp->t_state &XDELETE) == 0)  {
			/* flush the outq */
			while (getc(&tp->t_outq) >= 0);
			tp->t_state =& ~(XHOLD|XSTOP);
		}
		/* Flip the Delete state */
		tp->t_state  =^ XDELETE;
		echo(c, tp);
		goto start;
	}

stashchar:
	/* Ttyhogs do not get their characters on the inq, this
	 * helps to prevent a shortage of Clist space */
	if ((tp->t_inq.c_cc >= TTYHOG  &&  delimit == 0)
			||  putc(c, &tp->t_inq))
		goto echobell;
	echo(c, tp);
delimchar:
	/* If delimit is set, put a CDELIM on inq, and count it.
	 * Wake up any ttread that is waiting for data */
	if (delimit)  {
		if (putc(CDELIM, &tp->t_inq))
			goto echobell;
		tp->t_delct++;
		wakeup(&tp->t_inq);
	}
start:
	ttstart(tp);
	return;
echobell:
	/* The echobell label is used as an error label.
	 * Usually when putting a character on a queue fails
	 * due to lack of Clist space.
	 * Unfortunately it is likely that the warning <bell>
	 * will also fail due to a lack of Clist space */
	echo(CBELL, tp);
	ttstart(tp);
}

/*
 * Echo is called by ttyinput to echo characters on
 * the users terminal
 */
echo(ac, atp)
int  ac;
struct tty *atp;
{
	register struct tty *tp;
	register int  c;

	c = ac;
	tp = atp;
	/* No echo then quit */
	if ((tp->t_flags &ECHO) == 0)
		return;

	/* If we were echoing erased characters, print
	 * a closing bracket */
	if (tp->t_state &ERASING)  {
		tp->t_state =& ~ERASING;
		ttyoutput(CERASECLOSE, tp, ECHOING);
	}
	ttyoutput(c, tp, ECHOING);
}

/*
 * Eraseprint is called by ttyinput to print erased
 * ucharacters
 */
eraseprint(ac, atp)
int  ac;
struct tty *atp;
{
	register struct tty *tp;
	register int c, i;

	tp = atp;
	c =  ac;

	/* No echo then quit */
	if ((tp->t_flags &ECHO) == 0)
		return;

	/* c < 0 is returned by zapc when queue is empty.
	 * Echo a <bell> to indicate there is nothing to erase */
	if (c < 0)  {
		ttyoutput(CBELL, tp, ECHOING);
		return;
	}
	/* If terminal is in scope mode, then wipe the
	 * characters off the screen */
	if (tp->t_flags &SCOPE)  {
		i = 1;
		/* If it was a control character, then
		 * wipe out two characters */
		if (bitmap(indtable, c))
			i++;
		/* If it was a <tab>, wipe out however many */
		if (c == '\t')
			i =  tp->t_col &07;
		while (i--)  {
			ttyoutput('\b', tp, ECHOING);
			ttyoutput(' ', tp, ECHOING);
			ttyoutput('\b', tp, ECHOING);
			}
		return;
	}

	/* If we were not already printing erased characters,
	 * print an open erase bracket */
	if ((tp->t_state &ERASING) == 0)  {
		tp->t_state =| ERASING;
		ttyoutput(CERASEOPEN, tp, ECHOING);
	}
	ttyoutput(c, tp, ECHOING);
}

/*
 * Ttyoutput is used to put a character on the outq for
 * printing, by ttwrite and echo, eraseprint etc.
 */
ttyoutput(ac, atp, echoing)
int  ac;
struct tty *atp;
int  echoing;
{
	register int  c;
	register char  *colp;
	register struct tty *tp;
	int   sps, delay;

	tp = atp;
	c = ac;

	/* In 8 bit mode, don't mess about */
	if (tp->t_flags &ALL8BITS)  {
		putc(c, &tp->t_outq);
		return;
	}

	/* And off the parity bit */
	c =& 0177;

	/* In Crmod, print <newline> as <return><newline> */
	if (c == '\n'  &&  tp->t_flags &CRMOD)
		ttyoutput('\r', tp, echoing);

	/* In page mode, count the <newline>s output.  Before
	 * the last one put an 0200 on the queue and set Xstop.
	 * Tttstart (or dhstart etc) will catch the 0200 and set
	 * Xhold state to prevent further printing beyond the page
	 * end */
	if (echoing == WRITING  &&  tp->t_length  &&
		((c == '\n'  &&  ++(tp->t_line) >= tp->t_length)
		||  c == CFORM))  {
			tp->t_state =| XSTOP;
			tp->t_line = 0;
			putc(0200, &tp->t_outq);
	}

	/* Fools paradise */
	if (tp->t_flags &CAPSONLY)  {
		if ('a' <= c  &&  c <= 'z')
			c =+ 'A' - 'a';
		else  if ('A' <= c  &&  c <= 'Z'  &&  echoing == WRITING)
			ttyoutput('\\', tp, echoing);
		else {
			colp =  "({)}!|^~'`";
			while (*colp++)
				if (c == *colp++)   {
					ttyoutput('\\', tp, echoing);
					c =  colp[-2];
					break;
				}
		}
	}

	/* Print control characters as  ^<character +0140>  */
	if ((tp->t_flags&INDCTL || echoing)  &&  bitmap(indtable, c))  {
		ttyoutput('^', tp, echoing);
		if (c == 0177)
			c = '?';
		else  if ('A' <= (c=| 0100)  &&  c <= 'Z'
				&&  (tp->t_flags &CAPSONLY) == 0)
			c =| 0140;
	}

	/* Do not send ^d in Raw mode, to avoid hanging up certain
	 * (I never met one) terminals */
	if (tp->t_flags &RAW  &&  c == '\4')
		return;

	sps = PS->integ;
	spl5();
	colp =  &tp->t_col;

	/* Software tabs */
	if (c == '\t'  &&  tp->t_flags &XHTAB)  {
		c = tp->t_htab - lrem(*colp, tp->t_htab);
		do ttyoutput(' ', tp, echoing);
		while (--c > 0);
		goto out;
	}

	/* Software vertical tab or formfeed */
	if ((c==CFORM || c==CVTAB)  &&  tp->t_flags &XVTAB)  {
		for (c = 0;  c < tp->t_vtab;  c++)
			ttyoutput('\n', tp, echoing);
		goto out;
	}

	/* If it was printable, count it */
	if (' ' <= c  &&  c < '\177')  {
		(*colp)++;
		/* To stop the terminal running off the end of its
		 * line, after t_width characters put a <newline> */
		if (tp->t_width  &&  (*colp&0377) > (tp->t_width&0377))
			ttyoutput('\n', tp, echoing);
	}

	putc(c, &tp->t_outq);

	delay = 0;
	switch (c)  {
	case '\n':
		*colp = 0;
		delay = tp->t_nldelay;
		break;
	case '\r':
		*colp = 0;
		delay = tp->t_crdelay;
		break;
	case '\b':
		(*colp)--;
		goto out;
	case '\t':
		*colp =| 07;
		(*colp)++;
		delay = tp->t_htab;
		break;
	case CFORM:
	case CVTAB:
		delay = tp->t_vtab;
		break;
	}

	/* A delay is put on the outq as delay +0200 */
	if (delay)
		putc(delay|0200, &tp->t_outq);
out:
	PS->integ = sps;
}


/*
 * Ttrstrt is only called at the end of a timeout (delay) to
 * restart output
 */
ttrstrt(atp)
struct tty *atp;
{
	register struct tty *tp;

	tp = atp;
	tp->t_state =& ~TIMEOUT;
	ttstart(tp);
}

/*
 * Ttstart, start printing the contents of outq.
 * Note that after putting stuff on the outq, ttstart must be
 * called to start the printing, and as it is not called by
 * ttyoutput, it is the responsibility of the routine calling
 * ttyoutput to call ttstart. It is not called by ttyoutput
 * in order to keep down the number of calls.
 * Once the queue has started to print, ttstart will be called
 * by the transmitter interrupt handlers until the outq is empty
 */
ttstart(atp)
struct tty *atp;
{
	register struct tty *tp;
	register int c, *addr;
	int sps;
	struct {
		int (*func)();
	};

	tp = atp;
	addr = tp->t_addr;

	/* If the terminal has a special start routine
	 * e.g. dhstart, do it */
	if (tp->t_state &SSTART)  {
		(*addr.func)(tp);
		return;
	}
	sps =  PS->integ;
	spl5();

	/* If its busy or Xhold is set, quit */
	if ((addr->tttcsr &DONE) == 0  ||  tp->t_state &(TIMEOUT|XHOLD))
			goto out;

	/* If outq is empty wake up ttwrite who may be asleep
	 * after outq grew beyond Highwater */
	if ((c= getc(&tp->t_outq)) < 0)  {
		if (tp->t_state &ASLEEP)  {
			tp->t_state =& ~ASLEEP;
			wakeup(&tp->t_outq);
		}
		goto out;
	}

	/* c > 0200 is a delay, and c == 0200 marks a page end,
	 * so set Xhold */
	if (c &0200  &&  (tp->t_flags &ALL8BITS) == 0)  {
		if (c =& 0177)  {
			/* do a delay */
			tp->t_state =| TIMEOUT;
			timeout(ttrstrt, tp, c);
		}
		else	tp->t_state =| XHOLD;
		goto out;
	}

	/* Parity checking, who needs it ?
	 * pc Removed
#	ifdef	KLPARITY
	if ((tp->t_flags &ALL8BITS) == 0)  {
		if ((tp->t_flags &(ODDP|EVENP)) == ODDP)
			c =| (maptab[c] &0200);
		else  if ((tp->t_flags &(ODDP|EVENP)) == EVENP)
			c =| ~(maptab[c] &0200);
		else  if ((tp->t_flags &(ODDP|EVENP)) == ODDP|EVENP)
			c =| 0200;
	}
#	endif */
	addr->tttbuf =  c;

out:
	PS->integ = sps;
}

/*
 * Wflush, flush a terminal after it has become idle
 */
wflushtty(atp)
struct tty *atp;
{
	register struct tty *tp;

	tp = atp;
	spl5();
	while (tp->t_state &CARR_ON  &&  tp->t_outq.c_cc)  {
		/* ask to be woken up when outq is empty */
		tp->t_state =| ASLEEP;
		sleep(&tp->t_outq, TTOPRI);
		}
	flushtty(tp);
	spl0();
}

/*
 * Flushtty, flush all the queues and reset the state
 */
flushtty(atp)
struct tty *atp;
{
	register struct tty *tp;

	tp = atp;
	while (getc(&tp->t_outq) >= 0);
	spl5();
	while (getc(&tp->t_inq) >= 0);
	tp->t_delct = 0;
	tp->t_state =& ~(XHOLD|XSTOP|XDELETE|LITERAL|ERASING);
	spl0();
	wakeup(&tp->t_inq);
	wakeup(&tp->t_outq);
}

/*
 * Cinit is called only once, in the initialization.
 * It makes up a freelist of Clist blocks.  The address
 * of the first block and hence all others) is rounded
 * up to end in 0 (octal), wasting one cblock.
 * This property of Clist blocks is used in getc, putc
 * zapc, and CRETYPE
 */
cinit()
{
	register int  ccp;
	register struct cblock *cp;
	register struct cdevsw *cdp;

	/* Make up the freelist */
	ccp = cfree;
	for (cp = (ccp+07)&~07;  cp< &cfree[NCLIST-1];  cp++)  {
		cp->c_next = cfreelist;
		cfreelist = cp;
		}

	/* Count the number of character devices */
	ccp = 0;
	for (cdp = cdevsw;  cdp->d_open;  cdp++)
		ccp++;
	nchrdev = ccp;
}

/*
 * Gtty system call arranges a terms call type 0
 */

gtty()
{
	/* Move pointer to data area into the second argument,
	 * as expected by terms */
	u.u_arg[1] = u.u_arg[0];
	u.u_arg[0] = 0;
	terms();
}

/*
 * Stty system call arranges a terms call type 1
 */
stty()
{
	/* Move pointer to the data area into the second argument */
	u.u_arg[1] = u.u_arg[0];
	u.u_arg[0] =  1;
	terms();
}

/*
 * Terms system call, set and report modes
 */
terms()
{
	register struct file *fp;
	register struct inode *ip;

	/* Is the file descriptor valid */
	if ((fp= getf(u.u_ar0[R0])) == NULL)
		return;
	/* Is it a character device */
	ip = fp->f_inode;
	if ((ip->i_mode &IFMT) != IFCHR)  {
		u.u_error = ENOTTY;
		return;
	}
	(*cdevsw[ ip->i_addr[0].d_major ].d_sgtty)(ip->i_addr[0]);
}

/*
 * Ttystty, set and report information in the tty structure
 * of a terminal.
 * u.u_arg[0].lobyte = 0  report t_speeds to t_flags (5 bytes)
 *                     1 set t_speeds to t_flags
 *                     2 report t_speeds to t_breaktab (28 bytes)
 *                     3 set t_speeds to t_breaktab
 * 0 and 1 are almost compatible with the Bell tty driver
 * gtty and stty, except they used to also deal with
 * the delays.  This is no longer possible due to the
 * different implementation of delays
 */
ttystty(atp)
struct tty *atp;
{
	register struct tty *tp;
	register int  i;
	register char  *tpdata;
	char   *usr;

	tp = atp;
	wflushtty(tp);
	usr = u.u_arg[1];
	tpdata =  &tp->t_speeds;
	for ( i= 0;  i < 28;  i++)  {
		if (i == 5  &&  (u.u_arg[0] &02) == 0)
				return;
		if (u.u_arg[0] &01)	/* set */
			*tpdata++ =  fubyte(usr++);
		else			/* report */
			subyte(usr++, *tpdata++);
	}
	if (u.u_arg[0] &01)  {
		if (tp->t_flags &XHTAB  &&  (tp->t_htab=& 0177) == 0)
			tp->t_htab = 8;
		tp->t_length =& 0177;
	}
}

/*
 * bitmap returns non-zero is bit c of map_addr is set
 */
bitmap(map_addr, bit_num)
char *map_addr;
int bit_num;
{
	register int bit, word;

	bit =  bit_num;
	if (bit < 0)
		return(0);
	word =  bit >> 3;
	return(map_addr[word] &(1<< (bit &07)));
}
