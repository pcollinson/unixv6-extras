/*
 * Abstract tty character settings and modes into a separate
 * file which is then usable from stty, getty and any other
 * code that wants to use the values
 *
 * Peter Collinson
 * July 2023
 */

/* Characters */
#define	CNEXTLINE	012	/* newline, print another line in paging */
#define	CERASE		0177	/* rubout, the default character delete */
#define	CINTR		03	/* ^c, interrupt process */
#define	CEOF		04	/* ^d, end of file indicator */
#define	CXOFF		023	/* ^s, stop transmission */
#define	CXON		021	/* ^q, restart transmission */
#define	CXDELETE	017	/* ^o, delete output */
#define CBELL		07
#define CVTAB		013
#define CFORM		014
#define	CRETYPE		022	/* ^r, retype current line */
#define CSTATUS		024	/* ^t, report system status - not implemented */
#define	CLITERAL	026	/* ^v, the literal next char */
#define CDWORD		027	/* ^w, word delete character */
#define	CKILL		025	/* ^u, the default line delete char */
#define	CQUIT		034	/* ^\, quit (eat soggy cardboard, constipated pinkos!) */
#define	CDELIM		0377	/* internal delimiter, also used in zapc (m40.s) */

/* Modes */
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
