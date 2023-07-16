#
/*
 * cptree
 * From Glasgow Uni February 1980
 * Not clear who wrote this originally
 *
 * Flags: for flags see the manual page cptree.1
 *
 * I've made some minor cosmetic changes
 * and added this comment
 * Peter Collinson July 2023
 */

#define	USAGE_IS	"Usage is:  cptree (flags) {in directory} {out directory}\n"
#define	UNKNOWN_FLAG	"Unkown flag:  %s\n", argerr
#define	IN_FILE		"Warning - input file: %s"
#define	OUT_FILE	"output file: %s"
#define	IN_DIR		"Warning - input directory: %s"
#define	OUT_DIR		"output directory: %s"
#define	TYPE		060000
#define	PLAIN		000000
#define	DIRECTORY	040000
#define	CHAR_SPECIAL	020000
#define	BLOCK_SPECIAL	060000

struct	{
	char	low, high;
	};

struct	ibuf	{
	int	device;
	int	inumber;
	int	flags;
	char	nlinks;
	char	uid;
	char	gid;
	char	sizeo;
	char	size1;
	int	addr[8];
	int	actime[2];
	int	modtime[2];
	};

struct	entry	{
	int	inode;
	char	name[14];
	};

int	acc_flag	0;
int	mod_flag	0;
int	all_flag	0;
int	dir_flag	0;
int	overwrite_flag	0;
int	print_flag	1;
int	verify_flag	1;
int	same_owner	0;
int	unlink_flag	0;
int	depth_flag	0;
int	time_now[2], owner, pid, newpid, depth_limit;
int	acc_time[2], mod_time[2];
char	buf[512], in_name[100], out_name[100], *c_in, *c_out;
char	*arg, *argerr, *name, *bp;

main( argc,argv )
int	argc;
char	**argv;
	{

	argv++;    argc--;
	time( time_now);
	while ( **argv == '-')  {
		arg = argerr = *argv;    arg++;
		switch( *arg++ )  {
		case 'A': set_flag( &acc_flag, acc_time, "accessed");
			break;
		case 'M': set_flag( &mod_flag, mod_time, "modified");
			break;
		case 'D': depth_flag = 1;
			depth_limit =  atoi( arg);
			if ( depth_limit == 0 )  {
				printf( UNKNOWN_FLAG);
				exit(-1);
				}
			while ( *arg >= '0'  &&  *arg <= '9' )
				arg++;
			break;
		case 'd': dir_flag = 1;
			break;
		case 'a': all_flag = 1;
			break;
		case 't': if (getuid() ==0)
				same_owner = 1;
			else	{
				printf("No permission for -t flag\n");
				exit(-1);
				}
			break;
		case 'u': unlink_flag = 1;
			break;
		case 'o': overwrite_flag = 1;
			break;
		case 'p': print_flag = 0;
			break;
		default: printf( UNKNOWN_FLAG);
			exit(-1);
			}
		if ( *arg != '\0' )  {
			printf( UNKNOWN_FLAG );
			exit(-1);
			}
		argv++;    argc--;
		}
	if ( argc != 2)  {
		printf( USAGE_IS );
		exit(-1);
		}

	arg = *argv++;
	c_in = in_name;
	while ( (*c_in++ = *arg++) !='\0');
	arg = *argv;
	c_out = out_name;
	while( (*c_out++ = *arg++) !='\0');

	/* recursively copy in_name to out_name */
	exit( cptree(0));
	}


