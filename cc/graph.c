
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


#include "../common.h"



static inline int genSheetId (TGRAPH *graph)
{
	return ++graph->sheetIdSrc;
}

static inline int graphSheetLock (TGRAPHSHEET *sheet)
{
	return lockWait(sheet->graph.lock, INFINITE);
}

static inline void graphSheetUnlock (TGRAPHSHEET *sheet)
{
	lockRelease(sheet->graph.lock);
}

static inline int graphSheetDataGrow (TGRAPHSHEET *sheet, const unsigned int newLength)
{
	if (newLength <= sheet->graph.dataLength) return 0;
	
	int *data = my_realloc(sheet->graph.data, newLength * sizeof(int));
	if (data){
		sheet->graph.data = data;
		
		for (int i = sheet->graph.dataLength; i < newLength; i++)
			sheet->graph.data[i] = 0;

		sheet->graph.dataLength = newLength;
	}
	return data != NULL;
}

static inline void *graphSheetDataCopy (TGRAPHSHEET *sheet, int *nPoints)
{
	*nPoints = sheet->graph.insertPosition;
	void *data = my_malloc(*nPoints * sizeof(int));
	if (data)
		my_memcpy(data, sheet->graph.data, *nPoints * sizeof(int));
	return data;
}

static inline int graphSheetDataGetCapacity (TGRAPHSHEET *sheet)
{
	return sheet->graph.dataLength;
}

int graphSheetDataGetLength (TGRAPHSHEET *sheet)
{
	return sheet->graph.insertPosition;
}

static inline int graphSheetDataGet (TGRAPHSHEET *sheet, const int pointIdx)
{
	//if (pointIdx < sheet->graph.insertPosition)
		return sheet->graph.data[pointIdx];
	//else
	//	return -1;
}
		
static inline void graphSheetClear (TGRAPHSHEET *sheet)
{
	//memset(sheet->graph.data, 0, sheet->graph.insertPosition*sizeof(int));
	//memset(&sheet->stats, 0, sizeof(TGRAPHSTATS));
	if (sheet->render.mode&GRAPH_AUTOSCALE){
		sheet->stats.min = 9999999;
		sheet->stats.max = 0;
	}
	sheet->graph.insertPosition = 0;
}

static inline int graphSheetDataAdd (TGRAPHSHEET *sheet, const int data, const int unused)
{
	if (sheet->graph.canGrow){
		if (sheet->graph.insertPosition >= sheet->graph.dataLength){
			if (!graphSheetDataGrow(sheet, sheet->graph.dataLength+512))
				return 0;
		}
	}else{
		if (sheet->graph.insertPosition >= sheet->graph.dataLength){
			for (int i = 0; i <  sheet->graph.dataLength-1; i++)
				sheet->graph.data[i] = sheet->graph.data[i+1];
			sheet->graph.insertPosition = sheet->graph.dataLength-1;
		}
	}

	sheet->graph.data[sheet->graph.insertPosition++] = data;	
	
	if (data > sheet->stats.max) sheet->stats.max = data;
	if (data < sheet->stats.min) sheet->stats.min = data;

	return sheet->graph.insertPosition;
}

static inline int graphSheetDataAddEx (TGRAPHSHEET *sheet, const int *data, const int total, const int mode)
{
	int newLength = -1;
	
	if (mode == GRAPH_ADDEX_REPLACE){		// replace current dataset with this
		graphSheetClear(sheet);
		if (graphSheetDataGetCapacity(sheet) < total)
			graphSheetDataGrow(sheet, total+128);

		for (int i = 0; i < total; i++)
			newLength = graphSheetDataAdd(sheet, data[i], 0);
			
	}else if (mode == GRAPH_ADDEX_APPEND){	// append this to current dataset
		for (int i = 0; i < total; i++)
			newLength = graphSheetDataAdd(sheet, data[i], 0);
	}
	return newLength;
}

