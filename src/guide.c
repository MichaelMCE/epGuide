
//  Copyright (c) Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.





#include "common.h"
#include "demos.h"



#define ENABLE_CONIN	0

#define fontProg		"uf/lucida16.uf"
#define fontTime		"uf/lucida16.uf"
#define fontChan		"uf/ubuntu-r22.uf"
#define fontDesc		"uf/lucida20.uf"


#define colCell			(0xEEE7A0)
#define colCellAlt		(0xF5F5B3)
#define colCellSel		(COL_TASKBARBK)
#define colProg			(0xE05000)
#define colTime			(COL_GREEN)
#define colOutline		(COL_GRAY)
#define colTimeStamp	(COL_ORANGE)
#define colTimeMark		(COL_ALPHA(70)|COL_ORANGE)
#define colTimeNow		(COL_ALPHA(255)|COL_BLUE_SEA_TINT)
#define colMargin		(COL_ALPHA(255)|COL_BLUE_SEA_TINT)

#define GUIDE_TIMER_SHORT	(1)
#define GUIDE_TIMER_LONG	(2)
#define GUIDE_TIMER_SINGLE	(3)



static widgets_t ui = {0};

typedef struct{
	union{
		int64_t i64;
		void *ptr;
	}u;
}i64ptr_t;

typedef struct{
	int32_t pid;
	char name[32];
	uint32_t cellId;
	uint32_t tFreq;
}channel_t;


#if 1
static channel_t channels[] = {
	{4221, ""},		// BBC ONE NI
	{4285, ""},		// BBC TWO NI
	{8276, ""},		// UTV
	{8384, ""},		// Channel 4
	{8500, ""},		// Channel 5
	{8325, ""},		// ITV2
	{8294, ""},		// ITV3
	{8330, ""},		// ITV4
	{15856,""},		// ITVBe
	{8385, ""},		// Film4
	{8448, ""},		// E4
	{22272,""},		// Dave
	{27456,""},		// RT
	{27712,""},		// Al Jazeera Eng
	{8442, ""},		// More 4
	{27168, ""},	// 4seven
	{24032, ""},	// Sony Movie Channel
	{23712, ""},	// Really
	{25664, ""},	// 4Music
	{23040, ""},	// Food Network
	{12992, ""},	// 5 USA
	{12928, ""},	// 5STAR
	{14498, ""},	// QUEST
	//{23968, ""},	// True Entertainment	
	{33984, ""},	// True Entertainment	
	//{24016, ""},	// movies4men
	{15576, ""},	// movies4men
	{0, ""}
};
static const int totalChannelsAdded = (sizeof(channels) / sizeof(channel_t)) - 1;
#else
static channel_t channels[142];
#endif


extern int stationTotal;

static int epgEventSig = 0;
static int renderSig = 1;
static int applState = 1;
static time_t timeNow = 0;
static int labelInfoItemId = 0;
static int bottomChannelIdx = 0;
static int fontTimeIdx = 0;
static int stationIdx = 1;
static int selectedCellIdOld = -1;
static int selectedCellId = 0;
static int autoTimer = 0;
static int iconId = 0;
static int guideAutoScanCt = 0;
static int guideTimerActive = 0;







widgets_t *guideGetUi ()
{
	return &ui;
}


int guideGetChannelPid (const uint32_t tFreq, const int chIdx)
{
	int ct = 0;
	
	for (int i = 0; i < totalChannelsAdded; i++){
		channel_t *channel = &channels[i];
		if (channel->tFreq == tFreq){
			if (ct++ == chIdx)
				return channel->pid;
		}
	}
	return 0;
}

int guideGetChannelName (const uint32_t tFreq, int32_t chIdx, char *buffer, int bufferLen)
{
	int ct = 0;
	for (int i = 0; i < totalChannelsAdded; i++){
		channel_t *channel = &channels[i];
		if (channel->tFreq == tFreq){
			if (ct++ == chIdx){
				if (channel->cellId)
					my_snprintf(buffer, bufferLen, "%i", channel->pid);
				else
					strncpy(buffer, channel->name, bufferLen);
				buffer[bufferLen-1] = 0;
				return ct;
			}
		}
	}
	return 0;
}

int guideGetStationIdx ()
{
	return stationIdx;
}

void setApplState ()
{
	applState = 0xFF;
}

void removeApplState ()
{
	applState = 0;
}

int testApplState ()
{
	return applState;
}

void setSigRender ()
{
	if (winuiGetWindowState())
		renderSig = 0xFF;
}

static inline void removeSigRender ()
{
	renderSig = 0;
}

static inline int testSigRender ()
{
	return renderSig;
}

void setSigEpgEvent ()
{
	epgEventSig = 0xFF;
}

static inline void removeSigEpgEvent ()
{
	epgEventSig = 0;
}

static inline int testSigEpgEvent ()
{
	return epgEventSig;
}

static inline void getTimeUpdate ()
{
	time(&timeNow);
}

