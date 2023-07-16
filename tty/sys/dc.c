#
/*
 *   DC-11 driver
 *   Adapted from original UNIX version by
 *   Peter Collinson - Nov 76
 *   Copes with auto-dialing and carrier fail waits
 */
#include "../param.h"
#include "../conf.h"
#include "../user.h"
#include "../tty.h"
#include "../deftty.h"
#include "../proc.h"

/*
 * Base address of DC-11's. Minor device  i  is at
 * DCADDR + 10*i.
 */
#define	DCADDR	0174000
#define IENABLE	0100

/*
 * Number of DC's for which table space is allocated.
 */
#define	NDC11	14

/*
 * Control bits in device registers
 */
/*
 * Default speed for stty function
 */
#define DEF_SPEED	3
#define MAX_SPEED	9		/* Max speed in table */
#define	CDLEAD	01
#define	CARRIER	04
#define SPEED1	010
#define	STOP1	0400
#define STOP2	0
#define	RQSEND	01
#define	PARITY	040
#define	ERROR	0100000
#define	CTRANS	040000
#define	RINGIND	020000


/*
 * Time out value: should be equal to number of seconds required * 4
 */
#define WSEC	40


struct	tty dc11[NDC11];

struct dcregs {
	int dcrcsr;
	int dcrbuf;
	int dctcsr;
	int dctbuf;
};

/*
 * Input-side speed and control bit table.
 * Each DC11 has 4 speeds which correspond to the 4 non-zero entries.
 * The table index is the same as the speed-selector
 * number for the DH11.
 * Attempts to set the speed to a zero entry are ignored.
 * Table is for a DC11-AC
 */
int dcrstab[] {
	0,		/* 0 baud */
	0,		/* 50 baud */
	0,		/* 75 baud */
	0101,		/* 110 baud Speed 0 - 7b/ch */
	0,		/* 134.5 baud */
	0111,		/* 150 baud: 7b/ch, speed 1 */
	0,		/* 200 baud */
	0121,		/* 300 baud */
	0,		/* 600 baud */
	0131		/* 1200 baud */
};

/*
 * Transmitter speed table
 */
int dctstab[] {
	0,		/* 0 baud */
	0,		/* 50 baud */
	0,		/* 75 baud */
	0101,		/* 110 baud stop 1 */
	0,		/* 134.5 baud */
	0511,		/* 150 baud */
	0,		/* 200 baud */
	0521,		/* 300 baud */
	0,		/* 600 baud */
	0531		/* 1200 baud */
};

/*
 * Carrier wait routine - wait WSEC/4 seconds before killing any processes
 */

/* Give dcarrwait a kick after 0.25 sec to see whether carrier established */
dckick(tp)
struct tty *tp;
{	wakeup(&tp->t_erase);	}

dcarrwait(tty)
struct tty *tty;
{	register struct tty *tp;
	register int i;

	tp = tty;
	for(i = 0; i <= WSEC; i++)
	{	timeout(dckick,tp,12);		/* Timeout for 0.25 secs */
		sleep(&tp->t_erase,TTIPRI);	/* And sleep until kick */
		if(tp->t_state&CARR_ON) return(1);
	}
	return(0);
}
/*
 * Open a DC11, waiting for phone to ring.
 * When phone rings establish carrier.
 * Default initial conditions are set up on the first open.
 * t_state's CARR_ON bit is a pure copy of the hardware
 * CARRIER bit, and is only used to regularize
 * carrier tests in general tty routines.
 * If CARRIER fails - CARR_ON bit is used to kill any processes
 * after a suitable timeout.
 */
dcopen(dev, flag)
{
	register struct tty *rtp;
	register *addr;

	if (dev.d_minor >= NDC11) {
		u.u_error = ENXIO;
		return;
	}
	rtp = &dc11[dev.d_minor];
	rtp->t_addr = addr = DCADDR + dev.d_minor*8;
	rtp->t_state =| WOPEN;

	if(addr->dcrcsr&CARRIER) rtp->t_state =| CARR_ON;
	else
	while((rtp->t_state&CARR_ON) == 0)
	{	/* No carrier wait for ring */

		addr->dcrcsr = IENABLE|SPEED1;
		addr->dctcsr = IENABLE|SPEED1|STOP2;
		sleep(&rtp->t_state,TTIPRI);	/* Wait for ring */
		dcarrwait(rtp);
	}

	/* Carrier present set up tty structure if necessary */
	if((rtp->t_state&ISOPEN) == 0)
	{	rtp->t_erase = CERASE;
		rtp->t_kill = CKILL;
		rtp->t_flags = EVENP|ODDP|ECHO|CRMOD|INDCTL;
		rtp->t_state =| ISOPEN;
	}
	rtp->t_dev = dev;
	ttyopen(rtp);
}

