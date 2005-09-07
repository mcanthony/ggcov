/*
 * ggcov - A GTK frontend for exploring gcov coverage data
 * Copyright (c) 2005 Greg Banks <gnb@alphalink.com.au>
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

#include "canvas_scenegen.H"

CVSID("$Id: canvas_scenegen.C,v 1.1 2005-06-13 06:39:39 gnb Exp $");

#define RGB_TO_STR(b, rgb) \
    snprintf((b), sizeof((b)), "#%02x%02x%02x", \
		((rgb)>>16)&0xff, ((rgb)>>8)&0xff, (rgb)&0xff)

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

canvas_scenegen_t::canvas_scenegen_t(GnomeCanvas *can)
{
    canvas_ = can;
    root_ = gnome_canvas_root(canvas_);

    arrow_size_ = 0.5;
    points_.coords = (double *)0;
    points_.num_points = 0;
    points_.ref_count = 1;
    points_size_ = 0;
}

canvas_scenegen_t::~canvas_scenegen_t()
{
    if (points_.coords != 0)
	g_free(points_.coords);
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

void
canvas_scenegen_t::noborder()
{
    border_flag_ = FALSE;
}

void
canvas_scenegen_t::border(unsigned int rgb)
{
    border_flag_ = TRUE;
    RGB_TO_STR(border_buf_, rgb);
}

void
canvas_scenegen_t::nofill()
{
    fill_flag_ = FALSE;
}

void
canvas_scenegen_t::fill(unsigned int rgb)
{
    fill_flag_ = TRUE;
    RGB_TO_STR(fill_buf_, rgb);
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

void
canvas_scenegen_t::box(double x, double y, double w, double h)
{
    GnomeCanvasItem *item;

    item = gnome_canvas_item_new(root_, GNOME_TYPE_CANVAS_RECT,
		"x1", 	    	x,
		"y1",	    	y,
		"x2", 	    	x+w,
		"y2",	    	y+h,
		"fill_color",	fill_color(),
		"outline_color",border_color(),
#if GTK2
		"width_pixels", (border_flag_ ? 1 : 0),
#endif
		(char *)0);
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

void
canvas_scenegen_t::textbox(
    double x,
    double y,
    double w,
    double h,
    const char *text)
{
    GnomeCanvasItem *item;

    item = gnome_canvas_item_new(root_, GNOME_TYPE_CANVAS_TEXT,
		"text",     	text,
		"font",     	"fixed",
		"fill_color",	fill_color(),
		"x", 	    	x,
		"y",	    	y,
		"clip",     	TRUE,
		"clip_width",	w,
		"clip_height",	h,
		"anchor",   	GTK_ANCHOR_NORTH_WEST,
		(char *)0);
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

void
canvas_scenegen_t::arrow_size(double as)
{
    arrow_size_ = as;
}

void
canvas_scenegen_t::polyline_begin(gboolean arrow)
{
    first_arrow_flag_ = arrow;
    points_.num_points = 0;
}

void
canvas_scenegen_t::polyline_point(double x, double y)
{
    unsigned int newsize = 2 * sizeof(double) * (points_.num_points+1);
    if (newsize > points_size_)
    {
	/* round newsize up to a 256-byte boundary to reduce allocations */
	newsize = (newsize + 0xff) & ~0xff;
	/* TODO: use a new gnb_xrealloc */
	points_.coords = (double *)g_realloc(points_.coords, newsize);
	points_size_ = newsize;
    }

    double *p = points_.coords + 2*points_.num_points;
    p[0] = x;
    p[1] = y;
    points_.num_points++;
}

void
canvas_scenegen_t::polyline_end(gboolean arrow)
{
    GnomeCanvasItem *item;

    if (!points_.num_points)
	return;
    item = gnome_canvas_item_new(root_, GNOME_TYPE_CANVAS_LINE,
		"points", 	    	&points_,
		"first_arrowhead",	first_arrow_flag_,
		"last_arrowhead",	arrow,
		"arrow_shape_a",	arrow_size_,
		"arrow_shape_b",	arrow_size_,
		"arrow_shape_c",	arrow_size_/4.0,
		"fill_color",		fill_color(),
		/* setting width_pixels screws up the arrow heads !?!? */
		(char *)0);
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

void
canvas_scenegen_t::file(cov_file_t *)
{
}

void
canvas_scenegen_t::function(cov_function_t *)
{
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/*END*/