static inline void setProgrammeInfo (const int cellId, const int action)
{
	//printf("setProgrammeInfo %i\n", cellId);
	
	char *buffer = " ";
	if (action){
		i64ptr_t i64;
		if (!paneCellGetStringData(ui.paneEPG, cellId, 0, &i64.u.i64))
			return;
		buffer = i64.u.ptr;
		if (!buffer) return;
	}
	
	int newlabelInfoItemId = labelTextCreate(ui.labelInfo, buffer, LABEL_WORDWRAP, fontDesc, 0, -1);
	labelStringSetMetricLimit(ui.labelInfo, newlabelInfoItemId, DWIDTH-LEFTMARGIN, DHEIGHT);
	labelRenderColourSet(ui.labelInfo, newlabelInfoItemId, colProg, 0, colOutline);

	// do this so don't try to copy [to clipboard] between a delete and create
	int oldId = labelInfoItemId;
	labelInfoItemId = newlabelInfoItemId;
	labelItemDelete(ui.labelInfo, oldId);
}

void updateProgrammeDetails (const int action)
{
	static uint32_t colourOld = colCell;
	
	paneCellColourSet(ui.paneEPG, selectedCellIdOld, colourOld);
	if (action){
		colourOld = paneCellColourGet(ui.paneEPG, selectedCellId);
		paneCellColourSet(ui.paneEPG, selectedCellId, colCellSel);
		selectedCellIdOld = selectedCellId;
		setProgrammeInfo(selectedCellId, 1);
	}
}

static inline void clearProgrammeDetails ()
{
	updateProgrammeDetails(0);
	setProgrammeInfo(0, 0);
	selectedCellIdOld = 0;
	selectedCellId = 0;
}
			
char *guideGetProgrammeInfoStr ()
{
	return labelStringGet(ui.labelInfo, labelInfoItemId);
}

static inline void doButtonSelect (const int button)
{
	switch (button){
	  case BUTTON_MUTE:
	  	com_sendGlobalHotkey(HK_ALT, HK_SHIFT, 'M');		// mute
	  	break;
	  case BUTTON_QUIT:
		com_sendGlobalHotkey(HK_ALT, HK_SHIFT, 'Q');		// quit
		break;
	  case BUTTON_PLAY:
		com_sendGlobalHotkey(HK_ALT, HK_SHIFT, 'P');		// play
		break;
	  case BUTTON_STOP:
		com_sendGlobalHotkey(HK_ALT, HK_SHIFT, 'O');		// stop
		break;
	};
}

static inline int64_t paneCtrl_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	TPANE *pane = (TPANE*)object;
	//printf("paneCtrl_cb: %i\n", msg);
	
	if (msg == PANE_MSG_IMAGE_SELECTED){
		//printf("paneCtrl_cb: PANE_MSG_IMAGE_SELECTED %i %i\n", (int)data1, (int)data2);
		doButtonSelect(data2);
		setSigRender();
	}else if (msg == PANE_MSG_VALIDATED){
		labelItemEnable(pane->base, iconId);
	}
	return 1;
}

static inline int64_t paneEpg_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	//if (msg == CC_MSG_RENDER || msg == CC_MSG_INPUT || msg == CC_MSG_HOVER) return 1;
	
	//TPANE *pane = (TPANE*)object;
	//printf("paneEpg_cb:  %i\n", msg);
	
	
	if (msg == PANE_MSG_CELL_SELECTED_PRESS){
		selectedCellId = (int)data2;
		if (selectedCellId == selectedCellIdOld){
			selectedCellId = -1;
			setSigRender();
		}
#if 0
	}else if (msg == PANE_MSG_BASE_SELECTED_RELEASE){
		removeSigRender();
		paneInvalidate(ui.paneEPG);
		setSigRender();
#endif
	}else if (msg == PANE_MSG_SLIDE){
		removeSigRender();

		//while(1){
			point_t *delta = (point_t*)dataPtr;
			if (delta->y){
				int y = data2;
				if (y+delta->y > 0){
					delta->y = 0;
					//break;
					return 1;
				}

				int vheight = totalChannelsAdded * ui.layout.cellHeight;
				int nonDisplayableSize = (vheight - ccGetHeight(ui.paneEPG)) - 1;
				if ((y+delta->y)+1 < -nonDisplayableSize){
					delta->y = 0;
					//break;
					return 1;
				}

				int yOffset;
				paneScrollGet(ui.paneChan, NULL, &yOffset);
				int oy = yOffset + delta->y;
				paneScrollSet(ui.paneChan, 0, oy, 1);
			}
			//break;
			return 1;
		//}
	}else if (msg == PANE_MSG_CELL_DELETE){
		//printf("PANE_MSG_CELL_DELETE\n");
		i64ptr_t i64;
		if (paneCellGetStringData(ui.paneEPG, data2, 0, &i64.u.i64)){
			if (i64.u.ptr) my_free(i64.u.ptr);
		}
	}
	
	return 1;
}

static inline void paneCellSendToVLC (TPANE *pane, const int cellId)
{
	int64_t transponderFreq = 0;
	int64_t channelId = 0;
	paneCellGetStringData(pane, cellId, 0, &channelId);
	paneCellGetStringData(pane, cellId, 1, &transponderFreq);
	//printf("--> %i %i\n", (int)transponderFreq, (int)channelId);
	if (transponderFreq && channelId){
		HWND hwnd = com_getVLCIPCHandle();
		if (hwnd)
			com_sendToVLC(hwnd, transponderFreq, channelId);
		else
			com_startVLC(transponderFreq, channelId);
	}
}

