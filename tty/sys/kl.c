#
/*
 */

/*
 *   KL/DL-11 driver
 */
#include "../param.h"
#include "../conf.h"
#include "../user.h"
#include "../tty.h"
#include "../deftty.h"

/* base address */
#define	KLADDR	0177560	/* console */
#define	KLBASE	0176500	/* kl and dl11-a */
#define	DLBASE	0175610	/* dl-e */
#define	NKL11	1
#define	NDL11	0
#define DSRDY	02
#define	IENABLE	0100
#define	RDRENB	01

struct	tty kl11[NKL11+NDL11];

struct klregs {
	int klrcsr;
	int klrbuf;
	int kltcsr;
	int kltbuf;
}

klopen(dev, flag)
{
	register char *addr;
	register struct tty *tp;

	if(dev.d_minor >= NKL11+NDL11) {
		u.u_error = ENXIO;
		return;
	}
	tp = &kl11[dev.d_minor];
	/*
	 * set up minor 0 to address KLADDR
	 * set up minor 1 thru NKL11-1 to address from KLBASE
	 * set up minor NKL11 on to address from DLBASE
	 */
	addr = KLADDR + 8*dev.d_minor;
	if(dev.d_minor)
		addr =+ KLBASE-KLADDR-8;
	if(dev.d_minor >= NKL11)
		addr =+ DLBASE-KLBASE-8*NKL11+8;
	tp->t_addr = addr;
	tp->t_dev = dev;
	if ((tp->t_state&ISOPEN) == 0) {
		tp->t_state = ISOPEN|CARR_ON;
		tp->t_flags = XHTAB|ECHO|CRMOD|INDCTL|XVTAB;
		tp->t_erase = CERASE;
		tp->t_kill = CKILL;
		tp->t_htab = 8;
	}
	addr->klrcsr =| IENABLE|DSRDY|RDRENB;
	addr->kltcsr =| IENABLE;
	ttyopen( tp);
}

klclose(dev)
{
	register struct tty *tp;

	tp = &kl11[dev.d_minor];
	wflushtty(tp);
	tp->t_state = 0;
}

klread(dev)
{
	ttread(&kl11[dev.d_minor]);
}

klwrite(dev)
{
	ttwrite(&kl11[dev.d_minor]);
}

klxint(dev)
{
	register struct tty *tp;

	tp = &kl11[dev.d_minor];
	ttstart(tp);
	if (tp->t_outq.c_cc == 0 || tp->t_outq.c_cc == TTLOWAT)
		wakeup(&tp->t_outq);
}

klrint(dev)
{
	register int c, *addr;
	register struct tty *tp;

	tp = &kl11[dev.d_minor];
	addr = tp->t_addr;
	c = addr->klrbuf;
	addr->klrcsr =| RDRENB;
	if ((c&0177)==0)
		addr->kltbuf = c;	/* hardware botch */
	if ( (tp->t_flags &ALL8BITS) == 0 )
		c =& 0177;
	ttyinput(c, tp);
}

klsgtty(dev)
{
	register struct tty *tp;

	tp = &kl11[dev.d_minor];
	ttystty(tp);
}
