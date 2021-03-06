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

#include "hashtable.H"

CVSID("$Id: hashtable.C,v 1.6 2010-05-09 05:37:15 gnb Exp $");

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

void
gnb_hash_table_add_one_key(gpointer key, gpointer value, gpointer closure)
{
    list_t<void> *list = (list_t<void> *)closure;

    list->prepend(key);
}

static int
string_compare(gconstpointer v1, gconstpointer v2)
{
    return strcmp((const char *)v1, (const char *)v2);
}

static int
direct_compare(gconstpointer v1, gconstpointer v2)
{
    if ((unsigned long)v1 < (unsigned long)v2)
	return -1;
    else if ((unsigned long)v1 > (unsigned long)v2)
	return 1;
    else
	return 0;
}

/* copied from glib's g_str_hash() */
guint
uint64_t_hash (gconstpointer key)
{
    const char *p = (const char *)key;
    unsigned int i;
    guint h = *p;

    for (i = 0 ; i < sizeof(uint64_t) ; i++)
	h = (h << 5) - h + p[i];

    return h;
}

static gboolean
uint64_t_equal(gconstpointer v1, gconstpointer v2)
{
    return *(uint64_t *)v1 == *(uint64_t *)v2;
}

static int
uint64_t_compare(gconstpointer v1, gconstpointer v2)
{
    uint64_t u1 = *(uint64_t *)v1;
    uint64_t u2 = *(uint64_t *)v2;

    if (u1 < u2)
	return -1;
    else if (u1 > u2)
	return 1;
    else
	return 0;
}

template<> GHashFunc hashtable_ops_t<char>::hash = g_str_hash;
template<> GCompareFunc hashtable_ops_t<char>::compare = g_str_equal;
template<> GCompareFunc hashtable_ops_t<char>::sort_compare = string_compare;

template<> GHashFunc hashtable_ops_t<const char>::hash = g_str_hash;
template<> GCompareFunc hashtable_ops_t<const char>::compare = g_str_equal;
template<> GCompareFunc hashtable_ops_t<const char>::sort_compare = string_compare;

template<> GHashFunc hashtable_ops_t<void>::hash = g_direct_hash;
template<> GCompareFunc hashtable_ops_t<void>::compare = g_direct_equal;
template<> GCompareFunc hashtable_ops_t<void>::sort_compare = direct_compare;

template<> GHashFunc hashtable_ops_t<uint64_t>::hash = uint64_t_hash;
template<> GCompareFunc hashtable_ops_t<uint64_t>::compare = uint64_t_equal;
template<> GCompareFunc hashtable_ops_t<uint64_t>::sort_compare = uint64_t_compare;

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/*END*/