static inline int64_t labelInfo_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	return 1;
}

static inline int64_t paneChan_cb (const void *object, const int msg, const int64_t data1, const int64_t data2, void *dataPtr)
{
	//if (msg != CC_MSG_RENDER)
		//printf("paneChan_cb:  %i\n", msg);


	if (msg == PANE_MSG_CELL_SELECTED_RELEASE){
		//printf("PANE_MSG_CELL_SELECTED_RELEASE:  %i\n", (int)data2);
		paneCellSendToVLC((TPANE*)object, (int)data2);		
	//}else if (msg == PANE_MSG_BASE_SELECTED_RELEASE){
		//printf("PANE_MSG_BASE_SELECTED_RELEASE %i\n", selectedCellId);
	
	//}else if (msg == PANE_MSG_CELL_SELECTED_SLIDE){
		//printf("PANE_MSG_CELL_SELECTED_SLIDE:  %i\n", (int)data2);
			
	//}else if (msg == PANE_MSG_CELL_SELECTED_PRESS){
		//printf("PANE_MSG_CELL_SELECTED_PRESS:  %i\n", (int)data2);
	}
	
	return 1;
}

static inline TPANE *createPaneCtrl (TCC *cc)
{

	TPANE *pane = ccCreate(ui.cc, 1, CC_PANE, paneCtrl_cb, NULL, CTRL_WIDTH, CTRL_HEIGHT);

	int x = ((ui.cc->dwidth-1)-CTRL_WIDTH) + 30;
	ccSetPosition(pane, x, (ui.cc->dheight-1) - CTRL_HEIGHT);
	paneSwipeDisable(pane);
	paneDragDisable(pane);
	paneTextMultilineDisable(pane);
	paneTextWordwrapDisable(pane);
	labelRenderFlagsSet(pane->base, LABEL_RENDER_IMAGE|LABEL_RENDER_HOVER_OBJ);
	paneSetLayout(pane, PANE_LAYOUT_HORI);
	ccInputEnable(pane);

	
	const int hoverCol = COL_CYAN;
	int imgId = artManagerImageAdd(ui.im, L"icons/vlc.png");
	iconId = labelImgcCreate(pane->base, imgId, 0, -28, 0);
	labelItemEnable(pane->base, iconId);

	imgId = artManagerImageAdd(ui.im, L"icons/mute.png");
	int itemId = paneImageAdd(pane, imgId, 1.0, PANE_IMAGE_NW, 0, 0, BUTTON_MUTE);
	labelArtcHoverSet(pane->base, itemId, hoverCol, 1.0);

	imgId = artManagerImageAdd(ui.im, L"icons/quit.png");
	itemId = paneImageAdd(pane, imgId, 1.0, PANE_IMAGE_NORTH, 0, 0, BUTTON_QUIT);
	labelArtcHoverSet(pane->base, itemId, hoverCol, 1.0);
	
	imgId = artManagerImageAdd(ui.im, L"icons/play.png");
	itemId = paneImageAdd(pane, imgId, 1.0, PANE_IMAGE_SW, 0, 0, BUTTON_PLAY);
	labelArtcHoverSet(pane->base, itemId, hoverCol, 1.0);
	
	imgId = artManagerImageAdd(ui.im, L"icons/stop.png");
	itemId = paneImageAdd(pane, imgId, 1.0, PANE_IMAGE_SOUTH, 0, 0, BUTTON_STOP);
	labelArtcHoverSet(pane->base, itemId, hoverCol, 1.0);




	return pane;
}

static inline TPANE *createPaneEPG (TCC *cc)
{

	TPANE *pane = ccCreate(ui.cc, 1, CC_PANE, paneEpg_cb, NULL, cc->dwidth-LEFTMARGIN, cc->dheight-(BTMMARGIN+TOPMARGIN));

	ccSetPosition(pane, LEFTMARGIN, TOPMARGIN);
	paneSetAcceleration(pane, 3.0, 1.0);
	paneSwipeEnable(pane);
	paneDragDisable(pane);
	paneTextMultilineDisable(pane);
	paneTextWordwrapDisable(pane);
	labelRenderFlagsSet(pane->base, LABEL_RENDER_TEXT);
	paneSetLayout(pane, PANE_LAYOUT_TABLE);

	return pane;
}

static inline TPANE *createPaneChannelList (TCC *cc)
{

	TPANE *pane = ccCreate(cc, 1, CC_PANE, paneChan_cb, NULL, LEFTMARGIN, cc->dheight - (BTMMARGIN+TOPMARGIN));

	ccSetPosition(pane, 0, TOPMARGIN);
	paneSetAcceleration(pane, 0.0, 0.0);
	paneSwipeDisable(pane);
	paneDragDisable(pane);
	paneTextMultilineDisable(pane);
	paneTextWordwrapDisable(pane);
	labelRenderFlagsSet(pane->base, LABEL_RENDER_TEXT|LABEL_RENDER_HOVER_OBJ2);
	paneSetLayout(pane, PANE_LAYOUT_TABLE);

	return pane;
}