static inline int graphSheetGetSize (TGRAPHSHEET *sheet)
{
	return sheet->graph.insertPosition;
}

static inline void ldrawLine (TFRAME *restrict frame, const int x1, const int y1, const int x2, const int y2, const int colour)
{
	lDrawLine(frame, x1, y1, x2, y2, colour);
}

static inline void graphSheetRender_Scope (TGRAPHSHEET *sheet, TFRAME *frame, TMETRICS *metrics)
{
	const int total = sheet->graph.insertPosition;
	if (total < 2) return;

	//lDrawRectangle(frame, metrics->x, metrics->y, metrics->x+metrics->width-1, metrics->y+metrics->height-1, 0xFFFF00FF);
		
	double min = sheet->stats.min;
		
	if (sheet->render.mode&GRAPH_AUTOSCALE){
		double range;
		
		if (!sheet->stats.ceiling){
			range = fabs(sheet->stats.max - sheet->stats.min)+1.0;
		}else{
			double max = MAX(sheet->stats.ceiling, sheet->stats.max);
			//min = MAX(sheet->stats.floor, sheet->stats.min);
			range = fabs(max - min)+1.0;
		}
		sheet->render.scale = (metrics->height / range) * 0.98;
		//printf("%f %i %f, %i %i\n", sheet->render.scale, metrics->height, range, sheet->stats.max, sheet->stats.min);
	}
	
	const double scaleX = metrics->width / (double)(total-1);
	const double scaleY = sheet->render.scale;
	double x = metrics->x;


	const unsigned int col = sheet->render.palette[GRAPH_PAL_SCOPE];
	if (sheet->render.hints&GRAPH_HINT_SCOPE_FILL){
		const int btmY = metrics->y + metrics->height-1;
		int lastX = -999;
		//x = metrics->x;
		
		//printf("scaleX %f, %i\n", scaleX, total);

#if 1
		for (int x = metrics->x; x < metrics->x + metrics->width; x++){
			if ((int)x != lastX){
				int i = (1.0/scaleX) * (double)x;
				int y1 = metrics->y + (metrics->height - ((graphSheetDataGet(sheet,i) - /*sheet->stats.*/min) * scaleY));
				ldrawLine(frame, x, y1, x, btmY, col);
				lastX = x;
			}
		}
#else
		for (int i = 0; i < total; i++, x += scaleX){
			if ((int)x != lastX){
				int y1 = metrics->y + (metrics->height - ((graphSheetDataGet(sheet,i) - sheet->stats.min) * scaleY));
				ldrawLine(frame, x, y1, x, btmY, col);
				lastX = x;
			}
		}
#endif
	}

	if (sheet->render.hints&(GRAPH_HINT_SHADOW1|GRAPH_HINT_SHADOW2)){
		const unsigned int colSdw1 = sheet->render.palette[GRAPH_PAL_SHADOW1];
		const unsigned int colSdw2 = sheet->render.palette[GRAPH_PAL_SHADOW2];		
		
		for (int i = 0; i < total-1; i++, x += scaleX){
			int y1 = metrics->y + (metrics->height - ((graphSheetDataGet(sheet,i) - /*sheet->stats.*/min) * scaleY));
			int y2 = metrics->y + (metrics->height - ((graphSheetDataGet(sheet,i+1) - /*sheet->stats.*/min) * scaleY));

			if (sheet->render.hints&GRAPH_HINT_SHADOW1){
				ldrawLine(frame, x-1.0, y1, x+scaleX-1.0, y2, colSdw1);	// w
				ldrawLine(frame, x+1.0, y1, x+scaleX+1.0, y2, colSdw1);	// e
				ldrawLine(frame, x, y1-1.0, x+scaleX, y2-1.0, colSdw1);	// n
				ldrawLine(frame, x, y1+1.0, x+scaleX, y2+1.0, colSdw1);	// s
			}
			if (sheet->render.hints&GRAPH_HINT_SHADOW2){
				ldrawLine(frame, x-1.0, y1-1, x+scaleX-1.0, y2-1, colSdw2);	// nw
				ldrawLine(frame, x+1.0, y1+1, x+scaleX+1.0, y2+1, colSdw2);	// se
			}
		}
	}
	



	x = metrics->x;

	for (int i = 0; i < total-1; i++, x += scaleX){
		int y1 = metrics->y + (metrics->height - ((graphSheetDataGet(sheet,i) - /*sheet->stats.*/min) * scaleY));
		int y2 = metrics->y + (metrics->height - ((graphSheetDataGet(sheet,i+1) - /*sheet->stats.*/min) * scaleY));
		ldrawLine(frame, x, y1, x+scaleX, y2, col);
	}


}

