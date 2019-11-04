
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



#ifndef _VLC__H_
#define _VLC__H_



typedef struct{
	libvlc_instance_t *hLib;
	libvlc_media_t *m;
	libvlc_media_player_t *mp;
	libvlc_event_manager_t *emp; // media play[er/ing] events
	libvlc_event_manager_t *em;	// media events
}TVLCCONFIG;


typedef struct {
    int64_t start;    /* Interpreted as a value returned by time() */
    int     duration;    /* Duration of the event in seconds */
    char    *name;
    char    *description;
    char    *detail;
}TVLCEPGEVENT;

typedef struct {
    char          *psz_name;
    TVLCEPGEVENT  *p_current;
    int           i_event;
    TVLCEPGEVENT  **pp_event;
    int           programme;	// pid
}TVLCEPG;

typedef struct{
    char *name;
    int64_t timeOffset;
}TTITLESEEKPOINT;

typedef struct{
	char *name;
	int64_t length;				// title time length
	int tSeekPoints;
	TTITLESEEKPOINT *seekPoints;
}TTITLE;

typedef struct {
    char *name;					/**< Name of this info */
    char *value;				/**< Value of the info */
}TCATEGORYINFO;

typedef struct {
    char   *name;				/**< Name of this category */
    int    tInfos;       		/**< Number of infos in the category */
    TCATEGORYINFO *infos;		/**< Pointer to infos */
}TCATEGORY;

typedef struct {
	char *name;
	int id;
}TAVTRACK;

typedef struct {
	TAVTRACK *track;
	int totalTracks;
}TAVTRACKS;

typedef struct{
	char *name;
	char *value;
}vlc_meta_str_t;

typedef struct{
	vlc_meta_str_t **meta;
	int total;
}vlc_meta_extra_t;


# define  libvlcTrackDescriptionRelease libvlc_track_description_list_release



#ifndef libvlc_get_input_thread
input_thread_t *libvlc_get_input_thread (libvlc_media_player_t *p_mi);
#endif



const unsigned char *vlc_getVersion ();
const unsigned char *vlc_getCompiler ();
int vlc_getVolume (TVLCCONFIG *vlc);
void vlc_setMute (TVLCCONFIG *vlc, const int status);
int vlc_getMute (TVLCCONFIG *vlc);
void vlc_pause (TVLCCONFIG *vlc);
void vlc_play (TVLCCONFIG *vlc);
void vlc_stop (TVLCCONFIG *vlc);
void vlc_setPosition (TVLCCONFIG *vlc, float position);
float vlc_getPosition (TVLCCONFIG *vlc);
int vlc_getChapter (TVLCCONFIG *vlc);
void vlc_setChapter (TVLCCONFIG *vlc, int chapter);
void vlc_setVolume (TVLCCONFIG *vlc, int volume);
int vlc_getState (TVLCCONFIG *vlc);
libvlc_time_t vlc_getLength (TVLCCONFIG *vlc);
libvlc_media_t *vlc_new_mrl (TVLCCONFIG *vlc, const char *mediaPath);
libvlc_media_t *vlc_new_path (TVLCCONFIG *vlc, const char *mediaPath);
libvlc_media_t *vlc_new_node (TVLCCONFIG *vlc, const char *mediaPath);
libvlc_media_player_t *vlc_newFromMedia (TVLCCONFIG *vlc);
void vlc_mp_release (TVLCCONFIG *vlc);
void vlc_mediaRelease (TVLCCONFIG *vlc);
int vlc_canPause (TVLCCONFIG *vlc);
int vlc_isSeekable (TVLCCONFIG *vlc);
int vlc_getChapterCount (TVLCCONFIG *vlc);
libvlc_instance_t *vlc_init (TVLCCONFIG *vlc, int argc, char **argv);
void vlc_releaseLib (TVLCCONFIG *vlc);

char *vlc_EventTypeToName (int event);

