/*
 * ggcov - A GTK frontend for exploring gcov coverage data
 * Copyright (c) 2001-2003 Greg Banks <gnb@alphalink.com.au>
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

#ifndef _ggcov_cov_line_H_
#define _ggcov_cov_line_H_ 1

#include "string_var.H"

class cov_file_t;

class cov_line_t
{
public:

    enum status_t
    {
	COVERED,
	PARTCOVERED,
	UNCOVERED,
	UNINSTRUMENTED
    };
    enum _constants
    {
    	NUM_STATUS=4
    };

    count_t count()
    {
	if (!count_valid_)
    	    calculate_count();
	return count_;
    }

    status_t status()
    {
	if (!count_valid_)
    	    calculate_count();
	return status_;
    }

    const GList *blocks() const
    {
    	return blocks_;
    }
    
    static cov_line_t *find(const cov_location_t *);
    
    /*
     * Format a string compactly describing the list of blocks
     * on a particular line.  Used in the Source window and
     * for tggcov.  Width should be at least 8.
     */
    unsigned int format_blocks(char *buf, unsigned int maxlen);

private:
    cov_line_t();
    ~cov_line_t();
    void calculate_count();

    GList *blocks_;
    gboolean count_valid_;  /* covers status_, count_ */
    status_t status_;
    count_t count_;
    
    friend class cov_file_t;
};


#endif /* _ggcov_cov_line_H_ */