static inline void graphSheetRender_Points (TGRAPHSHEET *sheet, TFRAME *frame, TMETRICS *metrics)
{
	if (sheet->render.mode&GRAPH_AUTOSCALE){
		double range = abs(sheet->stats.max - sheet->stats.min)+1.0;
		sheet->render.scale = metrics->height / range;
	}
	const int total = sheet->graph.insertPosition;
	
	if (total < metrics->width){
		int x = metrics->x + (metrics->width-1 - total);

		for (int i = 0; i < total; i++, x++){
			int y = metrics->y + (metrics->height - (graphSheetDataGet(sheet, i) * sheet->render.scale));
			lSetPixel(frame, x, y, sheet->render.palette[GRAPH_PAL_POINTS]);
		}
	}else{
		int x = metrics->x;
		
		for (int i = total - metrics->width; i < total; i++, x++){
			int y = metrics->y + (metrics->height - (graphSheetDataGet(sheet, i) * sheet->render.scale));
			lSetPixel(frame, x, y, sheet->render.palette[GRAPH_PAL_POINTS]);
		}
	}
}

static inline void graphSheetRender_Polyline (TGRAPHSHEET *sheet, TFRAME *frame, TMETRICS *metrics)
{
	
	if (sheet->render.mode&GRAPH_AUTOSCALE){
		double range = abs(sheet->stats.max - sheet->stats.min)+1.0;
		sheet->render.scale = metrics->height / range;
	}
		
	const double scaleY = sheet->render.scale;
	int firstPt, firstX;
	const int total = sheet->graph.insertPosition;

	if (total < metrics->width){
		firstPt = 0;
		firstX = metrics->x + (metrics->width-1 - total)/2;
	}else{
		firstPt = total - metrics->width;
		firstX = metrics->x;
	
	}

	//printf("graphSheetRender_Polyline %i %i %i %i\n", firstPt, firstX, total, metrics->width);

	int x = firstX;
	
	if (sheet->render.hints&(GRAPH_HINT_SHADOW1|GRAPH_HINT_SHADOW2)){
		const unsigned int colSdw1 = sheet->render.palette[GRAPH_PAL_SHADOW1];
		const unsigned int colSdw2 = sheet->render.palette[GRAPH_PAL_SHADOW2];
		
		for (int i = firstPt; i < total-1; i++, x++){
			int y1 = metrics->y + (metrics->height - (graphSheetDataGet(sheet, i) * scaleY));
			int y2 = metrics->y + (metrics->height - (graphSheetDataGet(sheet, i+1) * scaleY));

			if (sheet->render.hints&GRAPH_HINT_SHADOW1){
				ldrawLine(frame, x-1, y1, x+1-1, y2, colSdw1);		// left
				ldrawLine(frame, x+1, y1, x+1+1, y2, colSdw1);		// right
				ldrawLine(frame, x, y1-1, x+1, y2-1, colSdw1);		// up
				ldrawLine(frame, x, y1+1, x+1, y2+1, colSdw1);		// down
			}
			if (sheet->render.hints&GRAPH_HINT_SHADOW2){
				ldrawLine(frame, x-1, y1-1, x+1-1, y2-1, colSdw2);	// nw
				ldrawLine(frame, x+1, y1+1, x+1+1, y2+1, colSdw2);	// se
			}
		}
		x = firstX;
	}
	
	for (int i = firstPt; i < total-1; i++, x++){
		int y1 = metrics->y + (metrics->height - (graphSheetDataGet(sheet, i) * scaleY));
		int y2 = metrics->y + (metrics->height - (graphSheetDataGet(sheet, i+1) * scaleY));
		ldrawLine(frame, x, y1, x+1, y2, sheet->render.palette[GRAPH_PAL_POLYLINE]);
	}

	//lDrawRectangle(frame, metrics->x, metrics->y, metrics->x+metrics->width-1, metrics->y+metrics->height-1, 0xFF00FFFF);
}