static inline TLABEL *createLabelInfo (TCC *cc)
{
	TLABEL *label = ccCreate(cc, 1, CC_LABEL, labelInfo_cb, NULL, cc->dwidth - LEFTMARGIN, BTMMARGIN);
	labelRenderFlagsSet(label, LABEL_RENDER_CLIP|LABEL_RENDER_TEXT|LABEL_RENDER_BASE);
	label->canDrag = 0;
	labelBaseColourSet(label, COLOUR_24TO16(colCellAlt));
	ccSetPosition(label, 0, (cc->dheight - BTMMARGIN)-1);
	
	int idx = ccFontAdd(cc, fontDesc);
	_ufont_t *hLib = ccFontGetHandle(cc, idx);
	if (hLib)
		fontSetRenderFlags(hLib, fontGetRenderFlags(hLib) | BFONT_RENDER_RETURN|BFONT_RENDER_NEWLINE);

	return label;
}


/*static inline uint64_t generateProgrammeId (TVLCEPGEVENT *prog, const uint64_t channelId)
{
	return (channelId<<47) | ((uint64_t)prog->duration<<32) | ((uint64_t)prog->start>>1);
}*/

static inline channel_t *channelGetChannel (const int channelId)
{
	for (int i = 0; i < totalChannelsAdded/*channels[i].pid*/; i++){
		if (channelId == channels[i].pid)
			return &channels[i];
	}	

	// shouldn't get here..
	for (int i = 0; i < totalChannelsAdded; i++){
		if (!channels[i].pid){
			channels[i].pid = channelId;
			//printf("channels[i].pid %i\n", channels[i].pid);
			return &channels[i];
		}
	}

	return NULL;
}

static inline int calcPaneRow (const int channelId)
{
	for (int i = 0; i < totalChannelsAdded/*channels[i].pid*/; i++){
		if (channelId == channels[i].pid)
			return i;
	}
	return -1;
}

static inline void setChannelNamesDefault ()
{
	char buffer[32];
	
	for (int i = 0; i < totalChannelsAdded/*channels[i].pid*/; i++){
		if (!channels[i].pid) continue;
		my_snprintf(buffer, sizeof(buffer), "%i", channels[i].pid);
		channels[i].cellId = paneCellCreate(ui.paneChan, colCell, i, 0, LEFTMARGIN, channels[i].pid);
		paneCellAddString(ui.paneChan, channels[i].cellId, buffer, fontChan, colProg, LABEL_ALLIGN_CENTER|LABEL_ALLIGN_LEFT, 0, 0, channels[i].pid);
	}
}

static inline void channelSetFreq (channel_t *channel, const uint32_t freq)
{
	channel->tFreq = freq;
}

static inline void channelSetName (channel_t *channel, const char *name)
{
	strncpy(channel->name, name, sizeof(channel->name)-1);
}

int guideChannelAdd (const uint32_t transponderFreq, const int channelId, const char *name)
{
	//printf("addChannel: %i #%s#\n", channelId, name);
	
	channel_t *channel = channelGetChannel(channelId);
	if (!channel) return 0;	// not a channel we want to handle, nor interested in
	
#if 1
	if (!channel->cellId) return -1;	// we've handled this before
	paneCellRemove(ui.paneChan, channel->cellId);
#endif

	channel->cellId = 0;	// indicate that we've been processed
	channelSetName(channel, name);
	channelSetFreq(channel, transponderFreq);

	int row = calcPaneRow(channelId);
	if (row > bottomChannelIdx) bottomChannelIdx = row;
	
	const uint32_t col[2] = {colCell, colCellAlt};
	int cellId = paneCellCreate(ui.paneChan, col[row&0x01], row, 0, LEFTMARGIN, channelId);
	paneCellAddString(ui.paneChan, cellId, channel->name, fontChan, colProg, LABEL_ALLIGN_CENTER|LABEL_ALLIGN_MIDDLE, 0, 0, channelId);
	paneCellSetStringData(ui.paneChan, cellId, 0, channelId);
	paneCellSetStringData(ui.paneChan, cellId, 1, transponderFreq);
	
	return 1;
}

static inline char *buildProgInfo (TVLCEPGEVENT *prog)
{
	//printf("\buildProgInfo:'%s'\n'%s'\n",  prog->name, prog->description);
	
	if (!prog || !prog->description || !prog->name)
		return NULL;
	
	char timestr[32];
	time_t startTime = prog->start;
	struct tm *startTm = localtime(&startTime);
	strftime(timestr, sizeof(timestr)-1, "%H:%M - ", startTm);
		
	time_t endTime = prog->start + prog->duration;
	struct tm *endTm = localtime(&endTime);
	strftime(timestr+strlen(timestr), sizeof(timestr)-strlen(timestr), "%H:%M", endTm);

	if (prog->description)
		com_removeChr(prog->description, '\n');

	int blen = strlen(prog->description) + strlen(prog->name) + sizeof(timestr) + 32;
	char *buffer = my_calloc(sizeof(char), blen);
	if (buffer)
		my_snprintf(buffer, blen, "%s: %s\r\n%s", timestr, prog->name, prog->description);
	return buffer;
	
}