libvlc_track_description_t *vlc_getSubtitleDescriptions (TVLCCONFIG *vlc);
int vlc_getSubtitleCount (TVLCCONFIG *vlc);
int vlc_setSubtitle (TVLCCONFIG *vlc, int idx);
int vlc_getSubtitle (TVLCCONFIG *vlc);
int vlc_setSubtitleDelay (TVLCCONFIG *vlc, int delay);
int vlc_getSubtitleDelay (TVLCCONFIG *vlc);

void vlc_previousChapter (TVLCCONFIG *vlc);
void vlc_nextChapter (TVLCCONFIG *vlc);
int vlc_willPlay (TVLCCONFIG *vlc);

libvlc_event_manager_t *vlc_getMediaPlayerEventManager (TVLCCONFIG *vlc);
libvlc_event_manager_t *vlc_getMediaEventManager (TVLCCONFIG *vlc);
void vlc_eventPlayerAttach (TVLCCONFIG *vlc, libvlc_event_type_t eventType, libvlc_callback_t callBack, void *udata);
void vlc_eventPlayerDetach (TVLCCONFIG *vlc, libvlc_event_type_t eventType, libvlc_callback_t callBack, void *udata);
void vlc_eventMediaAttach (TVLCCONFIG *vlc, libvlc_event_type_t eventType, libvlc_callback_t callBack, void *udata);
void vlc_eventMediaDetach (TVLCCONFIG *vlc, libvlc_event_type_t eventType, libvlc_callback_t callBack, void *udata);
char *vlc_getMeta (TVLCCONFIG *vlc, libvlc_meta_t e_meta);
int vlc_saveMeta (TVLCCONFIG *vlc);
void vlc_setMeta (TVLCCONFIG *vlc, libvlc_meta_t e_meta, const char *metaStr);
int vlc_isParsed (TVLCCONFIG *vlc);
int vlc_getVideoSize (TVLCCONFIG *vlc, int *w, int *h);


char *vlc_getAspectRatio (TVLCCONFIG *vlc);
void vlc_setAspectRatio (TVLCCONFIG *vlc, const char *aspect);

void vlc_setTitle (TVLCCONFIG *vlc, int title);
int vlc_getTitle (TVLCCONFIG *vlc);
int vlc_getTitleChapterCount (TVLCCONFIG *vlc, int title);
int vlc_getTitleCount (TVLCCONFIG *vlc);

libvlc_track_description_t *vlc_getTitleDescriptions (TVLCCONFIG *vlc);
char *vlc_getTitleDescription (TVLCCONFIG *vlc, int titleId);
char *vlc_getChapterDescription (TVLCCONFIG *vlc, int chapterId, int titleId);
libvlc_track_description_t *vlc_getChapterDescriptions (TVLCCONFIG *vlc, int title);

void vlc_setVideoCallbacks (TVLCCONFIG *vlc,
    void *(*lock) (void *data, void **plane),
    void (*unlock) (void *data, void *picture, void *const *plane),
    void (*display) (void *data, void *picture),
    void *data);
    
void vlc_setVideoFormat (TVLCCONFIG *vlc, const char *chroma,
						 unsigned int width, unsigned int height,
						 unsigned int pitch);

void vlc_setMarqueeInt (TVLCCONFIG *vlc, unsigned int option, int val);
void vlc_setMarqueeStr (TVLCCONFIG *vlc, unsigned int option, char *str);

void vlc_addOption (TVLCCONFIG *vlc, const char *ppsz_options);

void vlc_mediaParse (TVLCCONFIG *vlc);
void vlc_mediaParseAsync (TVLCCONFIG *vlc);

int vlc_getAudioTrack (TVLCCONFIG *vlc);
int vlc_getAudioTrackCount (TVLCCONFIG *vlc);
int vlc_setAudioTrack (TVLCCONFIG *vlc, const int id);

int vlc_getVideoTrack (TVLCCONFIG *vlc);
int vlc_setVideoTrack (TVLCCONFIG *vlc, const int id);
int vlc_getVideoTrackCount (TVLCCONFIG *vlc);

