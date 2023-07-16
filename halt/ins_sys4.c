/*
 * Added as a system call to cleanly stop the system
 * Peter C Nov 1976
 */
stopunix()
{
	if(suser())
	{
		update();
		/* was 5 for real hardware */
		u.u_ar0[R0] = 2;
		sslep();
		stopit();
	}
}
