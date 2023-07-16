#
/*
 * Emergency stty to set the control characters
 * needed until getty, login and stty are changed
 * assumes that fd 1 is the terminal
 */

/* don't use /usr/sys because things may not be
 * installed there as yet
 */
#include "sys/deftty.h"

struct {
	char    ispeed, ospeed;
	char    erase, kill;
	int     mode;
} st;

main()
{
	gtty(1, &st);
	st.erase = ERASE;
	st.kill = CKILL;
	stty(1, &st);
}