void guideProgrammeAdd (TVLCEPGEVENT *prog, const int channelId)
{
	if (prog->start - timeNow > GUIDEPERIOD)
		return;

	const int row = calcPaneRow(channelId);
	if (row < 0) return;

	const uint64_t pid = generateProgrammeId(prog, channelId);
	TPANEOBJ *obj = paneCellFind(ui.paneEPG, pid, SEARCH_MASK_ALL);
	if (obj) return;

	const uint32_t col[2] = {colCell, colCellAlt};
	int cellId = paneCellCreate(ui.paneEPG, col[row&0x01], row, prog->start, prog->duration, pid);
	if (!cellId){
		abort();
		return;
	}

	char *str = buildProgInfo(prog);
	if (!str) return;
	
	i64ptr_t i64;
	i64.u.ptr = str;
	paneCellAddString(ui.paneEPG, cellId, prog->name, fontProg, colProg, 0, 0, 0, i64.u.i64);

	char timestr[16];
	time_t startTime = prog->start;
	struct tm *tmTime = localtime(&startTime);
	strftime(timestr, sizeof(timestr)-1, "%H:%M", tmTime);
	paneCellAddString(ui.paneEPG, cellId, timestr, fontTime, colTime, 0, 0, 20, pid);
}

#if ENABLE_CONIN
static inline int sleepKb (int sleepFor)
{
	const int decBy = 15;

	do{
		sleepFor -= decBy;
		if (kbhit()) return 1;
		lSleep(decBy);
	}while (sleepFor > 0 && !testSigRender() && !testSigEpgEvent() && testApplState());

	setSigRender();
	return 0;
}
#else
static inline int sleepKb (int sleepFor)
{
	const int decBy = 15;
	
	do{
		sleepFor -= decBy;
		lSleep(decBy);
	}while (sleepFor > 0 && !testSigRender() && !testSigEpgEvent() && testApplState());

	return (sleepFor > 0);
}
#endif


static inline void setEpgLayout (TPANE *pane)
{
	time_t t = 0;
	
	float columnPitch = COLUMNWIDTH;			// pixels per hour (width of columns)
	ui.layout.pixelsPerUnit = Hr / columnPitch;	// n seconds per pixel
	ui.layout.horiMin = (int)time(&t) - TIMEADJUST;	// left side
	ui.layout.horiMax = ui.layout.horiMin+93600;	// right side
	ui.layout.vertMin = 1;							// top
	ui.layout.vertMax = 7;							// bottom
	ui.layout.cellHeight = 42;						// assumes cells expand horizontally from left to right
	ui.layout.cellSpaceHori = 2;					// gap between cells
	ui.layout.cellSpaceVert = 2;
	paneTableSetLayoutMetrics(pane, &ui.layout);
}

static inline void setChanLayout (TPANE *pane)
{
	pane_tablelayout_t layout;
	
	layout.pixelsPerUnit = 1.0f;
	layout.horiMin = 0;
	layout.horiMax = LEFTMARGIN-1;
	layout.vertMin = 1;							// top
	layout.vertMax = 7;							// bottom
	layout.cellHeight = 42;						// assumes cells expand horizontally from left to right
	layout.cellSpaceHori = 0;					// gap between cells
	layout.cellSpaceVert = 2;
	paneTableSetLayoutMetrics(pane, &layout);
}

static inline void scrollPane (TPANE *pane, const int seconds, const int channel)
{
	paneScrollSet(pane, -(seconds/pane->layout.table.pixelsPerUnit),
						  channel*pane->layout.metrics.vertLineHeight,
				1);
}

static inline void scrollPaneTime (TPANE *pane, const int seconds)
{
	int yOffset;
	paneScrollGet(ui.paneEPG, NULL, &yOffset);
	paneScrollSet(pane, -(seconds/pane->layout.table.pixelsPerUnit), yOffset, 1);
}

static inline void scrollPaneChannel (TPANE *pane, const int channel)
{
	int xOffset;
	paneScrollGet(ui.paneEPG, &xOffset, NULL);
	paneScrollSet(pane, xOffset, channel*pane->layout.metrics.vertLineHeight, 1);
}

static inline void closeCCLib ()
{
	//ccFontClose(ui.cc, fontTimeIdx);
	ccDestroy(ui.cc);
}

static inline TCC *initCCLib ()
{
	memset(&ui.cursor, 0, sizeof(ui.cursor));
	
	TCC *cc = ccInit(hw, &ui.cursor);
	cc->strc = strcNew(hw);
	cc->dwidth = DWIDTH;
	cc->dheight = DHEIGHT;
	
	ui.im = artManagerNew(hw);
	ccSetImageManager(cc, CC_IMAGEMANAGER_IMAGE, ui.im);
	
	fontTimeIdx = ccFontAdd(cc, fontChan);
	ccFontOpen(cc, fontTimeIdx);
	ccHoverRenderSigEnable(cc, 40.0);

	return cc;
}


