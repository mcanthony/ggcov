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

#ifndef _ggcov_cov_file_H_
#define _ggcov_cov_file_H_ 1

#include "common.h"

class cov_function_t;

class cov_file_t
{
public:
    static cov_file_t *find(const char *name);

    const char *
    name() const
    {
    	return name_;
    }

    /*
     * cov_file_t maintain a notion of the common prefix of
     * the names of all instances, and defines a minimal name
     * as the part after the common prefix.  The common part
     * is private, but various operations which use it are
     * defined here.
     *
     * To minimise GUI clutter usually only the minimal name
     * is displayed to the user.
     *
     * For a project containing source files in only one
     * directory, the minimal name is the basename.  Minimal
     * names are calculated on a path component basis, not
     * a character basis, so the common path for "/home/foobar"
     * and "/home/foobaz" is "/home/".
     */
    const char *minimal_name() const;
    /* returns a new string */
    static char *minimise_name(const char *name);
    /* returns a new string */
    static char *unminimise_name(const char *name);

    /* Iterate over all file instances.  TODO: real Iterator pattern */
    static void foreach(void (*func)(cov_file_t*, void *userdata), void *userdata);

    /* For iterating over a file's functions */
    unsigned int
    num_functions() const
    {
    	return functions_->len;
    }
    cov_function_t *
    nth_function(unsigned int n) const
    {
    	return (cov_function_t *)functions_->pdata[n];
    }
    cov_function_t *find_function(const char *name) const;

    const cov_location_t *get_last_location() const;

    void calc_stats(cov_stats_t *stats) const;

private:
    cov_file_t(const char *name);
    ~cov_file_t();
    
    static void init();
    static void add_name(const char *name);
    cov_function_t *add_function();

    gboolean read_bb_file(const char *filename);
    gboolean read_bbg_function(FILE *fp);
    gboolean read_bbg_file(const char *filename);
    gboolean read_da_file(const char *filename);
    gboolean read_o_file_relocs(const char *filename);
    gboolean read_o_file(const char *filename);
    gboolean solve();

    static GHashTable *files_;
    static char *common_path_;
    static int common_len_;

    char *name_;
    GPtrArray *functions_;
    GHashTable *functions_by_name_;

    friend gboolean cov_read_source_file_2(const char *fname, gboolean quiet);
    friend void cov_init(void);
    friend void cov_post_read(void);
    friend class cov_function_t;
};


#endif /* _ggcov_cov_file_H_ */