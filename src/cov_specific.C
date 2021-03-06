/*
 * ggcov - A GTK frontend for exploring gcov coverage data
 * Copyright (c) 2003-2005 Greg Banks <gnb@users.sourceforge.net>
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

#include "cov_specific.H"
#include "string_var.H"
#include "filename.h"
#include "demangle.h"

CVSID("$Id: cov_specific.C,v 1.8 2010-05-09 05:37:15 gnb Exp $");


cov_factory_item_t *cov_factory_item_t::all_;

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

cov_filename_scanner_t::cov_filename_scanner_t()
{
    cbfd_ = 0;
}

cov_filename_scanner_t::~cov_filename_scanner_t()
{
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

gboolean
cov_filename_scanner_t::attach(cov_bfd_t *c)
{
    cbfd_ = c;
    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/* TODO: allocate these dynamically */
int
cov_filename_scanner_t::factory_category()
{
    return 1;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

cov_shlib_scanner_t::cov_shlib_scanner_t()
{
    cbfd_ = 0;
}

cov_shlib_scanner_t::~cov_shlib_scanner_t()
{
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

gboolean
cov_shlib_scanner_t::attach(cov_bfd_t *c)
{
    cbfd_ = c;
    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/* TODO: allocate these dynamically */
int
cov_shlib_scanner_t::factory_category()
{
    return 3;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

cov_call_scanner_t::cov_call_scanner_t()
{
}

cov_call_scanner_t::~cov_call_scanner_t()
{
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

gboolean
cov_call_scanner_t::attach(cov_bfd_t *c)
{
    cbfd_ = c;
    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

gboolean
cov_call_scanner_t::setup_calldata(
    cov_bfd_section_t *sec,
    unsigned long address,
    const char *callname,
    cov_call_scanner_t::calldata_t *calld)
{
    string_var callname_dem = demangle(callname);

    calld->callname = (char *)0;    /* free and null out */
    memset(calld, 0, sizeof(*calld));
    if (!sec->find_nearest_line(address, &calld->location, &calld->function))
	return FALSE;

    if (debug_enabled(D_CGRAPH))
    {
	string_var function_dem = demangle(calld->function);
	duprintf4("%s:%ld: %s calls %s\n",
		calld->location.filename,
		calld->location.lineno,
		function_dem.data(),
		callname_dem.data());
    }

    calld->callname = callname_dem.take();
    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/* TODO: allocate these dynamically */
int
cov_call_scanner_t::factory_category()
{
    return 2;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/*END*/
