/*
 * ggcov - A GTK frontend for exploring gcov coverage data
 * Copyright (c) 2003-2005 Greg Banks <gnb@users.sourceforge.net>
 *
 *
 * TODO: attribution for decode-gcov.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * tggcov is a libcov-based reimplementation of the gcov commandline
 * program.  It's primary use is for regression testing, to allow
 * automated comparisons of the statistics generated by libcov and
 * the real ones from gcov.
 *
 * TODO: update
 */

#include "common.h"
#include "cov.H"
#include "filename.h"
#include "estring.H"
#include "tok.H"
#include "fakepopt.h"
#include "report.H"
#include "callgraph_diagram.H"
#include "check_scenegen.H"

CVSID("$Id: tggcov.c,v 1.24 2010-05-09 05:37:15 gnb Exp $");

char *argv0;
static list_t<const char> files;            /* incoming specification from commandline */

static int header_flag = FALSE;
static int blocks_flag = FALSE;
static int lines_flag = FALSE;
static int new_format_flag = FALSE;
static int status_flag = FALSE;
static int annotate_flag = FALSE;
static int check_callgraph_flag = FALSE;
static int dump_callgraph_flag = FALSE;
static const char *reports = 0;
static char *output_filename;

static const char *status_short_names[cov::NUM_STATUS] =
{
    "CO",
    "PC",
    "UN",
    "UI",
    "SU"
};

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#define BLOCKS_WIDTH    8

static void
annotate_file(cov_file_t *f)
{
    const char *cfilename = f->name();
    FILE *infp, *outfp = NULL;
    unsigned long lineno;
    cov_line_t *ln;
    char *ggcov_filename;
    char buf[1024];

    if ((infp = fopen(cfilename, "r")) == 0)
    {
	perror(cfilename);
	return;
    }

    if (output_filename && !strcmp(output_filename, "-"))
    {
	ggcov_filename = NULL;
	outfp = stdout;
    }
    else if (output_filename)
    {
	estring e = (const char *)output_filename;
	e.replace_all("{}", file_basename_c(cfilename));
	ggcov_filename = e.take();
    }
    else
    {
	ggcov_filename = g_strconcat(cfilename, ".tggcov", (char *)0);
    }

    if (ggcov_filename)
    {
	fprintf(stderr, "Writing %s\n", ggcov_filename);
	if ((outfp = fopen(ggcov_filename, "w")) == 0)
	{
	    perror(ggcov_filename);
	    g_free(ggcov_filename);
	    fclose(infp);
	    return;
	}
	g_free(ggcov_filename);
    }

    if (header_flag)
    {
	fprintf(outfp, "    Count       ");
	if (blocks_flag)
	    fprintf(outfp, "Block(s)");
	if (lines_flag)
	    fprintf(outfp, " Line   ");
	if (status_flag)
	    fprintf(outfp, " Status ");
	fprintf(outfp, " Source\n");

	fprintf(outfp, "============    ");
	if (blocks_flag)
	    fprintf(outfp, "======= ");
	if (lines_flag)
	    fprintf(outfp, "======= ");
	if (status_flag)
	    fprintf(outfp, "======= ");
	fprintf(outfp, "=======\n");
    }

    lineno = 0;
    while (fgets(buf, sizeof(buf), infp) != 0)
    {
	++lineno;
	ln = f->nth_line(lineno);

	if (new_format_flag)
	{
	    if (ln->status() != cov::UNINSTRUMENTED &&
		ln->status() != cov::SUPPRESSED)
	    {
		if (ln->count())
		    fprintf(outfp, "%9llu:%5lu:",
			    (unsigned long long)ln->count(),
			    lineno);
		else
		    fprintf(outfp, "    #####:%5lu:", lineno);
	    }
	    else
		fprintf(outfp, "        -:%5lu:", lineno);
	}
	else
	{
	    if (ln->status() != cov::UNINSTRUMENTED &&
		ln->status() != cov::SUPPRESSED)
	    {
		if (ln->count())
		    fprintf(outfp, "%12llu    ", (unsigned long long)ln->count());
		else
		    fputs("      ######    ", outfp);
	    }
	    else
		fputs("\t\t", outfp);
	}
	if (blocks_flag)
	{
	    char blocks_buf[BLOCKS_WIDTH];
	    ln->format_blocks(blocks_buf, BLOCKS_WIDTH-1);
	    fprintf(outfp, "%*s ", BLOCKS_WIDTH-1, blocks_buf);
	}
	if (lines_flag)
	{
	    fprintf(outfp, "%7lu ", lineno);
	}
	if (status_flag)
	{
	    fprintf(outfp, "%7s ", status_short_names[ln->status()]);
	}
	fputs(buf, outfp);
    }

    fclose(infp);
    if (outfp != stdout)
	fclose(outfp);
}

