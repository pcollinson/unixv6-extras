/*
 * A clist structure is the head
 * of a linked list queue of characters.
 * The characters are stored in 4-word
 * blocks containing a link and 6 characters.
 * The routines getc, putc and zapc (m45.s or m40.s)
 * manipulate these structures.
 */
struct	clist  {
	int	c_cc;		/* character count */
	char	*c_cf;		/* pointer to first block */
	char	*c_cl;		/* pointer to last block */
	};

/*
 * A tty structure is needed for
 * each UNIX character device that
 * is used for normal terminal IO.
 * The routines in tty.c handle the
 * common code associated with
 * these structures.
 * The definition and device dependent
 * code is in each driver. (kl.c dc.c dh.c)
 */
struct	tty  {
	struct clist	t_inq;		/* input list from device */
	struct clist	t_outq;		/* output list to device */
	int		t_dev;		/* device name */
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
	int		*t_addr;	/* device addr (registers or start routine */
	int		t_state;	/* internal state, not visible */
	char		t_line;		/* line number on screen */
	char		t_col;		/* column number on line */
	char		t_delct;	/* number of delimiters in queue */
	char		t_char;		/* scratch byte for the driver */
	};


#define	TTIPRI	10
#define	TTOPRI	20

/* Characters now in deftty.h */

/* limits */
#define	TTHIWAT	50
#define	TTLOWAT	20
#define	TTYHOG	132

/* modes */
#define	HUPCL		01
#define	XHTAB		02
#define	CAPSONLY	04
#define	ECHO		010
#define	CRMOD		020
#define	RAW		040
#define	ODDP		0100
#define	EVENP		0200
#define	SCOPE		0400
#define INDCTL		01000
#define	USERBREAK	02000
#define	ALL8BITS	04000
#define	HOLD		010000
#define	BREAKS		020000
#define	XVTAB		040000
#define	DATA100		0100000

/* Internal state bits */
#define	TIMEOUT		01	/* Delay timeout in progress */
#define	WOPEN		02	/* Waiting for open to complete */
#define	ISOPEN		04	/* Device is open */
#define	SSTART		010	/* Has special start routine at addr */
#define	CARR_ON		020	/* Software copy of carrier-present */
#define	BUSY		040	/* Output in progress */
#define	ASLEEP		0100	/* Wakeup when output done */
#define LITERAL		0200	/* last char was a literal escape */
#define	XHOLD		0400	/* output is being held up */
#define	XSTOP		01000	/* stop write from filling the outq */
#define	XDELETE		02000	/* delete output */
#define ERASING		04000	/* erase string in progress */

/* ERASING brackets */
#define CERASEOPEN	'['
#define CERASECLOSE	']'