static inline void drawTimeStamps ()
{
	time_t tnow = ui.layout.horiMin;
	int xt = abs(Hr - (tnow%Hr));
	tnow += xt;
	xt += (ui.paneEPG->offset.x * ui.layout.pixelsPerUnit);
	static int xtOld = 0;

	_ufont_t *hLib = ccFontGetHandle(ui.cc, 0);
	if (!hLib) return;
	
	fontSetRenderSurface(hLib, ui.timeSurface);
	fontSetRenderColour(ui.timeSurface, COLOUR_24TO16(colTimeStamp));
	hLib->render.flags &= ~BFONT_RENDER_CLIPFRONT;
	
	if (xtOld != xt){
		xtOld = xt;

		fontCleanSurface(NULL, ui.timeSurface);
		int x = ((xt / ui.layout.pixelsPerUnit) + LEFTMARGIN)-1;
		
		while(x < DWIDTH){
			if (x > LEFTMARGIN-4){
				char timestr[32];
				struct tm *tmTime = localtime(&tnow);
				strftime(timestr, sizeof(timestr)-1, "%H:%M", tmTime);
			
				int width = 0;
				fontGetMetrics(hLib, (uint8_t*)timestr, &width, NULL);
				int _x = (x-(width/2)) - LEFTMARGIN;
				int _y = 0;
				fontPrint(hLib, &_x, &_y, (uint8_t*)timestr);
				
				if (tmTime->tm_hour == 0){
					strftime(timestr, sizeof(timestr)-1, "%a. %b %d", tmTime);
					_x += width/2;
					fontGetMetrics(hLib, (uint8_t*)timestr, &width, NULL);
					_x += (COLUMNWIDTH - width)/2;
					_y = 0;
					fontPrint(hLib, &_x, &_y, (uint8_t*)timestr);
				}
			}

			tnow += Hr;
			x += COLUMNWIDTH;
		}
	}
	const int y = TOPMARGIN - 30;
	fontApplySurface(hLib, LEFTMARGIN, y);
	fontSetRenderColour(ui.timeSurface, COLOUR_24TO16(colOutline));
	fontApplySurfaceOutline(hLib, LEFTMARGIN, y);
}

static inline void drawTimeMeasure ()
{
	time_t tnow = ui.layout.horiMin;
	int xt = (Hr - (tnow%Hr)) + (ui.paneEPG->offset.x * ui.layout.pixelsPerUnit);
	int x = ((xt / ui.layout.pixelsPerUnit) + LEFTMARGIN)-1;
	
	while(x < DWIDTH){
		if (x > LEFTMARGIN-4)
			drawRectangleFilled(frame, x-4, TOPMARGIN-10, x+4, TOPMARGIN-1, colTimeMark);	
		x += COLUMNWIDTH;
	}
}

static inline void drawTimeNowMeasure ()
{
	const time_t tnow = timeNow - ui.layout.horiMin;
	int xt = tnow + (ui.paneEPG->offset.x * ui.layout.pixelsPerUnit);
	int x = ((xt / ui.layout.pixelsPerUnit) + LEFTMARGIN)-1;
	
	if (x > DWIDTH-1) return;
	if (x > LEFTMARGIN-3){
		drawRectangleFilled(frame, x-3, TOPMARGIN-10, x+3, TOPMARGIN-1, colTimeNow);
		drawLineDotted(frame, x, TOPMARGIN, x, (DHEIGHT-BTMMARGIN)-1, colTimeNow);
	}
}

static inline void drawFrame ()
{
	
	// top margin
	drawLine(frame, LEFTMARGIN, TOPMARGIN, DWIDTH-1, TOPMARGIN, colMargin);
	
	// left margin
	drawLine(frame, LEFTMARGIN, TOPMARGIN, LEFTMARGIN, (DHEIGHT-BTMMARGIN)-1, colMargin);
	drawLine(frame, LEFTMARGIN-1, TOPMARGIN, LEFTMARGIN-1, (DHEIGHT-BTMMARGIN)-1, colMargin);

	// bottom  margin
	drawLine(frame, 0/*LEFTMARGIN*/, (DHEIGHT-BTMMARGIN)-1, DWIDTH-1, (DHEIGHT-BTMMARGIN)-1, colMargin);
	
	// bottom right margin
	drawLine(frame, DWIDTH-CTRL_WIDTH, (DHEIGHT-CTRL_HEIGHT), DWIDTH-CTRL_WIDTH, DHEIGHT-1, colMargin);
}

static inline void drawCtrlBase (TFRAME *frame32)
{
	//uint32_t col = COL_ALPHA(50) | COL_ORANGE;
	//drawRectangleFilled(frame32, frame32->width-LEFTMARGIN, frame32->height-BTMMARGIN, frame32->width-1, frame32->height-1, col);menuFillChannellist
}


static inline void renderBoard16 (TFRAME *frame16)
{

	lClearFrame(frame16);
	
	ccRender(ui.paneChan, frame16);
	ccRender(ui.paneEPG, frame16);
	ccRender(ui.labelInfo, frame16);
	drawTimeStamps();
}

static inline void renderBoard32 (TFRAME *frame32)
{
	drawCtrlBase(frame32);
	ccRender(ui.paneCtrl, frame32);
	drawTimeMeasure();
	drawTimeNowMeasure();
	drawFrame();
}