// display right (most recent data) to left (oldest), but render left to right
static inline void graphSheetRender_Spectrum (TGRAPHSHEET *sheet, TFRAME *frame, TMETRICS *metrics)
{
	
	if (sheet->render.mode&GRAPH_AUTOSCALE){
		double range = abs(sheet->stats.max - sheet->stats.min)+1.0;
		sheet->render.scale = metrics->height / range;
	}
	
	const double scaleY = sheet->render.scale;
	const int y1 = metrics->y + metrics->height-1;
	const int col = sheet->render.palette[GRAPH_PAL_SPECTRUM];
	const int total = sheet->graph.insertPosition;
	int x, startPt;
	
	if (total < metrics->width){
		x = metrics->x + (metrics->width-1 - total)/2;
		startPt = 0;
	}else{
		x =  metrics->x;
		startPt = total - metrics->width;
	}
	
	for (int i = startPt; i < total; i++, x++){
		int y2 = metrics->y + (metrics->height - (graphSheetDataGet(sheet, i) * scaleY));
		ldrawLine(frame, x, y2, x, y1, col);
	}

	
	//lDrawRectangle(frame, metrics->x, metrics->y, metrics->x+metrics->width-1, metrics->y+metrics->height-1, 0xFF00FFFF);
}

static inline void graphSheetRender_Bar (TGRAPHSHEET *sheet, TFRAME *frame, TMETRICS *metrics)
{
	if (sheet->render.mode&GRAPH_AUTOSCALE){
		double range = abs(sheet->stats.max - sheet->stats.min)+1.0;
		sheet->render.scale = metrics->height / range;
	}
	
	const double scaleY = sheet->render.scale;
	const int y1 = metrics->y + metrics->height-1;
	const int col = sheet->render.palette[GRAPH_PAL_BAR];
	const int totalPts = sheet->graph.insertPosition;
	
	int barTotal = 32;		// number of vertical bars
	double barSpace = 4.0;	// gap between bars
	double barWidth = (metrics->width - (barTotal*barSpace)) / (double)barTotal;
	double x;
	int startPt;
	
	if (totalPts < barTotal){
		startPt = 0;
		x = metrics->x + metrics->width - ((barWidth + barSpace) * (double)totalPts);
	}else{
		startPt = totalPts - barTotal;
		x = metrics->x;
	}
	x += barSpace/2.0;
	
	//printf("barWidth %f, x:%f\n", barWidth, x);
	const double barWidthMinus1 = barWidth-1.0;
	
	for (int i = startPt; i < totalPts; i++, x+=barWidth+barSpace){
		int y2 = metrics->y + (metrics->height - (graphSheetDataGet(sheet, i) * scaleY));
		lDrawRectangleFilled(frame, x, y2, x+barWidthMinus1, y1, col);
	}
}

