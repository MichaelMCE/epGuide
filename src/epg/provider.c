
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






#include "../common.h"
#include "vlcheaders.h"
#include "vlc.h"
#include "provider.h"


typedef struct {
	TVLCEPG **epg;
	int etotal;	
	int stationIdx;
}guide_t;


typedef struct {
	uint32_t freq;
	char mrl[VLC_MRLLEN];
	void *udata;
}station_t;

static station_t station[] = {
	{522000000},			// bbc
	{474200000},
	{490000000},
	{514000000},
	//{538000000},
	{546000000},
	{594000000},
	{0}
};


extern HANDLE hMsgWin;

const int stationTotal = (sizeof(station) / sizeof(*station))-1;
static uint32_t stationActiveIdx = -1;
static int providerState = 0;
static TVLCCONFIG vlc = {0};
TLISTITEM otherPids;		// unused but available channels





/*void pidDump ()
{
	printf("\n");
	
	TLISTITEM *item = &listAllPids;
	while(item){
		chpid_t *ch = listGetStorage(item);
		if (ch){
			printf("%.3i: %s\n", ch->pid, ch->name);
		}
		item = listGetNext(item);
	}
	
	printf("\n");
}*/


static inline int pidSearch (TLISTITEM *list, const int pid)
{
	while(list){
		chpid_t *ch = listGetStorage(list);
		if (ch && ch->pid == pid)
			return 1;
		list = listGetNext(list);
	}
	return 0;
}

static inline chpid_t *pidAdd (TLISTITEM *list, const uint32_t freqIdx, const uint32_t pid, const char *name)
{
	if (pidSearch(list, pid))
		return NULL;

	chpid_t *ch = my_malloc(sizeof(chpid_t));
	if (!ch) return NULL;

	ch->freqIdx = freqIdx;
	ch->pid = pid;
	strncpy(ch->name, name, CPIDNAMELEN);
	
	TLISTITEM *item = listNewItem(ch);
	list = listGetLast(list);
	if (item)
		listInsert(list, NULL, item);

	return ch;
}

int providerGetState ()
{
	return providerState;
}

void *providerGetData (const int idx)
{
	return station[idx].udata;
}

void providerSetData (const int idx, void *udata)
{
	station[idx].udata = udata;
}

uint32_t providerGetMrlFreq (const int idx)
{
	return station[idx].freq;
}

const char *providerGetMrl (const int idx)
{
	if (idx < 0 || idx >= stationTotal)	
		return NULL;
	
	if (station[idx].freq){
		if (!*station[idx].mrl)
			my_snprintf(station[idx].mrl, sizeof(station[idx].mrl), VLC_MRL"%u", station[idx].freq);
			
		return station[idx].mrl;
	}else{
		return NULL;
	}
}

static inline const station_t *providerGetStation (const int idx)
{
	if (idx < 0 || idx >= stationTotal) return NULL;
	return &station[idx];
}

#if 0
static inline TVLCEPGEVENT *providerFindEvent (guide_t *guide, const int mrlIdx, const uint64_t pid)
{
	TVLCEPG **epg = guide[mrlIdx].epg;
	int etotal = guide[mrlIdx].etotal;
	
	
	for (int i = 0; i < etotal; i++){
		TVLCEPG *channel = epg[i];
		if (!channel || !channel->pp_event) continue;
		//printf("channel: %i '%s', %p\n", channel->programme, channel->psz_name, (*epg)[i]->p_current);

		for (int j = 0; j < channel->i_event; j++){
			TVLCEPGEVENT *programme = channel->pp_event[j];
			if (programme){
				if (pid == generateProgrammeId(programme, channel->programme))
					return programme;
			}
		}
	}
	return NULL;
}

TVLCEPGEVENT *providerFindPid (guide_t *guide, const int mrlIdx, const uint64_t pid)
{
	for (int i = 0; station[i].freq; i++){
		TVLCEPGEVENT *event = providerFindEvent(guide, i, pid);
		if (event) return event;
	}
	return NULL;
}
#endif

static inline int providerGuideDup (guide_t *guide)
{
	guide->epg = epg_dupEpg(&vlc, &guide->etotal);
	
	if (!guide->etotal || !guide->epg){
		//printf("testEpg(): epg_dupEpg returned NULL\n");
		return 0;
	}
	
	return guide->etotal;
}

static inline void providerGuideFree (guide_t *guide)
{
	epg_freeEpg(guide->epg, guide->etotal);
	guide->epg = NULL;
	guide->etotal = 0;
}


static inline void providerGuideChannelsExtractMeta (guide_t *guide)
{
	// extract and remove programme ID from title string
	for (int i = 0; i < guide->etotal; i++){
		if (guide->epg[i]->psz_name){
			char *bracket = strrchr(guide->epg[i]->psz_name, ' ');
			if (bracket){
				guide->epg[i]->programme = (int)strtol(bracket, NULL, 10);
				if (guide->epg[i]->programme < 0) guide->epg[i]->programme = 0;

				char *bracket = strrchr(guide->epg[i]->psz_name, '[');
				if (bracket) *(bracket-1) = 0;
				
			}
		}
	}
}

