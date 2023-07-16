#
/*
 * Print test config file
 *
 * really a test routine
 */

#include "lib/lib.h"

main()
{

	if (rdconfig("conf.ini") == 0) {
		printf("%s %s\n", "device", device);
 		printf("%s %s\n", "mount", mountpt);
		printf("%s %s\n", "safety", safety);
	}

	exit(0);
}
