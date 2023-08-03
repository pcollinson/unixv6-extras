/*
 * Added as a system call to cleanly stop the system
 * Peter C Nov 1976
 * revised 2023
 */
stopunix()
{
	if(suser()) {
		/* if still updating then return to caller */
		if (updlock)
			u.u_error = EBUSY;
		else {
			update();
			stopit();
		}
	}
}
