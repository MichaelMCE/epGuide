
#ifndef _GUIDE_H_
#define _GUIDE_H_



#define Sec					(1) 
#define Min					(Sec*60)
#define Hr   				(Min*60)
#define Day   				(Hr*24)
#define Week				(Day*7)
#define Year				(Week*52)

#define GUIDEPERIOD			(Day*4)
#define TIMEADJUST			(Hr*4)		// initial left-side scroll history (seconds)
#define GUIDETIMER_SHORT	(Sec*10)	// quick cycle
#define GUIDETIMER_LONG		(Min*3)	// extended cycle
#define GUIDETIMER_SINGLE	(Min*3)	// One shot, non continuing, extended cycle
#define COLUMNWIDTH			(300)		// width of one hour (pixels)
#define LEFTMARGIN			(152)
#define TOPMARGIN			(32)
#define BTMMARGIN			(72)

#define CTRL_WIDTH		(LEFTMARGIN)
#define CTRL_HEIGHT		(BTMMARGIN)


enum _buttons{
	BUTTON_MUTE = 1001,
	BUTTON_QUIT,
	BUTTON_PLAY,
	BUTTON_STOP,
	
	BUTTON_TOTAL
};



struct widgets_t{
	TCC *cc;
	TARTMANAGER *im;
	TGUIINPUT cursor;
	pane_tablelayout_t layout;
	
	TPANE *paneEPG;
	TPANE *paneChan;
	TPANE *paneCtrl;
	TLABEL *labelInfo;
	TFRAME *frame16;
	_ufont_surface_t *timeSurface;
};



void setApplState ();
void removeApplState ();
int testApplState ();

void setSigEpgEvent ();
void setSigRender ();

void guideProviderNext ();
void guideProviderSet (int idx);
void guideProviderContinue ();
void guideProviderStop ();
void guideCleanGuide ();
int guideGetStationIdx ();
void guideScrollUp ();
void guideScrollDown ();
void guideScrollTimeNow ();
char *guideGetProgrammeInfoStr ();
int guideGetChannelName (const uint32_t tFreq, int32_t chIdx, char *buffer, int bufferLen);
int guideGetChannelPid (const uint32_t tFreq, const int chIdx);
int guideRemoveTransponderListings (const int32_t idx);
widgets_t *guideGetUi ();


void guideTimerStartCycleShort ();
void guideTimerStartCycleLong ();
void guideTimerStartSingle ();
void guideTimerRestartCycle ();
void guideTimerStopCycle ();


void setSigEpgEvent ();

int guideChannelAdd (const uint32_t transponderFreq, const int channelId, const char *name);
void guideProgrammeAdd (TVLCEPGEVENT *prog, const int channelId);


#endif

