/*
 * ggcov - A GTK frontend for exploring gcov coverage data
 * Copyright (c) 2001 Greg Banks <gnb@alphalink.com.au>
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

#ifndef _ggcov_fileswin_H_
#define _ggcov_fileswin_H_ 1

#include "window.H"
#include "cov.h"

GLADE_CALLBACK void on_files_close_activate(GtkWidget *w, gpointer data);
GLADE_CALLBACK void on_files_exit_activate(GtkWidget *w, gpointer data);
GLADE_CALLBACK void on_files_lines_check_activate(GtkWidget *w, gpointer data);
GLADE_CALLBACK void on_files_calls_check_activate(GtkWidget *w, gpointer data);
GLADE_CALLBACK void on_files_branches_check_activate(GtkWidget *w, gpointer data);
GLADE_CALLBACK void on_files_percent_check_activate(GtkWidget *w, gpointer data);
GLADE_CALLBACK void on_files_clist_button_press_event(GtkWidget *w, GdkEvent *event, gpointer data);

class fileswin_t : public window_t
{
public:
    fileswin_t();
    ~fileswin_t();

private:
    void populate();
    void update();
    static fileswin_t *from_widget(GtkWidget *w)
    {
    	return (fileswin_t *)window_t::from_widget(w);
    }
    static void add_file(cov_file_t *f, void *userdata);


    GList *files_;	    /* list of file_rec_t */
    
    GtkWidget *lines_check_;
    GtkWidget *calls_check_;
    GtkWidget *branches_check_;
    GtkWidget *percent_check_;
    GtkWidget *clist_;

    friend void on_files_close_activate(GtkWidget *w, gpointer data);
    friend void on_files_exit_activate(GtkWidget *w, gpointer data);
    friend void on_files_lines_check_activate(GtkWidget *w, gpointer data);
    friend void on_files_calls_check_activate(GtkWidget *w, gpointer data);
    friend void on_files_branches_check_activate(GtkWidget *w, gpointer data);
    friend void on_files_percent_check_activate(GtkWidget *w, gpointer data);
    friend void on_files_clist_button_press_event(GtkWidget *w, GdkEvent *event, gpointer data);
};

#endif /* _ggcov_fileswin_H_ */