static inline void providerGuideChannelsDispatch (guide_t *guide)
{
	const station_t *station = providerGetStation(guide->stationIdx);

	for (int i = 0; i < guide->etotal; i++){
		if (!guideChannelAdd(station->freq, guide->epg[i]->programme, guide->epg[i]->psz_name)){
			chpid_t *ch = pidAdd(&otherPids, guide->stationIdx, guide->epg[i]->programme, guide->epg[i]->psz_name);
			if (ch) SendMessage(hMsgWin, WM_ADDCH, 0, (LPARAM)ch);
		}
	}
}
/*
static inline void providerGuideChannelsDump (guide_t *guide)
{
	const station_t *station = providerGetStation(guide->stationIdx);

	printf("\ntransponderFreq: %u\n", station->freq);
	for (int i = 0; i < guide->etotal; i++)
		printf("  %.2i: %i %s\n", i, guide->epg[i]->programme, guide->epg[i]->psz_name);

	printf("\n");
}
*/
static inline void providerGuideProgrammesDispatch (guide_t *guide)
{
	for (int i = 0; i < guide->etotal; i++){
		TVLCEPG *channel = guide->epg[i];
		if (!channel || !channel->pp_event) continue;
		//printf("channel: %i '%s', %p\n", channel->programme, channel->psz_name, (*epg)[i]->p_current);

		for (int j = 0; j < channel->i_event; j++){
			TVLCEPGEVENT *programme = channel->pp_event[j];
			if (programme)
				guideProgrammeAdd(programme, channel->programme);
		}
	}
}

int providerRun ()
{
	if (!providerState) return 0;
	
	guide_t guide = {0};
	guide.stationIdx = stationActiveIdx;	
	
	if (!providerGetStation(guide.stationIdx))
		return 0;

	if (providerGuideDup(&guide)){
		providerGuideChannelsExtractMeta(&guide);
		providerGuideChannelsDispatch(&guide);
		providerGuideProgrammesDispatch(&guide);
		providerGuideFree(&guide);
	}
	
	return guide.etotal;
}

void vlc_eventsCallback (const libvlc_event_t *event, void *udata)
{
	// stub, needed for vlc.c::input_event_changed()
}

void providerStop ()
{
	//printf("providerStop\n");
	
	
	vlc_detachEvents(&vlc, vlc_eventsCallback, NULL);
	vlc_inputEventCbDel(&vlc, NULL);
	vlc_stop(&vlc);
	vlc_mp_release(&vlc);
	vlc_mediaRelease(&vlc);
	vlc_releaseLib(&vlc);
	memset(&vlc, 0, sizeof(vlc));
	/*vlc.m = NULL;
	vlc.mp = NULL;
	vlc.em = NULL;
	vlc.emp = NULL;*/
	
	providerState = 0;
}

/*extern THWD *hw;
extern int virtualDisplayId;*/

int providerStart (const int mrlIdx)
{
	//printf("\nStarting: %i, %s\n", mrlIdx, station[mrlIdx].mrl);
	
	if (providerState) return -1;

	char *vlcConfigOpts[] = {
	/*"--width=152",
	"--height=72",
	"--embedded-video",
	"--no-video-on-top",
	"--no-overlay",*/
	"--no-audio",
	"--no-video",
	//"--vout=usbd480,"
	"--ignore-config",
	"--verbose=0",
	"--ts-silent",
	"--quiet"
	};	

	/*HWND hwnd = 0;
	lGetDisplayOption(hw, virtualDisplayId, lOPT_DDRAW_GETHWND, (intptr_t*)&hwnd);*/


	//if (!vlc.hLib)
		vlc.hLib = libvlc_new(sizeof(vlcConfigOpts)/sizeof(*vlcConfigOpts), (const char**)&vlcConfigOpts);
	if (vlc.hLib){
		vlc.m = vlc_new_mrl(&vlc, providerGetMrl(mrlIdx));
		if (vlc.m){
			vlc.mp = vlc_newFromMedia(&vlc);
			if (vlc.mp){
				/*libvlc_media_player_set_hwnd(vlc.mp, hwnd);
				libvlc_video_set_format(vlc.mp, "RV32", 152, 72, 152*4);
				libvlc_media_player_set_rate(vlc.mp,  25);*/
				vlc_attachEvents(&vlc, vlc_eventsCallback, NULL);
				vlc_play(&vlc);
				vlc_inputEventCbSet(&vlc, NULL);
				stationActiveIdx = mrlIdx;
				providerState = 1;
				return 1;
			}
		}
	}
	providerStop();
	stationActiveIdx = -1;
	return 0;
}