static inline TGRAPHSHEET *graphSheetCreate (const char *name, const int initialSize)
{
	TGRAPHSHEET *sheet = my_calloc(1, sizeof(TGRAPHSHEET));
	if (sheet){
		sheet->name = my_strdup(name);
		sheet->graph.canGrow = 0;
		sheet->graph.lock = lockCreate(__FUNCTION__);
		sheet->graph.dataLength = initialSize;
		
		if (sheet->graph.dataLength)
			sheet->graph.data = my_calloc(sheet->graph.dataLength, sizeof(int));

		sheet->render.enabled = 1;
		sheet->render.mode = GRAPH_AUTOSCALE | GRAPH_SPECTRUM;
		//sheet->render.hints = 
		sheet->render.scale = 1.0;
		sheet->render.palette[GRAPH_PAL_SPECTRUM] = 200<<24 | COL_RED;
		sheet->render.palette[GRAPH_PAL_POLYLINE] = 255<<24 | COL_GREEN;
		sheet->render.palette[GRAPH_PAL_SCOPE] = 255<<24 | COL_BLUE;
		sheet->render.palette[GRAPH_PAL_POINTS] = 255<<24 | COL_WHITE;
		sheet->render.palette[GRAPH_PAL_BAR] = 200<<24 | COL_ORANGE;
		sheet->render.palette[GRAPH_PAL_SHADOW1] = 80<<24 | 0x111111;
		sheet->render.palette[GRAPH_PAL_SHADOW2] = 80<<24 | 0x111111;
		sheet->render.palette[GRAPH_PAL_BASE] = 40<<24 | 0x115111;
		sheet->render.palette[GRAPH_PAL_BORDER0] = 224<<24 | COL_BLUE_SEA_TINT;
		sheet->render.palette[GRAPH_PAL_BORDER1] = 224<<24 | COL_BLUE_SEA_TINT;
		sheet->render.palette[GRAPH_PAL_BORDER2] = 224<<24 | COL_BLUE_SEA_TINT;
		sheet->stats.min = 9999999;
		sheet->stats.max = 0;

	}
	return sheet;
}

static inline void graphSheetDelete (TGRAPHSHEET *sheet)
{
	lockWait(sheet->graph.lock, INFINITE);
	my_free(sheet->graph.data);
	my_free(sheet->name);
	lockClose(sheet->graph.lock);
	my_free(sheet);
}

static inline int graphSheetAdd (TGRAPH *graph, TGRAPHSHEET *sheet)
{
	if (!graph->sheetsTotal)
		graph->sheets = my_malloc(sizeof(TGRAPHSHEET*));
	else
		graph->sheets = my_realloc(graph->sheets, (graph->sheetsTotal+1) * sizeof(TGRAPHSHEET*));
	
	graph->sheets[graph->sheetsTotal++] = sheet;
	return sheet->id = genSheetId(graph);
}

static inline TGRAPHSHEET *graphSheetFromId (TGRAPH *graph, const int id)
{
	for (int i = 0; i < graph->sheetsTotal; i++){
		TGRAPHSHEET *sheet = graph->sheets[i];
		
		if (sheet->id == id){
			//printf("graphSheetFromId idx:%i '%s' %i %i\n", i, sheet->name, sheet->id, id);
			return sheet;
		}
	}
	return NULL;
}

static inline TGRAPHSHEET *graphSheetFromName (TGRAPH *graph, const char *name)
{
	for (int i = 0; i < graph->sheetsTotal; i++){
		TGRAPHSHEET *sheet = graph->sheets[i];
		
		if (!stricmp(sheet->name, name)){
			//printf("graphSheetFromName idx:%i '%s' '%s' %i\n", i, sheet->name, name, sheet->id);
			return sheet;
		}
	}
	return NULL;
}

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

// lock must be held (via graphSheetAcquire) before calling
int graphSheetAddDataEx (TGRAPHSHEET *sheet, const int *data, const int total, const int mode)
{
	if (!mode) return -1;
	return graphSheetDataAddEx(sheet, data, total, mode);
}