/*
 * Close a dc11
 */
dcclose(dev)
{
	register struct tty *tp;
	register int hup;
	(tp = &dc11[dev.d_minor])->t_state = 0;
	hup = tp->t_flags&HUPCL;
	wflushtty(tp);
	if(hup)
	{	tp->t_addr->dcrcsr =& ~CDLEAD;
		tp->t_addr->dctcsr =& ~RQSEND;
	}
}

/*
 * See if carrier is off and if so wait for 10 secs before killing processes
 */
dcseeoff(rtp)
struct tty *rtp;
{	register struct tty *tp;
	tp = rtp;
	if((tp->t_state&CARR_ON) == 0 )
	{	if(dcarrwait(tp) == 0)
		{	/* Time up close device */
			tp->t_addr->dcrcsr =& ~CDLEAD;
			tp->t_addr->dctcsr =& ~RQSEND;
			if(tp->t_state&ISOPEN)
				signal(tp, SIGHUP);
			flushtty(tp);
		}
	}
}

/*
 * Read a DC11
 */
dcread(dev)
{	register struct tty *tp;
	dcseeoff(tp = &dc11[dev.d_minor]);
	ttread(tp);
}

/*
 * Write a DC11
 */
dcwrite(dev)
{	register struct tty *tp;
	dcseeoff(tp = &dc11[dev.d_minor]);
	ttwrite(tp);
}

/*
 * DC11 transmitter interrupt.
 */
dcxint(dev)
{
	register struct tty *tp;

	ttstart(tp = &dc11[dev.d_minor]);
	if (tp->t_outq.c_cc == 0 || tp->t_outq.c_cc == TTLOWAT)
		wakeup(&tp->t_outq);
}

/*
 * DC11 receiver interrupt.
 */
dcrint(dev)
{
	register struct tty *tp;
	register int c, csr;

	tp = &dc11[dev.d_minor];
	c = tp->t_addr->dcrbuf;
	/*
	 * Check for error bit set
	 * if so could be a RING trying to open
	 * or a carrier transition
	 * all other error conditions ignored
	 */

	if((csr = tp->t_addr->dcrcsr)&ERROR)
	{	if(csr&RINGIND)		/* Ring indicator */
		{	if((tp->t_state&WOPEN) == 0) return;	/* Not opening exit */
			tp->t_addr->dcrcsr =| CDLEAD;
			tp->t_addr->dctcsr =| RQSEND;
			wakeup(&tp->t_state);			/* Wake up open routine */
			return;
		}

		if(csr&CTRANS)
		{	/* Check carrier state */
			if(csr&CARRIER)
			{	tp->t_state =| CARR_ON;
				return;
			}
			else
			{	tp->t_state =& ~CARR_ON;
				/* Force top level to kill if not opening */
				if((tp->t_state&WOPEN) == 0)
					c = '\n';
			}
		}
		else return;
	}
	if((tp ->t_flags&ALL8BITS) == 0)
		c =& 0177;
	ttyinput(c, tp);
}

/*
 * DC11 stty/gtty.
 * Perform general functions and set speeds.
 */
dcsgtty(dev, av)
int *av;
{
	register struct tty *tp;
	register r, val;

	tp = &dc11[dev.d_minor];
	if (ttystty(tp, av))
		return;
	if (((val = tp->t_speeds.lobyte&017) <= MAX_SPEED) && (r = dcrstab[val]))
		tp->t_addr->dcrcsr = r;
	else
	{	tp->t_addr->dcrcsr = dcrstab[DEF_SPEED];
		tp->t_speeds.lobyte = DEF_SPEED;
	}
	if (((val = tp->t_speeds.hibyte&017) <= MAX_SPEED) && (r = dctstab[val]))
		tp->t_addr->dctcsr = r;
	else
	{	tp->t_addr->dctcsr = dctstab[DEF_SPEED];
		tp->t_speeds.hibyte = DEF_SPEED;
	}
}