cptree( depth)
int	depth;
	{
	char	*in_path_end, *out_path_end;
	int	file_in, file_out, mode, length, blocks, i;
	int	files_copied, new_directory;
	struct	entry	dir_entry[32];
	struct	ibuf	status;

	depth++;
	if ( stat( in_name, &status) <0 )  {
		printf("%s not found\n", in_name);
		exit(-1);
		}
	mode = status.flags;
	owner.low = status.uid;
	owner.high = status.gid;

	/* Is it a special file ? */
	if ( (mode &TYPE) ==CHAR_SPECIAL || (mode &TYPE) ==BLOCK_SPECIAL )  {
		spaces( depth);
		printf( IN_FILE, "special file, no copy\n", in_name);
		return(0);
		}

	/* Copy a plain file */
	if ( (mode &TYPE) ==PLAIN )  {
		/* If output file exists, it must be plain */
		if ( comp( status.modtime, mod_time, mod_flag )
			||  comp( status.actime, acc_time, acc_flag) )
				return(0);
		if ( stat( out_name, &status) >=0  &&  overwrite_flag )
			return(0);
		if ( (status.flags &TYPE) !=PLAIN )  {
			spaces( depth);
			printf( IN_FILE, "non-plain output file", in_name);
			printf(" exists, no copy\n");
			return(0);
			}

		if ( (file_in=open( in_name, 0)) <0)  {
			spaces( depth);
			printf( IN_FILE, "not readable, no copy\n", in_name);
			return(0);
			}
		if ( (file_out=creat( out_name, mode)) <0)  {
			printf( IN_FILE, "cannot create", in_name);
			printf( OUT_FILE, "no copy\n", out_name);
			return(0);
			}
		chmod( out_name, mode&07777);
		if ( same_owner )
			chown( out_name, owner );

		while ( (length=read( file_in, buf, 512)) >0)
			if ( write( file_out, buf, length) !=length)  {
				printf( "ERROR - write error on ");
				printf( OUT_FILE, "copy aborted\n", out_name);
				exit(-1);
				}
		close( file_in);
		close( file_out);
		if ( length <0)  {
			spaces( depth);
			printf("Error - Input file %s, read error\n", in_name);
			return( 1 );
			}
		if ( print_flag)  {
			spaces( depth);
			c_in = in_name;
			while ( *c_in != '\0' )
				if ( *c_in++ == '/')
					name = c_in;
			printf("%s\n", name);
			}
		if ( unlink_flag)
			unlink( in_name);
		return(1);
		}


	/* Copy a directory, calling cptree for each file */
	/* The strings in_name and out_name will be extended to
	 * longer pathnames in this and lower calls, so save the
	 * current pathname ends in local variables */
	if ( depth_flag  &&  depth > depth_limit )
		return(0);
	in_path_end = in_name;
	while ( *in_path_end != '\0')
		in_path_end++;
	out_path_end = out_name;
	while ( *out_path_end != '\0')
		out_path_end++;

	/* If out_name exists, then it must be a directory */
	new_directory = 0;
	if ( stat( out_name, &status) >= 0)  {
		if ( (status.flags &TYPE) !=DIRECTORY )  {
			spaces( depth);
			printf( IN_DIR, "non-directory output file", in_name);
			printf(" exists, no copy\n");
			return(0);
			}
		}
	else	{
		/* It doesn't exist, so make a new directory */
		pid = getpid();
		newpid = fork();
		if ( pid != getpid() )  {
			execl("/bin/mkdir", "", out_name, 0);
			printf(IN_DIR, "try again\n", in_name);
			exit(-1);
			}
		while ( wait() != newpid );
		if ( stat( out_name, &status) <0 )  {
			spaces( depth);
			printf( IN_DIR, "cannot create ", in_name);
			printf( OUT_DIR, "no copy\n", out_name);
			return(0);
			}
		new_directory = 1;
		if ( same_owner )
			chown( out_name, owner);
		}

	/* Copy the files */
	files_copied = 0;
	blocks = 0;
	if ( print_flag)  {
		spaces( depth);
		printf("searching: %s\n", in_name);
		}
	if ( (file_in=open(in_name,0)) <0)  {
		spaces( depth);
		printf( IN_DIR, "not readable, no copy\n", in_name);
		return(0);
		}
	while ( (length=read( file_in, &dir_entry, 512)) > 0)  {
		blocks++;
		close( file_in);
		length =>>4;
		for ( i=0; i<length; i++)  {
			if ( dir_entry[i].inode == 0 )
				continue;
			if ( *(name=dir_entry[i].name) == '.' )  {
				if ( all_flag == 0 )
					continue;
				if ( *++name == '\0'  ||
					(*name=='.'  &&  *++name=='\0') )
						continue;
				}
			/* concatenate this filename onto the pathnames*/
			c_in = in_path_end;
			c_out = out_path_end;
			*c_in++ = *c_out++ = '/';
			bp = name = (dir_entry[i]).name;
			while ( *name != '\0' && name < bp+14 )
				*c_in++ = *c_out++ = *name++;
			*c_in = *c_out = '\0';
			files_copied =+ cptree(depth);
			}
		*in_path_end = '\0';
		file_in = open( in_name, 0);
		seek( file_in, blocks*512, 0);
		}
	close( file_in);
	if ( length < 0)  {
		printf( IN_DIR, "read error\n", in_name);
		}
	*out_path_end = '\0';
	if ( new_directory )
		chmod( out_name, mode&07777);
	if ( dir_flag == 0  &&  new_directory  &&  files_copied == 0 )  {
		newpid = fork();
		if ( pid != getpid() )  {
			execl("/bin/rmdir", "", out_name, 0);
			printf("rmdir  %s  failed\n",  out_name );
			exit(-1);
			}
		while ( wait() != newpid );
		}
	return( files_copied);
	}


comp( file_date, limit_date, side_of)
int	file_date[2], limit_date[2], side_of;
	{
	int	c[2];
	lsub( c, limit_date, file_date );
	if ( side_of*c[0] >= 0 )
		return(0);
	return(-1);
	}


set_flag( flag, time, mod_acc)
int	*flag, time[2];
char	*mod_acc;
	{
	int	hours, c[2];
	char	*before_after;

	hours = 24*atoi( arg);
	while ( *arg >='0'  &&  *arg <='9' )
		arg++;
	if ( *arg == ':' )  {
		hours =+ atoi( ++arg );
		while ( *arg >='0'  &&  *arg <='9' )
			arg++;
		}
	c[1] = hours*3600;
	c[0] = hmul( hours, 3600);
	lsub( time, time_now, c );
	switch (*arg++)  {
	case 'b': *flag = 1;
		before_after = "before";
		break;
	case 'a': *flag = -1;
		before_after = "after";
		break;
	default:  printf( UNKNOWN_FLAG );
		exit(-1);
		}
	if ( print_flag )
		printf("Copying files last %s %s  %s", mod_acc, before_after,
			ctime( time) );
	}


spaces( depth)
int	depth;
	{
	int	i;
	for ( i=1; i<depth; i++)
		printf("    ");
	}
