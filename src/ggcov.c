/*
 * ggcov - A GTK frontend for exploring gcov coverage data
 * Copyright (c) 2001-2004 Greg Banks <gnb@users.sourceforge.net>
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

#include "common.h"
#include "cov.H"
#include "ui.h"
#include "filename.h"
#include "prefs.H"
#include "estring.H"
#include "tok.H"
#include "sourcewin.H"
#include "summarywin.H"
#include "callswin.H"
#include "callgraphwin.H"
#include "diagwin.H"
#include "callgraph_diagram.H"
#include "lego_diagram.H"
#include "functionswin.H"
#include "fileswin.H"
#include "reportwin.H"
#include <libgnomeui/libgnomeui.h>
#include "fakepopt.h"

CVSID("$Id: ggcov.c,v 1.53 2010-05-09 05:37:15 gnb Exp $");

#define DEBUG_GTK 1

char *argv0;
static list_t<const char> files;            /* incoming specification from commandline */

static const char ** debug_argv;
static const char *initial_windows = "summary";
static int profile_mode = FALSE;

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/*
 * Construct and return a read-only copy of the given commandline
 * argument vector.  It's read-only because the whole vector is
 * collapsed into a single memory allocation for efficiency.
 */

static void
stash_argv(int argc, char **argv)
{
    int i, len;
    char **nargv, *p;

    len = 0;
    for (i = 0 ; i < argc ; i++)
	len += sizeof(char*) + strlen(argv[i]) + 1/*trailing nul char*/;
    len += sizeof(char*)/* trailing NULL ptr */ ;

    nargv = (char **)gnb_xmalloc(len);
    p = (char *)(nargv + argc + 1);

    for (i = 0 ; i < argc ; i++)
    {
	nargv[i] = p;
	strcpy(p, argv[i]);
	p += strlen(p) + 1;
    }
    nargv[i] = 0;

    debug_argv = (const char **)nargv;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*
 * Read a file from the File->Open dialog.
 */
gboolean
ggcov_read_file(const char *filename)
{
    if (file_is_directory(filename) == 0)
    {
	if (!cov_read_directory(filename, /*recursive*/FALSE))
	    return FALSE;
    }
    else if (errno != ENOTDIR)
    {
	perror(filename);
    }
    else if (file_is_regular(filename) == 0)
    {
	if (cov_is_source_filename(filename))
	{
	    if (!cov_read_source_file(filename))
		return FALSE;
	}
	else
	{
	    if (!cov_read_object_file(filename))
		return FALSE;
	}
    }
    else
    {
	/* TODO: gui alert to user */
	fprintf(stderr, "%s: don't know how to handle this filename\n",
		filename);
	return FALSE;
    }

    return TRUE;
}

static GtkWidget *open_window = 0;

GLADE_CALLBACK void
on_open_ok_button_clicked(GtkWidget *w, gpointer userdata)
{
    const char *filename;

    dprintf0(D_UICORE, "on_open_ok_button_clicked\n");

    filename = gtk_file_selection_get_filename(
		    GTK_FILE_SELECTION(open_window));

    if (filename != 0 && *filename != '\0')
    {
	cov_pre_read();
	if (ggcov_read_file(filename))
	    cov_post_read();
    }

    gtk_widget_hide(open_window);

    /*
     * No files loaded: user started ggcov without a commandline
     * argument and failed to load a file in the Open dialog.
     */
    if (!*cov_file_t::first())
	exit(1);
}

GLADE_CALLBACK void
on_open_cancel_button_clicked(GtkWidget *w, gpointer userdata)
{
    dprintf0(D_UICORE, "on_open_cancel_button_clicked\n");
    gtk_widget_hide(open_window);

    /*
     * No files loaded: user started ggcov without a commandline
     * argument and cancelled the Open dialog.
     */
    if (!*cov_file_t::first())
	exit(1);
}

GLADE_CALLBACK void
on_file_open_activate(GtkWidget *w, gpointer userdata)
{
    dprintf0(D_UICORE, "on_file_open_activate\n");

    if (open_window == 0)
    {
	GladeXML *xml;

	xml = ui_load_tree("open");
	open_window = glade_xml_get_widget(xml, "open");
    }

    gtk_widget_show(open_window);
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

static void
on_windows_new_summarywin_activated(GtkWidget *w, gpointer userdata)
{
    summarywin_t *sw;

    sw = new summarywin_t();
    sw->show();
}

static void
on_windows_new_fileswin_activated(GtkWidget *w, gpointer userdata)
{
    fileswin_t *fw;

    fw = new fileswin_t();
    fw->show();
}

static void
on_windows_new_functionswin_activated(GtkWidget *w, gpointer userdata)
{
    functionswin_t *fw;

    fw = new functionswin_t();
    fw->show();
}

static void
on_windows_new_callswin_activated(GtkWidget *w, gpointer userdata)
{
    callswin_t *cw;

    cw = new callswin_t();
    cw->show();
}

static void
on_windows_new_callgraphwin_activated(GtkWidget *w, gpointer userdata)
{
    callgraphwin_t *cgw;

    cgw = new callgraphwin_t();
    cgw->show();
}

static void
on_windows_new_callgraph2win_activated(GtkWidget *w, gpointer userdata)
{
    diagwin_t *lw;

    lw = new diagwin_t(new callgraph_diagram_t);
    lw->show();
}

static void
on_windows_new_legowin_activated(GtkWidget *w, gpointer userdata)
{
    diagwin_t *lw;

    lw = new diagwin_t(new lego_diagram_t);
    lw->show();
}

static void
on_windows_new_sourcewin_activated(GtkWidget *w, gpointer userdata)
{
    sourcewin_t *srcw;
    list_iterator_t<cov_file_t> iter = cov_file_t::first();
    cov_file_t *f = *iter;

    srcw = new sourcewin_t();
    srcw->set_filename(f->name(), f->minimal_name());
    srcw->show();
}

static void
on_windows_new_reportwin_activated(GtkWidget *w, gpointer userdata)
{
    reportwin_t *rw;

    rw = new reportwin_t();
    rw->show();
}

#include "ui/ggcov32.xpm"

static const struct window
{
    const char *name;
    const char *label;
    void (*create)(GtkWidget*, gpointer);
}
windows[] =
{
    {"summary", N_("New Summary..."), on_windows_new_summarywin_activated},
    {"files", N_("New File List..."), on_windows_new_fileswin_activated},
    {"functions", N_("New Function List..."), on_windows_new_functionswin_activated},
    {"calls", N_("New Calls List..."), on_windows_new_callswin_activated},
    {"callbutterfly", N_("New Call Butterfly..."), on_windows_new_callgraphwin_activated},
    {"callgraph", N_("New Call Graph..."), on_windows_new_callgraph2win_activated},
    {"lego", N_("New Lego Diagram..."), on_windows_new_legowin_activated},
    {"source", N_("New Source..."), on_windows_new_sourcewin_activated},
    {"reports", N_("New Report..."), on_windows_new_reportwin_activated},
    {0, 0, 0}
};

static void
ui_create(const char *full_argv0)
{
    /*
     * If we're being run from the source directory, fiddle the
     * glade search path to point to ../ui first.  This means
     * we can run ggcov before installation without having to
     * compile with UI_DEBUG=1.
     */
    estring dir = file_make_absolute(full_argv0);
    const char *p = strrchr(dir, '/');
    if (p != 0 &&
	(p -= 4) >= dir.data() &&
	!strncmp(p, "/src/", 5))
    {
	dir.truncate_to(p - dir);
	dir.append_string("/ui");
	if (!file_is_directory(dir))
	{
	    fprintf(stderr, "ggcov: running from source directory, "
			    "so prepending %s to glade search path\n",
			    dir.data());
	    ui_prepend_glade_path(dir);
	}
    }

    const struct window *wp;
    for (wp = windows ; wp->name ; wp++)
	ui_register_windows_entry(_(wp->label), wp->create, 0);

    ui_set_default_icon(ggcov32_xpm);

    prefs.load();

    if (!files.head())
    {
	/* Nothing on commandline...show the File->Open dialog to get some */
	on_file_open_activate(0, 0);
	while (!*cov_file_t::first())
	    gtk_main_iteration();
    }

    /* Possibly have files from commandline or dialog...show initial windows */
    tok_t tok(initial_windows, ", \n\r");
    const char *name;
    int nwindows = 0;
    while ((name = tok.next()) != 0)
    {
	for (wp = windows ; wp->name ; wp++)
	{
	    if (!strcmp(name, wp->name))
	    {
		wp->create(0, 0);
		nwindows++;
		break;
	    }
	}
	if (!wp->name)
	    fprintf(stderr, "%s: unknown window name \"%s\"\n", argv0, name);
    }

    if (!nwindows)
	on_windows_new_summarywin_activated(0, 0);

    if (profile_mode)
    {
	while (g_main_context_iteration(NULL, FALSE))
	    ;
	exit(0);
    }
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
static const struct poptOption popt_options[] =
{
    {
	"initial-windows",                      /* longname */
	'w',                                    /* shortname */
	POPT_ARG_STRING,                        /* argInfo */
	&initial_windows,                       /* arg */
	0,                                      /* val 0=don't return */
	"list of windows to open initially",    /* descrip */
	0                                       /* argDescrip */
    },
    {
	"profile",                              /* longname */
	0,                                      /* shortname */
	POPT_ARG_NONE,                          /* argInfo */
	&profile_mode,                          /* arg */
	0,                                      /* val 0=don't return */
	0,                                      /* descrip */
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

#if HAVE_GNOME_PROGRAM_INIT
    /* gnome_program_init() already has popt options */
#elif GTK2
    popt_context = poptGetContext(PACKAGE, argc, (const char**)argv,
				  popt_options, 0);
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
#endif

    while ((file = poptGetArg(popt_context)) != 0)
    {
	/* transparently handle file: URLs for Nautilus integration */
	if (!strncmp(file, "file://", 7))
	    file += 7;
	files.append(file);
    }

    poptFreeContext(popt_context);

    cov_post_args();

    if (debug_enabled(D_DUMP|D_VERBOSE))
    {
	const char **p;

	duprintf0("parse_args: argv[] = {");
	for (p = debug_argv ; *p ; p++)
	{
	    if (strpbrk(*p, " \t\"'") == 0)
		duprintf1(" %s", *p);
	    else
		duprintf1(" \"%s\"", *p);
	}
	duprintf0(" }\n");
    }
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

int
main(int argc, char **argv)
{
    ui_log_init();

    /* stash a copy of argv[] in case we want to dump it for debugging */
    stash_argv(argc, argv);

#if HAVE_GNOME_PROGRAM_INIT
    GnomeProgram *prog;

    prog = gnome_program_init(PACKAGE, VERSION, LIBGNOMEUI_MODULE,
			      argc, argv,
			      GNOME_PARAM_POPT_TABLE, popt_options,
			      GNOME_PROGRAM_STANDARD_PROPERTIES,
			      GNOME_PARAM_NONE);
    g_object_get(prog, GNOME_PARAM_POPT_CONTEXT, &popt_context, (char *)0);
#elif GTK2
    gtk_init(&argc, &argv);
    /* As of 2.0 we don't need to explicitly initialise libGlade anymore */
#else
    gnome_init_with_popt_table(PACKAGE, VERSION, argc, argv,
			       popt_options, /*popt flags*/0,
			       &popt_context);
    glade_gnome_init();
#endif

    parse_args(argc, argv);
    cov_read_files(files);

    cov_dump(stderr);
    ui_create(argv[0]);
    gtk_main();

    return 0;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/*END*/