static inline void renderBoard (TFRAME *frame16, TPANE *paneEPG, TPANE *paneChan)
{
	//double t0 = com_getTime(ui.cc);

	renderBoard16(frame16);
	com_frame16ToBuffer32(frame16, frame->pixels);
	renderBoard32(frame);
	
	//double t1 = com_getTime(ui.cc);
	//printf("renderBoard:  %.1fms\n", t1-t0);
}

void guideProviderNext ()
{
	if (++stationIdx >= stationTotal)
		stationIdx = 0;
	
	providerStop();
	lSleep(250);
	providerStart(stationIdx);
}

void guideProviderStop ()
{
	guideTimerStopCycle();
	providerStop();
	winuiMenuUpdate();
	lSleep(250);
}

void guideProviderContinue ()
{
	providerStop();
	lSleep(250);
	providerStart(stationIdx);
}

void guideProviderSet (int idx)
{
	if (idx >= stationTotal)
		idx = 0;
	
	providerStop();
	lSleep(250);
	stationIdx = idx;
	providerStart(stationIdx);
}

void guideCleanGuide ()
{
	paneCellRemoveAll(ui.paneEPG);
	setSigRender();
}

void guideScrollUp ()
{
	int yOffset, xOffset;
	paneScrollGet(ui.paneEPG, &xOffset, &yOffset);
	int oy = yOffset + ui.layout.cellHeight;
		
	//printf("yOffset %i %i %i\n", oy, yOffset, -nonDisplayableSize);
		
	if (oy > 0) oy = 0;
	paneScrollSet(ui.paneEPG, xOffset, oy, 1);
	paneScrollSet(ui.paneChan, 0, oy, 1);
	setSigRender();
}
	
void guideScrollDown ()
{
	int vheight = totalChannelsAdded * ui.layout.cellHeight;
	int nonDisplayableSize = (vheight - ccGetHeight(ui.paneEPG)) - 1;
	int yOffset, xOffset;
	paneScrollGet(ui.paneEPG, &xOffset, &yOffset);
	int oy = yOffset - ui.layout.cellHeight;
		
	//printf("yOffset %i %i %i\n", oy, yOffset, -nonDisplayableSize);
		
	if (oy < -nonDisplayableSize) oy = -nonDisplayableSize;
	paneScrollSet(ui.paneEPG, xOffset, oy, 1);
	paneScrollSet(ui.paneChan, 0, oy, 1);
	setSigRender();
}

void guideScrollTimeNow ()
{
	getTimeUpdate();
	const time_t tnow = timeNow - ui.layout.horiMin;
	int x = tnow - (Hr*1);
	scrollPaneTime(ui.paneEPG, x);
	setSigRender();
}


static inline int guideRemoveChannelListings (int32_t pid)
{
	int64_t pid64 = ((int64_t)pid<<46)&SEARCH_MASK_PID;
	int ct = 0;
	TPANEOBJ *obj;
	
	while((obj=paneCellFind(ui.paneEPG, pid64, SEARCH_MASK_PID))){
		//printf("found %i, %i %i\n", obj->id, obj->cell.item[0].id, obj->cell.item[1].id);
		paneCellRemove(ui.paneEPG, obj->id);
		ct++;
	};
	return ct;
}

int guideRemoveTransponderListings (const int32_t idx)
{
	int ct = 0;
	const uint32_t tFreq = providerGetMrlFreq(idx);
	
	for (int i = 0; i < totalChannelsAdded; i++){
		int32_t pid = guideGetChannelPid(tFreq, i);
		ct += guideRemoveChannelListings(pid);
	}
	return ct;
}


#if ENABLE_CONIN
static inline int doKeyInput ()
{
	printf("doKeyInput...\n");
	const int ch = getch();
	printf("doKeyInput: %i\n", ch);
	
	if (ch == 27 || ch == 13) return 1;
	
	if (ch == ' '){
		pidDump();
	}
	return 0;
}
#endif

static inline void createWidgets (TCC *cc)
{
	ui.paneEPG = createPaneEPG(cc);
	setEpgLayout(ui.paneEPG);
	ccEnable(ui.paneEPG);

	ui.paneChan = createPaneChannelList(cc);
	ccInputDisable(ui.paneChan);
	setChanLayout(ui.paneChan);
	setChannelNamesDefault();
	ccEnable(ui.paneChan);
	
	ui.labelInfo = createLabelInfo(cc);
	ccInputDisable(ui.labelInfo);
	ccEnable(ui.labelInfo);

	ui.paneCtrl = createPaneCtrl(cc);
	ccEnable(ui.paneCtrl);

	ui.timeSurface = fontCreateSurface(ccGetWidth(ui.paneEPG), 28, COLOUR_24TO16(0xFF0000), NULL);
}

void guideTimerStopCycle ()
{
	guideTimerActive = 0;
	if (autoTimer){
		timeKillEvent(autoTimer);
		autoTimer = 0;
	}
}

