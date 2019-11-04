
// libmylcd - http://mylcd.sourceforge.net/
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.
//
//	You should have received a copy of the GNU Library General Public
//	License along with this library; if not, write to the Free
//	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#ifndef _GRAPH_H_
#define _GRAPH_H_

#define GRAPH_AUTOSCALE			0x0001
#define GRAPH_SPECTRUM			0x0002
#define GRAPH_SCOPE				0x0004
#define GRAPH_POLYLINE			0x0008
#define GRAPH_POINTS			0x0010
#define GRAPH_BAR				0x0020

#define GRAPH_HINT_SHADOW1		0x01
#define GRAPH_HINT_SHADOW2		0x02
#define GRAPH_HINT_BORDER0		0x04
#define GRAPH_HINT_BORDER1		0x08
#define GRAPH_HINT_BORDER2		0x10
#define GRAPH_HINT_BASE_FILL	0x20
#define GRAPH_HINT_BASE_BLUR	0x40
#define GRAPH_HINT_SCOPE_FILL	0x80

#define GRAPH_ADDEX_REPLACE		1
#define GRAPH_ADDEX_APPEND		2


enum _palette{
	GRAPH_PAL_SPECTRUM,
	GRAPH_PAL_SCOPE,
	GRAPH_PAL_POLYLINE,
	GRAPH_PAL_POINTS,
	GRAPH_PAL_BAR,
	GRAPH_PAL_SHADOW1,
	GRAPH_PAL_SHADOW2,
	GRAPH_PAL_BASE,			// background fill colour
	GRAPH_PAL_BORDER0,
	GRAPH_PAL_BORDER1,
	GRAPH_PAL_BORDER2
};


typedef struct {
	int *data;
	unsigned int dataLength;		// capacity of array in data points
	unsigned int insertPosition;	// total/add next data entry here
	int canGrow;
	TMLOCK *lock;
}TGRAPHDATASET;

typedef struct {			// per dataset render detail
	int enabled;			// 1:renderable, 0: render disabled
	int mode;				// type: render as bar, spectrum, etc..
	unsigned int hints;		// base, filled, outline, dotted, etc..
	double scale;
	unsigned int palette[16];
	TMETRICS metrics;
}TGRAPHRENDER;

typedef struct {
	int min;
	int max;
	
	// clamps
	int ceiling;		// minimum value for .max
	int floor;			// maximum value for .min
}TGRAPHSTATS;

typedef struct {
	int id;
	char *name;

	TGRAPHDATASET graph;
	TGRAPHRENDER render;
	TGRAPHSTATS stats;		// collect a few usuful statistics
}TGRAPHSHEET;

struct TGRAPH {
	COMMONCTRLOBJECT;

	TGRAPHSHEET **sheets;
	int sheetsTotal;
	int sheetIdSrc;
	
	struct {
		double x;		// percentage of scale
		double y;
	}hoveredPt;
};



int graphNew (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t panel_cb, int *id, const int width, const int height);


int graphNewSheet (TGRAPH *graph, const char *name, int initialSize);		// returns sheet id on success, 0 on failure
int graphSheetAddData (TGRAPHSHEET *sheet, const int data, const int unused);
int graphSheetAddDataEx (TGRAPHSHEET *sheet, const int *data, const int total, const int mode);

int graphSheetDataGetLength (TGRAPHSHEET *sheet);

void graphClear (TGRAPH *graph, const int unused);

TGRAPHSHEET *graphSheetAcquire (TGRAPH *graph, const char *name, const int id);	// acquire by name or id
void graphSheetRelease (TGRAPHSHEET *sheet);

// return number of sheets ready to render
int graphRenderGetSheetCount (void *object);

#endif