void vlc_navigate (TVLCCONFIG *vlc, const int action);

int vlc_getStats (TVLCCONFIG *vlc, libvlc_media_stats_t *stats);

void epg_freeEpg (TVLCEPG **epg, const int total);
TVLCEPGEVENT *epg_dupEvent (vlc_epg_event_t *vlc_epg_event);
TVLCEPGEVENT *epg_getCurrent (TVLCCONFIG *vlc);
TVLCEPG **epg_dupEpg (TVLCCONFIG *vlc, int *tepg);
void epg_freeEPGEvent (TVLCEPGEVENT *epgevent);

TVLCEPGEVENT *epg_getCurrentFromEpg (TVLCEPG **vepg, const int total);
TVLCEPGEVENT *epg_getGetProgramme (TVLCEPG **vepg, const int total, const int chanIdx, const int progIdx);
char *epg_getChannelName (TVLCEPG **vepg, const int total, const int chanIdx);

int epg_displayOSD (libvlc_media_player_t *p_mi, const int timePeriod);

// chapter lock must be held
TTITLE *getTitles (TVLCCONFIG *vlc, int *tTitles);
TCATEGORY *getCategories (TVLCCONFIG *vlc, int *tCats);

int vlc_setProgram (TVLCCONFIG *vlc, const int newProg);
int vlc_getProgram (TVLCCONFIG *vlc);

int vlc_prevTitle (TVLCCONFIG *vlc);
int vlc_nextTitle (TVLCCONFIG *vlc);

TAVTRACKS *getAudioTracks (TVLCCONFIG *vlc, int *currentAudioTrack);
TAVTRACKS *getVideoTracks (TVLCCONFIG *vlc, int *currentVideoTrack);

void vlc_setAdjustInt (TVLCCONFIG *vlc, unsigned int option, const int value);
void vlc_setAdjustFloat (TVLCCONFIG *vlc, unsigned int option, const double value);




/*

unsigned int vlc_equalizerPresetGetTotal ();
const char *vlc_equalizerPresetGetName (const unsigned int index);
unsigned int vlc_equalizerBandsGetCount ();
double vlc_equalizerBandsGetFrequency (const unsigned int index);
libvlc_equalizer_t *vlc_equalizerNew ();
libvlc_equalizer_t *vlc_equalizerNewFromPreset (const unsigned int index);
void vlc_equalizerRelease (libvlc_equalizer_t *eq);
int vlc_equalizerPreampSet (libvlc_equalizer_t *eq, const double preamp);
double vlc_equalizerPreampGet (libvlc_equalizer_t *eq);
int vlc_equalizerAmpSetByIndex (libvlc_equalizer_t *eq, const double amp, const unsigned int band);
double vlc_equalizerAmpGetByIndex (libvlc_equalizer_t *eq, const unsigned int index);
int vlc_equalizerApply (libvlc_media_player_t *mp, libvlc_equalizer_t *eq);
*/

int vlc_setFilter (libvlc_media_player_t *p_mi, const char *filter, const int add, const int setconfig);


unsigned int vlc_cursorSet (libvlc_media_player_t *p_mi, const int x, const int y);
unsigned int vlc_cursorClicked (libvlc_media_player_t *p_mi, const int x, const int y);

void vlc_inputEventCbSet (TVLCCONFIG *vlc, void *opaque);
void vlc_inputEventCbDel (TVLCCONFIG *vlc, void *opaque);


int vlc_metaExtraGetCount (TVLCCONFIG *vlc);
vlc_meta_extra_t *vlc_metaExtraGet (TVLCCONFIG *vlc);
void vlc_metaExtraFree (vlc_meta_extra_t *extra);

void vlc_detachEvents (TVLCCONFIG *vlc, void *func, void *opaque);
void vlc_attachEvents (TVLCCONFIG *vlc, void *func, void *opaque);



#endif

