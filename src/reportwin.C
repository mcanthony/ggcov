/*
 * ggcov - A GTK frontend for exploring gcov coverage data
 * Copyright (c) 2004 Greg Banks <gnb@users.sourceforge.net>
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

#include "reportwin.H"
#include "sourcewin.H"
#include "filename.h"
#include "cov.H"
#include "estring.H"
#include "prefs.H"
#include "uix.h"
#include "gnbstackedbar.h"

CVSID("$Id: reportwin.C,v 1.4 2010-05-09 05:37:15 gnb Exp $");

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

static int
null_report_func(FILE *, const char *)
{
    return 0;
}

static const report_t null_report =  { "none", N_("None"), null_report_func, NULL };

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

reportwin_t::reportwin_t()
{
    GladeXML *xml;
    GtkWidget *w;

    /* load the interface & connect signals */
    xml = ui_load_tree("report");

    set_window(glade_xml_get_widget(xml, "report"));

    w = glade_xml_get_widget(xml, "report_report_combo");
    report_combo_ = init(UI_COMBO(w));
    text_ = glade_xml_get_widget(xml, "report_text");
    ui_text_setup(text_);
    save_as_button_ = glade_xml_get_widget(xml, "report_save_as_button");

    ui_register_windows_menu(ui_get_dummy_menu(xml, "report_windows_dummy"));

    report_ = &null_report;
}


reportwin_t::~reportwin_t()
{
    if (save_dialog_)
	gtk_widget_destroy(save_dialog_);
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

void
reportwin_t::populate_report_combo()
{
    const report_t *rep;
    clear(report_combo_);
    add(report_combo_, _(null_report.label), (gpointer)&null_report);
    for (rep = all_reports ; rep->name != 0 ; rep++)
	add(report_combo_, _(rep->label), (gpointer)rep);
    done(report_combo_);
    set_active(report_combo_, (gpointer)report_);
}

void
reportwin_t::populate()
{
    dprintf0(D_REPORTWIN, "reportwin_t::populate\n");

    populating_ = TRUE;     /* suppress combo entry callbacks */
    populate_report_combo();
    populating_ = FALSE;

    update();
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

void
reportwin_t::grey_items()
{
    gtk_widget_set_sensitive(save_as_button_, (report_ != &null_report));
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

static FILE *
open_temp_file()
{
    char *fname;
    int fd;
    FILE *fp = 0;

    fname = g_strdup("/tmp/gcov-reportXXXXXX");
    if ((fd = mkstemp(fname)) < 0)
    {
	perror(fname);
    }
    else if ((fp = fdopen(fd, "w+")) == 0)
    {
	perror(fname);
	close(fd);
    }
    g_free(fname);
    return fp;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

void
reportwin_t::update()
{
    FILE *fp;
    int n;
    char buf[1024];

    dprintf0(D_REPORTWIN, "reportwin_t::update\n");

    grey_items();

    populating_ = TRUE;
    assert(report_ != 0);
    set_active(report_combo_, (gpointer)report_);
    populating_ = FALSE;

    set_title(_(report_->label));

    if ((fp = open_temp_file()) == 0)
	return;

    report_->func(fp, NULL);
    fflush(fp);
    fseek(fp, 0L, SEEK_SET);

    ui_text_begin(text_);
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
	ui_text_add(text_, 0, buf, n);
    ui_text_end(text_);

    fclose(fp);
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

GLADE_CALLBACK void
reportwin_t::on_report_combo_changed()
{
    const report_t *rep;

    if (populating_ || !shown_)
	return;
    rep = (const report_t *)get_active(report_combo_);
    if (rep != 0)
    {
	/* stupid gtk2 */
	report_ = rep;
	update();
    }
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

GLADE_CALLBACK void
reportwin_t::on_save_as_ok_clicked()
{
    dprintf0(D_UICORE, "reportwin_t::on_save_as_ok_clicked\n");

    const char *filename = gtk_file_selection_get_filename(
		    GTK_FILE_SELECTION(save_dialog_));

    if (filename != 0 && *filename != '\0')
    {
	FILE *fp = fopen(filename, "w");
	if (!fp)
	{
	    perror(filename);
	}
	else
	{
	    report_->func(fp, filename);
	    fclose(fp);
	}
    }

    gtk_widget_hide(save_dialog_);
}

GLADE_CALLBACK void
reportwin_t::on_save_as_cancel_clicked()
{
    dprintf0(D_UICORE, "reportwin_t::on_save_as_cancel_clicked\n");
    gtk_widget_hide(save_dialog_);
}

GLADE_CALLBACK void
reportwin_t::on_save_as_clicked()
{
    dprintf0(D_UICORE, "reportwin_t::on_save_as_clicked\n");
    if (save_dialog_ == 0)
    {
	GladeXML *xml = ui_load_tree("report_save_as");
	save_dialog_ = glade_xml_get_widget(xml, "report_save_as");
	attach(save_dialog_);
    }

    string_var filename = report_->filename;
    if (!filename.data())
	filename = g_strconcat("report_", report_->name, ".txt", (char *)NULL);
    gtk_file_selection_set_filename(
	    GTK_FILE_SELECTION(save_dialog_),
	    filename);

    gtk_widget_show(save_dialog_);
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/*END*/