// lock must be held before calling
int graphSheetAddData (TGRAPHSHEET *sheet, const int data, const int unused)
{
	return graphSheetDataAdd(sheet, data, unused);
}

TGRAPHSHEET *graphSheetAcquire (TGRAPH *graph, const char *name, const int id)
{
	//if (ccLock(graph)){
		TGRAPHSHEET *sheet = NULL;
		
		if (id) sheet = graphSheetFromId(graph, id);
		if (name && !sheet) sheet = graphSheetFromName(graph, name);

		if (sheet){
			if (graphSheetLock(sheet)){
				//ccUnlock(graph);
				return sheet;
			}
		}
		//ccUnlock(graph);
	//}
	return NULL;
}

void graphSheetRelease (TGRAPHSHEET *sheet)
{
	graphSheetUnlock(sheet);
}

int graphNewSheet (TGRAPH *graph, const char *name, int initialSize)
{
	if (initialSize < 1) initialSize = ccGetWidth(graph);
	
	TGRAPHSHEET *sheet = graphSheetCreate(name, initialSize);
	ccGetMetrics(graph, &sheet->render.metrics);
	
	return graphSheetAdd(graph, sheet);
}

void graphClear (TGRAPH *graph, const int unused)
{
	for (int i = 0; i < graph->sheetsTotal; i++){
		TGRAPHSHEET *sheet = graph->sheets[i];
		if (graphSheetLock(sheet)){
			graphSheetClear(sheet);
			graphSheetUnlock(sheet);
		}
	}
}

/*
############################################################################################################
############################################################################################################
############################################################################################################
############################################################################################################
*/

int64_t graph_cb (const void *object, const int msg, const int64_t dataInt1, const int64_t dataInt2, void *dataPtr)
{
	//TGRAPH *graph = (TGRAPH*)object;
	
	return 0;
}

static inline void updateHovered (TGRAPH *graph)
{
	TMETRICS metrics;
	ccGetMetrics(graph, &metrics);
		
	const double range = 100.0;	// 0 - 100%
	int posX = graph->cc->cursor->dx - metrics.x;
	graph->hoveredPt.x = posX / (double)(metrics.width/range);
	int posY = graph->cc->cursor->dy - metrics.y;
	graph->hoveredPt.y = (metrics.height - posY) / (double)(metrics.height/range);
	//printf("graphRender %i %.3f %.3f\n", posY, graph->hoveredPt.x, graph->hoveredPt.y);
}

int graphRenderGetSheetCount (void *object)
{
	TGRAPH *graph = (TGRAPH*)object;
	int count = 0;

	if (ccLock(graph)){
		for (int i = 0; i < graph->sheetsTotal; i++){
			TGRAPHSHEET *sheet = graph->sheets[i];
			count += sheet->render.enabled;
		}
		ccUnlock(graph);
	}
	return count;
}