static void
annotate(void)
{
    for (list_iterator_t<cov_file_t> iter = cov_file_t::first() ; *iter ; ++iter)
	annotate_file(*iter);
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

static int report_lastlines;

static void
do_report(FILE *fp, const report_t *rep)
{
    /* ensure there is one line of empty space between each non-empty report */
    if (report_lastlines > 0)
	fputc('\n', fp);

    report_lastlines = (*rep->func)(fp, "stdout");
}

static void
report(void)
{
    FILE *fp = stdout;
    const report_t *rep;
    gboolean did_msg1 = FALSE;

    report_lastlines = -1;
    if (reports == 0 || *reports == '\0' || !strcmp(reports, "all"))
    {
	/* call all reports */
	for (rep = all_reports ; rep->name != 0 ; rep++)
	    do_report(fp, rep);
    }
    else if (!strcmp(reports, "list"))
    {
	/* print available reports and exit */
	printf("Available reports:\n");
	for (rep = all_reports ; rep->name != 0 ; rep++)
	    printf("    %s\n", rep->name);
	fflush(stdout);
	exit(0);
    }
    else
    {
	/* call only the named reports */
	tok_t tok(reports, ", ");
	const char *name;

	while ((name = tok.next()) != 0)
	{
	    for (rep = all_reports ; rep->name != 0 ; rep++)
	    {
		if (!strcmp(rep->name, name))
		{
		    do_report(fp, rep);
		    break;
		}
	    }
	    if (rep->name == 0)
	    {
		fprintf(stderr, "%s: unknown report name \"%s\"", argv0, name);
		if (!did_msg1)
		{
		    fputs(", use \"-R list\" to print listing.", stderr);
		    did_msg1 = TRUE;
		}
		fputc('\n', stderr);
	    }
	}
    }
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

static void
check_callgraph(void)
{
    check_scenegen_t *sg = new check_scenegen_t;
    callgraph_diagram_t *diag = new callgraph_diagram_t;

    diag->prepare();
    diag->render(sg);

    if (!sg->check())
	exit(1);

    delete diag;
    delete sg;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

static void
set_printable_location(cov_location_var &loc, const cov_location_t *from)
{
    if (from == 0)
	loc.set("-", 0);
    else
	loc.set(cov_file_t::minimise_name(from->filename), from->lineno);
}

static void
dump_callgraph(void)
{
    FILE *fp;
    const char *cgfilename = file_make_absolute("callgraph.tggcov");

    fprintf(stderr, "Writing %s\n", cgfilename);
    if ((fp = fopen(cgfilename, "w")) == 0)
    {
	perror(cgfilename);
	return;
    }

    ptrarray_t<cov_callnode_t> *nodes = new ptrarray_t<cov_callnode_t>;
    cov_callgraph_t *callgraph = cov_callgraph_t::instance();
    for (cov_callspace_iter_t csitr = callgraph->first() ; *csitr ; ++csitr)
    {
	for (cov_callnode_iter_t cnitr = (*csitr)->first() ; *cnitr ; ++cnitr)
	    nodes->append(*cnitr);
    }
    nodes->sort(cov_callnode_t::compare_by_name);

    fprintf(fp, "# tggcov callgraph version 1\n");
    fprintf(fp, "base %s\n", cov_file_t::common_path());
    unsigned int i;
    for (i = 0 ; i < nodes->length() ; i++)
    {
	cov_callnode_t * cn = nodes->nth(i);

	fprintf(fp, "callnode %s\n\tsource %s\n",
	    cn->name.data(),
	    (cn->function != 0 ? cn->function->file()->minimal_name() : "-"));

	for (list_iterator_t<cov_callarc_t> caitr = cn->out_arcs.first() ; *caitr ; ++caitr)
	{
	    cov_callarc_t *ca = *caitr;

	    fprintf(fp, "\tcallarc %s\n\t\tcount %llu\n",
		ca->to->name.data(),
		(unsigned long long)ca->count);
	}

	if (cn->function != 0)
	{
	    fprintf(fp, "function %s\n", cn->function->name());
	    cov_call_iterator_t *itr;

	    itr = new cov_function_call_iterator_t(cn->function);
	    while (itr->next())
	    {
		fprintf(fp, "\tblock %u\n", itr->block()->bindex());
		cov_location_var loc;
		set_printable_location(loc, itr->location());
		fprintf(fp, "\t\tcall %s\n\t\t\tcount %llu\n\t\t\tlocation %s\n",
			(itr->name() != 0 ? itr->name() : "-"),
			(unsigned long long)itr->count(),
			loc.describe());
	    }
	    delete itr;

	    const cov_location_t *first = cn->function->get_first_location();
	    const cov_location_t *last = cn->function->get_last_location();
	    itr = new cov_range_call_iterator_t(first, last);
	    while (itr->next())
	    {
		cov_location_var loc;
		set_printable_location(loc, itr->location());
		fprintf(fp, "\tlocation %s\n", loc.describe());
		fprintf(fp, "\t\tcall %s\n\t\t\tcount %llu\n",
			(itr->name() != 0 ? itr->name() : "-"),
			(unsigned long long)itr->count());
	    }
	    delete itr;
	}
    }

    fclose(fp);
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/*
 * With the old GTK, we're forced to parse our own arguments the way
 * the library wants, with popt and in such a way that we can't use the
 * return from poptGetNextOpt() to implement multiple-valued options
 * (e.g. -o dir1 -o dir2).  This limits our ability to parse arguments
 * for both old and new GTK builds.  Worse, gtk2 doesn't depend on popt
 * at all, so some systems will have gtk2 and not popt, so we have to
 * parse arguments in a way which avoids potentially buggy duplicate
 * specification of options, i.e. we simulate popt in fakepopt.c!
 */
static poptContext popt_context;
static struct poptOption popt_options[] =
{
    {
	"report",                               /* longname */
	'R',                                    /* shortname */
	POPT_ARG_STRING,                        /* argInfo */
	&reports,                               /* arg */
	0,                                      /* val 0=don't return */
	"display named reports or \"all\"",     /* descrip */
	0                                       /* argDescrip */
    },
    {
	"annotate",                             /* longname */
	'a',                                    /* shortname */
	POPT_ARG_NONE,                          /* argInfo */
	&annotate_flag,                         /* arg */
	0,                                      /* val 0=don't return */
	"save annotated source to FILE.tggcov", /* descrip */
	0                                       /* argDescrip */
    },
    {
	"blocks",                               /* longname */
	'B',                                    /* shortname */
	POPT_ARG_NONE,                          /* argInfo */
	&blocks_flag,                           /* arg */
	0,                                      /* val 0=don't return */
	"in annotated source, display block numbers",/* descrip */
	0                                       /* argDescrip */
    },
    {
	"header",                               /* longname */
	'H',                                    /* shortname */
	POPT_ARG_NONE,                          /* argInfo */
	&header_flag,                           /* arg */
	0,                                      /* val 0=don't return */
	"in annotated source, display header line", /* descrip */
	0                                       /* argDescrip */
    },
    {
	"lines",                                /* longname */
	'L',                                    /* shortname */
	POPT_ARG_NONE,                          /* argInfo */
	&lines_flag,                            /* arg */
	0,                                      /* val 0=don't return */
	"in annotated source, display line numbers", /* descrip */
	0                                       /* argDescrip */
    },
    {
	"status",                               /* longname */
	'S',                                    /* shortname */
	POPT_ARG_NONE,                          /* argInfo */
	&status_flag,                           /* arg */
	0,                                      /* val 0=don't return */
	"in annotated source, display line status", /* descrip */
	0                                       /* argDescrip */
    },
    {
	"new-format",                           /* longname */
	'N',                                    /* shortname */
	POPT_ARG_NONE,                          /* argInfo */
	&new_format_flag,                       /* arg */
	0,                                      /* val 0=don't return */
	"in annotated source, display count in new gcc 3.3 format", /* descrip */
	0                                       /* argDescrip */
    },
    {
	"check-callgraph",                      /* longname */
	'G',                                    /* shortname */
	POPT_ARG_NONE,                          /* argInfo */
	&check_callgraph_flag,                  /* arg */
	0,                                      /* val 0=don't return */
	"generate and check callgraph diagram", /* descrip */
	0                                       /* argDescrip */
    },
    {
	"dump-callgraph",                       /* longname */
	'P',                                    /* shortname */
	POPT_ARG_NONE,                          /* argInfo */
	&dump_callgraph_flag,                   /* arg */
	0,                                      /* val 0=don't return */
	"dump callgraph data in text form",     /* descrip */
	0                                       /* argDescrip */
    },
    {
	"output",                               /* longname */
	'o',                                    /* shortname */
	POPT_ARG_STRING,                        /* argInfo */
	&output_filename,                       /* arg */
	0,                                      /* val 0=don't return */
	"output file for annotation",           /* descrip */
	0                                       /* argDescrip */
    },
    COV_POPT_OPTIONS
    POPT_AUTOHELP
    POPT_TABLEEND
};

static void
parse_args(int argc, char **argv)
{
    const char *file;

    argv0 = argv[0];

    popt_context = poptGetContext(PACKAGE, argc, (const char**)argv,
				  popt_options, 0);
    poptSetOtherOptionHelp(popt_context,
			   "[OPTIONS] [executable|source|directory]...");

    int rc;
    while ((rc = poptGetNextOpt(popt_context)) > 0)
	;
    if (rc < -1)
    {
	fprintf(stderr, "%s:%s at or near %s\n",
	    argv[0],
	    poptStrerror(rc),
	    poptBadOption(popt_context, POPT_BADOPTION_NOALIAS));
	exit(1);
    }

    while ((file = poptGetArg(popt_context)) != 0)
	files.append(file);

    poptFreeContext(popt_context);

    cov_post_args();

    if (debug_enabled(D_DUMP|D_VERBOSE))
    {
	duprintf1("parse_args: blocks_flag=%d\n", blocks_flag);
	duprintf1("parse_args: header_flag=%d\n", header_flag);
	duprintf1("parse_args: lines_flag=%d\n", lines_flag);
	duprintf1("parse_args: reports=\"%s\"\n", reports);
    }
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#define DEBUG_GLIB 1
#if DEBUG_GLIB

static const char *
log_level_to_str(GLogLevelFlags level)
{
    static char buf[32];

    switch (level & G_LOG_LEVEL_MASK)
    {
    case G_LOG_LEVEL_ERROR: return "ERROR";
    case G_LOG_LEVEL_CRITICAL: return "CRITICAL";
    case G_LOG_LEVEL_WARNING: return "WARNING";
    case G_LOG_LEVEL_MESSAGE: return "MESSAGE";
    case G_LOG_LEVEL_INFO: return "INFO";
    case G_LOG_LEVEL_DEBUG: return "DEBUG";
    default:
	snprintf(buf, sizeof(buf), "%d", level);
	return buf;
    }
}

void
log_func(
    const char *domain,
    GLogLevelFlags level,
    const char *msg,
    gpointer user_data)
{
    fprintf(stderr, "%s:%s:%s\n",
	(domain == 0 ? PACKAGE : domain),
	log_level_to_str(level),
	msg);
    if (level & G_LOG_FLAG_FATAL)
	exit(1);
}

#endif
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

int
main(int argc, char **argv)
{
#if DEBUG_GLIB
    g_log_set_handler("GLib",
		      (GLogLevelFlags)(G_LOG_LEVEL_MASK|G_LOG_FLAG_FATAL),
		      log_func, /*user_data*/0);
#endif

    parse_args(argc, argv);
    cov_read_files(files);

    cov_dump(stderr);

    if (reports)
	report();
    if (annotate_flag)
	annotate();
    if (check_callgraph_flag)
	check_callgraph();
    if (dump_callgraph_flag)
	dump_callgraph();

    return 0;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/*END*/
