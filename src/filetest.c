#include "filename.h"

int
main(int argc, char **argv)
{
    int i;
    char *in, *out, *expected;
    
    for (i = 1 ; i < argc ; i++)
    {
    	if (!strcmp(argv[i], "make_absolute"))
	{
	    in = argv[++i];
	    expected = argv[++i];

    	    out = file_make_absolute(in);
	    fprintf(stderr, "file_make_absolute(\"%s\") => \"%s\"\n", in, out);
	    if (strcmp(out, expected))
	    {
	    	fprintf(stderr, "ERROR: expected \"%s\" got \"%s\"\n",
		    	    expected, out);
	    	exit(1);
	    }
	    g_free(out);
	}
	else
	{
	    fprintf(stderr, "Unknown function to test: %s\n", argv[i]);
	}
    }
    
    return 0;
}