int graphRender (void *object, TFRAME *frame)
{
	TGRAPH *graph = (TGRAPH*)object;

	//if (ccIsHovered(graph))
		updateHovered(graph);
	
	//lDrawRectangleFilled(frame, metrics.x, metrics.y, metrics.x+metrics.width-1, metrics.y+metrics.height-1, 50<<24 | 0x111111);
				
	for (int i = 0; i < graph->sheetsTotal; i++){
		TGRAPHSHEET *sheet = graph->sheets[i];
		if (!sheet->render.enabled) continue;
		
		if (graphSheetLock(sheet)){
			TMETRICS *metrics = &sheet->render.metrics;
			
			if (sheet->render.hints&GRAPH_HINT_BASE_FILL)
				lDrawRectangleFilled(frame, metrics->x, metrics->y, metrics->x+metrics->width-1, metrics->y+metrics->height-1, sheet->render.palette[GRAPH_PAL_BASE]);
			if (sheet->render.hints&GRAPH_HINT_BASE_BLUR)
				lBlurArea(frame, metrics->x, metrics->y, metrics->x+metrics->width-1, metrics->y+metrics->height-1, 2);
			if (sheet->render.hints&GRAPH_HINT_BORDER0)
				lDrawRectangle(frame, metrics->x, metrics->y, metrics->x+metrics->width-1, metrics->y+metrics->height-1, sheet->render.palette[GRAPH_PAL_BORDER0]);
			if (sheet->render.hints&GRAPH_HINT_BORDER1)
				lDrawRectangle(frame, metrics->x-1, metrics->y-1, metrics->x+metrics->width, metrics->y+metrics->height, sheet->render.palette[GRAPH_PAL_BORDER1]);
			if (sheet->render.hints&GRAPH_HINT_BORDER2)
				lDrawRectangle(frame, metrics->x-2, metrics->y-2, metrics->x+metrics->width+1, metrics->y+metrics->height+1, sheet->render.palette[GRAPH_PAL_BORDER2]);
				
			if (sheet->render.mode&GRAPH_SPECTRUM)
				graphSheetRender_Spectrum(sheet, frame, metrics);
			if (sheet->render.mode&GRAPH_SCOPE)
				graphSheetRender_Scope(sheet, frame, metrics);
			if (sheet->render.mode&GRAPH_POLYLINE)
				graphSheetRender_Polyline(sheet, frame, metrics);
			if (sheet->render.mode&GRAPH_POINTS)
				graphSheetRender_Points(sheet, frame, metrics);	
			if (sheet->render.mode&GRAPH_BAR)
				graphSheetRender_Bar(sheet, frame, metrics);		
				
			graphSheetUnlock(sheet);
		}
	}

	return 0;
}

void graphEnable (void *object)
{
	TGRAPH *graph = (TGRAPH*)object;
	
	graph->enabled = 1;
	return;
}

void graphDisable (void *object)
{
	TGRAPH *graph = (TGRAPH*)object;
	
	graph->enabled = 0;
	return;
}

int graphInput (void *object, TTOUCHCOORD *pos, const int flags)
{
	TGRAPH *graph = (TGRAPH*)object;
	//printf("graphInput %i %i %X\n", pos->x, pos->y, flags);
	
	updateHovered(graph);
	
	return 0;
}

int graphSetPosition (void *object, const int x, const int y)
{
	TGRAPH *graph = (TGRAPH*)object;
	
	graph->metrics.x = x;
	graph->metrics.y = y;
	
	return 1;
}

int graphSetMetrics (void *object, const int x, const int y, const int width, const int height)
{
	TGRAPH *graph = (TGRAPH*)object;
	
	graph->metrics.width = width;
	graph->metrics.height = height;
	
	return 1;
}

void graphDelete (void *object)
{
	TGRAPH *graph = (TGRAPH*)object;
	
	if (graph->sheetsTotal){
		while (graph->sheetsTotal--)
			graphSheetDelete(graph->sheets[graph->sheetsTotal]);
		my_free(graph->sheets);
	}
}

int graphNew (TCCOBJECT *object, void *unused, const int pageOwner, const int type, const TCommonCrtlCbMsg_t graph_cb, int *id, const int width, const int height)
{
	TGRAPH *graph = (TGRAPH*)object;

	graph->pageOwner = pageOwner;
	if (id) *id = graph->id;
	graph->type = type;

	graph->cb.msg = graph_cb;
	graph->cb.render = graphRender;
	graph->cb.create = graphNew;
	graph->cb.free = graphDelete;
	graph->cb.enable = graphEnable;
	graph->cb.disable = graphDisable;
	graph->cb.input = graphInput;
	graph->cb.setPosition = graphSetPosition;
	graph->cb.setMetrics = graphSetMetrics;

	graph->metrics.width = width;
	graph->metrics.height = height;

	graph->processInput	= 0;
	graph->sheets = NULL;
	graph->sheetsTotal = 0;
	graph->sheetIdSrc = 100;
		
	return 1;
}