static void (CALLBACK autoTicker_cb)(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	int *guideCt = (int*)dwUser;
	
	if (!guideTimerActive) return;
	
	++*guideCt;
	//printf("guideAutoScanCt: %i, %i\n", guideTimerActive, *guideCt);
	
	if (guideTimerActive == GUIDE_TIMER_SINGLE){
		guideProviderStop();
		return;
		
	}else if (*guideCt >= stationTotal){
		if (guideTimerActive == GUIDE_TIMER_LONG){
			guideProviderStop();
			return;
		
		}else if (guideTimerActive == GUIDE_TIMER_SHORT){
			guideTimerStopCycle();
			guideTimerStartCycleLong();
		}
	}
	
	guideProviderNext();
	winuiMenuUpdate();
}


static inline void guideTimerStartCycle (const uint32_t period)
{
	//printf("guideTimerStartCycle\n");
	if (autoTimer){
		guideTimerStopCycle();
		sleep(10);			// 10ms
	}
	
	guideAutoScanCt = 0;
	autoTimer = (int)timeSetEvent(period, 100, autoTicker_cb, (DWORD_PTR)&guideAutoScanCt, TIME_PERIODIC|TIME_KILL_SYNCHRONOUS);
}	


void guideTimerStartCycleShort ()
{
	//printf("guideTimerStartCycleShort\n");
	guideTimerActive = GUIDE_TIMER_SHORT;
	guideTimerStartCycle(GUIDETIMER_SHORT * 1000);
}

void guideTimerStartCycleLong ()
{
	//printf("guideTimerStartCycleLong\n");
	guideTimerActive = GUIDE_TIMER_LONG;
	guideTimerStartCycle(GUIDETIMER_LONG * 1000);
}

void guideTimerStartSingle ()
{
	//printf("guideTimerStartCycleLong\n");
	guideTimerActive = GUIDE_TIMER_SINGLE;
	guideTimerStartCycle(GUIDETIMER_SINGLE * 1000);
}

void guideTimerRestartCycle ()
{
	guideTimerStopCycle();
	if (guideTimerActive == GUIDE_TIMER_SHORT)
		guideTimerStartCycleShort();
	else if (guideTimerActive == GUIDE_TIMER_LONG)
		guideTimerStartCycleLong();
}

static inline void CALLBACK updateTimer_cb (UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	//static int fps = 0;
	//static uint32_t t0 = 0;
	
	if (testSigRender()){
		renderBoard(ui.frame16, ui.paneEPG, ui.paneChan);
		lRefresh(frame);
		removeSigRender();
		
#if 0
		fps++;
		if (GetTickCount() - t0 >= 1000){
			printf("render %i %i\n", (int)timeNow - ui.layout.horiMin, fps);
			fps = 0;
			t0 = GetTickCount();
		}
#endif
	}
}


int main (const int argc, const char *argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;

	TFRAME *frame16 = lNewFrame(frame->hw, DWIDTH, DHEIGHT, LFRM_BPP_16);
	ui.frame16 = frame16;
	lClearFrame(frame16);

	ui.cc = initCCLib();
	createWidgets(ui.cc);

	fontSetDisplayBuffer(ccFontGetHandle(ui.cc, 0), frame16->pixels, frame16->width, frame16->height);
	scrollPane(ui.paneEPG, TIMEADJUST-(Hr*1), 0);		// scroll forward by one hour (will move right to left)

	setSigRender();
	setApplState();
	startMouseCapture();

	timeBeginPeriod(25);
	guideTimerStartCycleShort();
	providerStart(stationIdx);
	winuiMenuUpdate();
		

	int updateTimer = (int)timeSetEvent((1.0f/40.0f)*1000, 10, updateTimer_cb, (DWORD_PTR)NULL, TIME_PERIODIC|TIME_KILL_SYNCHRONOUS);
	//int ct = 1;
	uint64_t timeEpgSig = 0;
	
	while (testApplState()){
		//printf("pass %i: %i %i\n", ct++, testSigRender()&0x1, testSigEpgEvent()&0x1);
		
		getTimeUpdate();
		
		if (selectedCellId != selectedCellIdOld){
			if (selectedCellId == -1)
				clearProgrammeDetails();
			else
				updateProgrammeDetails(1);
			setSigRender();
		}
		
		if (testSigEpgEvent()){
			uint64_t t1 = com_getTickCount();
			if (t1 - timeEpgSig > 1000){
				removeSigEpgEvent();
				timeEpgSig = t1;
				if (providerRun())
					setSigRender();
			}
		}

		if (sleepKb(15000)){
#if ENABLE_CONIN
			if (doKeyInput()) break;
#endif
			setSigRender();
		}
	}
	
	timeKillEvent(updateTimer);
	guideTimerStopCycle();
	removeApplState();
	endMouseCapture();
	timeEndPeriod(25);
	
	providerStop();
	lSleep(250);

	fontSurfaceFree(ui.timeSurface);
	lDeleteFrame(frame16);
	
	ccDelete(ui.paneChan);
	ccDelete(ui.paneEPG);
	ccDelete(ui.paneCtrl);
	ccDelete(ui.labelInfo);
	closeCCLib();
	demoCleanup();

	//printf("\n:: Exited\n");

	return EXIT_SUCCESS;
}
