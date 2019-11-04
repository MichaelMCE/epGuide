            
// vlc.c. libvlc 1.1.0 wrapper
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.


#include "../common.h"

#include "vlcheaders.h"
#include "vlc.h"




static const libvlc_event_type_t m_events[] = {
    libvlc_MediaMetaChanged,
    libvlc_MediaSubItemAdded,
    libvlc_MediaDurationChanged,
    libvlc_MediaParsedChanged,
    //libvlc_MediaFreed,
    libvlc_MediaStateChanged
};
		
static const libvlc_event_type_t mp_events[] = {
    //libvlc_MediaPlayerMediaChanged,
    //libvlc_MediaPlayerNothingSpecial,
    //libvlc_MediaPlayerOpening,
#if (LIBVLC_VERSION_MAJOR >= 2 /*&& LIBVLC_VERSION_MINOR >= 1*/)
    libvlc_MediaPlayerBuffering,
#endif
    libvlc_MediaPlayerPlaying,
    libvlc_MediaPlayerPaused,
    libvlc_MediaPlayerStopped,
    //libvlc_MediaPlayerForward,
    //libvlc_MediaPlayerBackward,
    libvlc_MediaPlayerEndReached,
    libvlc_MediaPlayerEncounteredError,
    //libvlc_MediaPlayerTimeChanged,
    libvlc_MediaPlayerPositionChanged,
    //libvlc_MediaPlayerSeekableChanged,
    //libvlc_MediaPlayerPausableChanged,
    libvlc_MediaPlayerTitleChanged
    //libvlc_MediaPlayerSnapshotTaken,
    //libvlc_MediaPlayerLengthChanged,
    //libvlc_MediaPlayerVout
};


#define vlibvlc_free	libvlc_free
//#define vlibvlc_free	free








/*
static inline void lock_input (libvlc_media_player_t *mp)
{
    vlc_mutex_lock(&mp->input.lock);
}

static inline void unlock_input (libvlc_media_player_t *mp)
{
    vlc_mutex_unlock(&mp->input.lock);
}*/


static inline void vlcMutexLock (void *mutex)
{
	if (mutex)
		vlc_mutex_lock(mutex);
}

static inline void vlcMutexUnlock (void *mutex)
{
	if (mutex)
		vlc_mutex_unlock(mutex);
}

void vlc_detachEvents (TVLCCONFIG *vlc, void *func, void *opaque)
{
	if (vlc->emp){
		for (int i = 0; i < sizeof(mp_events)/sizeof(*mp_events); i++)
			vlc_eventPlayerDetach(vlc, mp_events[i], func/*vlc_eventsCallbackLocked*/, opaque);
		vlc->emp = NULL;
	}

	if (vlc->em){
		for (int i = 0; i < sizeof(m_events)/sizeof(*m_events); i++)
			vlc_eventMediaDetach(vlc, m_events[i], func/*vlc_eventsCallbackLocked*/, opaque);
		vlc->em = NULL;
	}
}

void vlc_attachEvents (TVLCCONFIG *vlc, void *func, void *opaque)
{
	vlc_getMediaPlayerEventManager(vlc);
	for (int i = 0; i < sizeof(mp_events)/sizeof(*mp_events); i++)
		vlc_eventPlayerAttach(vlc, mp_events[i], func/*vlc_eventsCallbackLocked*/, opaque);

	vlc_getMediaEventManager(vlc);
	for (int i = 0; i < sizeof(m_events)/sizeof(*m_events); i++)
		vlc_eventMediaAttach(vlc, m_events[i], func/*vlc_eventsCallbackLocked*/, opaque);
}

static inline vout_thread_t **GetVouts (libvlc_media_player_t *p_mi, size_t *n)
{
    input_thread_t *p_input = libvlc_get_input_thread(p_mi);
    if (!p_input){
        *n = 0;
        return NULL;
    }

	
    vout_thread_t **pp_vouts;
    if (input_Control(p_input, INPUT_GET_VOUTS, &pp_vouts, n)){
        *n = 0;
        pp_vouts = NULL;
    }
    
    vlc_object_release(p_input);
    return pp_vouts;
}

#if 0
int _vout_OSDEpg(vout_thread_t *vout, input_item_t *input, const int timePeriod);

int epg_displayOSD (libvlc_media_player_t *p_mi, const int timePeriod)
{
	if (!p_mi) return 0;

	size_t n;
	vout_thread_t **pp_vouts = GetVouts(p_mi, &n);

	if (!pp_vouts) return -1;
	int ret = 0;
	
	for (size_t i = 0; i < n; i++){
		vout_thread_t *vout = pp_vouts[i];
		if (!vout) continue;

		//printf("epg_displayOSD: vout %p\n", vout);

		input_thread_t *p_input = libvlc_get_input_thread(p_mi);
		if (p_input){
			input_item_t *p_item = input_GetItem(p_input);
			ret = _vout_OSDEpg(vout, p_item, timePeriod) == VLC_SUCCESS;
			vlc_object_release(p_input);
		//}else{
			//printf("epg_displayOSD: no input\n");
		}
		vlc_object_release(vout);
    }
    vlibvlc_free(pp_vouts);
    return ret;
}
#endif

#if 0
int vlc_setFilter (libvlc_media_player_t *p_mi, const char *filter, const int add, const int setconfig)
{
	int ret = 0;
	if (!p_mi) return ret;

	size_t n;
	vout_thread_t **pp_vouts = GetVouts(p_mi, &n);
	if (!pp_vouts) return ret;
	
	for (size_t i = 0; i < n; i++){
		vout_thread_t *vout = pp_vouts[i];
		if (!vout) continue;
		
		//vout_thread_sys_t *p_sys = vout->p;
		//printf("vlc_setFilter, %u: vd:%p, input:%p\n", i, p_sys->display.vd, p_sys->input);

		//vout_EnableFilter(vout, filter, add, setconfig);
		
		//osd_MenuShow(vout);
		//vout_OSDText(vout, 3, 1, 1000000, "position test");
		//vout_OSDSlider(vout, 0, setconfig, OSD_HOR_SLIDER);
		//vout_OSDSlider(vout, 4, setconfig, OSD_VERT_SLIDER);
 		
 		//OSD_PLAY_ICON, OSD_PAUSE_ICON, OSD_SPEAKER_ICON, OSD_MUTE_ICON
		//vout_OSDIcon(vout, 1, OSD_PAUSE_ICON);
		//vout_OSDMessage(vout, 2, "testing 123");

		vlc_object_release(p_input);

		ret += 1;
		vlc_object_release(vout);
    }
    /vlibvlc_free(pp_vouts);
    return 1;
}
#endif

unsigned int vlc_cursorClicked (libvlc_media_player_t *p_mi, const int x, const int y)
{
    size_t n;
    vout_thread_t **pp_vouts = GetVouts(p_mi, &n);
    
    if (!pp_vouts) return 0;
    
    for (size_t i = 0; i < n; i++){
    	vout_thread_t *vout = pp_vouts[i];
    	if (!vout) continue;
        var_SetCoords(vout, "mouse-clicked", x, y);
        vlc_object_release(vout);
	}
    vlibvlc_free(pp_vouts);
    return n;
}

unsigned int vlc_cursorSet (libvlc_media_player_t *p_mi, const int x, const int y)
{
    size_t n;
    vout_thread_t **pp_vouts = GetVouts(p_mi, &n);
    
    if (!pp_vouts) return 0;
    
    for (size_t i = 0; i < n; i++){
    	vout_thread_t *vout = pp_vouts[i];
    	if (!vout) continue;
        var_SetCoords(vout, "mouse-moved", x, y);
        vlc_object_release(vout);
	}
    vlibvlc_free(pp_vouts);
    return n;
}

static inline unsigned int vlc_getSize (libvlc_media_player_t *p_mi, int *width, int *height)
{
	//printf("vlc_getSize\n");

    size_t n;
    vout_thread_t **pp_vouts = GetVouts(p_mi, &n);
    if (!pp_vouts) return 0;
    
    for (size_t i = 0; i < n; i++){
    	vout_thread_t *vout = pp_vouts[i];
    	if (!vout) continue;

    	vout_thread_sys_t *p_sys = vout->p;

#if (LIBVLC_VERSION_MAJOR < 2)
		if (p_sys->p_vf2_chain){
    		if (width) *width = p_sys->p_vf2_chain->fmt_in.video.i_width;
    		if (height) *height = p_sys->p_vf2_chain->fmt_in.video.i_height;
    	}
#else
    	if (width) *width = p_sys->filter.format.i_width * (p_sys->filter.format.i_sar_num / (double)p_sys->filter.format.i_sar_den);
    	if (height) *height = p_sys->filter.format.i_height;
    	//printf("### %i,%i -%i,%i-\n", *width, *height, p_sys->filter.format.i_sar_num, p_sys->filter.format.i_sar_den);
#endif
        vlc_object_release(vout);
	}
    vlibvlc_free(pp_vouts);
    return n;
}

static inline TTITLE *dupTitles (input_title_t **i_titles, const int i_tTitles, int *tTitles)
{
	TTITLE *titles = NULL;

	//printf("dupTitles\n");
	
	if (i_tTitles){
		input_title_t *t;
		*tTitles = i_tTitles;
		titles = my_calloc(i_tTitles, sizeof(TTITLE));
		if (!titles) return NULL;
		TTITLE *title = titles;
					
		for (int i = 0; i < i_tTitles; title++,i++){
			t = i_titles[i];
			if (t){
				if (t->psz_name){
					if (!*t->psz_name){
						char *name = my_calloc(32, sizeof(char));
						if (name){
							my_snprintf(name, 31, " %i ", i+1);
							title->name = name;
						}else{
							break;
						}
					}else{
						title->name = my_strdup(t->psz_name);
					}
					//printf("[%i] '%s' %i %i\n", i, title->name, (int)t->i_length, (int)t->b_menu);
				}else{
					char *name = my_calloc(32, sizeof(char));
					if (name){
						my_snprintf(name, 31, " %i ", i+1);
						title->name = name;
					}else{
						break;
					}
					//printf("[%i] '%s'\n", i, title->name);
				}

				if (t->i_seekpoint && t->seekpoint){
					title->length = t->i_length;
					title->tSeekPoints = t->i_seekpoint;
					title->seekPoints = my_calloc(t->i_seekpoint, sizeof(TTITLESEEKPOINT));
					if (!title->seekPoints){
						my_free(title->name);
						my_free(titles);
						return NULL;
					}					

					for (int j = 0; j < t->i_seekpoint; j++){
						if (t->seekpoint[j]->psz_name){
							title->seekPoints[j].name = my_strdup(t->seekpoint[j]->psz_name);
						}else{
							char *buffer = my_calloc(32, sizeof(char));
							if (buffer){
								my_snprintf(buffer, 31, "%i", j);
								title->seekPoints[j].name = buffer;
							}else{
								title->seekPoints[j].name = my_strdup("-");
								//return NULL;
							}
						}
						title->seekPoints[j].timeOffset = t->seekpoint[j]->i_time_offset;
						//printf("[%i,%i] %I64d '%s'\n", i, j, t->seekpoint[j]->i_time_offset, t->seekpoint[j]->psz_name);
					}
				}
			}
		}
	}
	return titles;
}

int input_event_changed (vlc_object_t *p_this, char const * psz_cmd, vlc_value_t oldval, vlc_value_t newval, void *p_userdata)
{
	//if (SHUTDOWN) return 0;

	const int event = newval.i_int;

	//if (event != INPUT_EVENT_POSITION && event != INPUT_EVENT_SIGNAL && event != INPUT_EVENT_STATISTICS) 
	//	printf("input_event_changed:: %i %p %s %I64d %p\n", event, p_this, psz_cmd, newval.i_int, p_userdata);
	

	//TVLCPLAYER *vp = (TVLCPLAYER*)p_userdata;
	//TVLCCONFIG *vlc = vp->vlc;
	//input_thread_t *p_input = (input_thread_t*)p_this;

	switch (event){
	  case INPUT_EVENT_STATE:
	  	//printf("INPUT_EVENT_STATE\n");
		//printf("INPUT_EVENT_STATE %i\n", vlc_getState(vlc));
		//printf("INPUT_EVENT_STATE volume:%i\n", getVolume(vp));
		//if (getVolume(vp) == -1 /*&& vp->vlc->volume != -1*/)
		//	setVolume(vp, vp->vlc->volume);

		//sbuiDKStateChange();
		break;

		/* b_dead is true */
	  case INPUT_EVENT_DEAD: 
		//printf("INPUT_EVENT_DEAD  \n");
		//timerSet(vp, TIMER_CHAPTER_UPDATE, 50);
		//timerSet(vp, TIMER_EPG_UPDATE, 0);
		//timerSet(vp, TIMER_ES_UPDATE, 50);
		//timerSet(vp, TIMER_META_UPDATE, 50);
		//timerSet(vp, TIMER_SUB_UPDATE, 50);
	  	//sbuiDKStateChange();
		break;
	  
	  /* a *user* abort has been requested */
	  case INPUT_EVENT_ABORT:
		//printf("INPUT_EVENT_ABORT  \n");
		//sbuiDKStateChange();
		break;

      /* "rate" has changed */
	  //case INPUT_EVENT_RATE: 
		//printf("INPUT_EVENT_RATE\n");
		//break;

      /* At least one of "position" or "time" */
	   //case INPUT_EVENT_POSITION:
		//printf("INPUT_EVENT_POSITION  \n");
		//break;

      /* "length" has changed */
	  //case INPUT_EVENT_LENGTH:
	  	//	printf("INPUT_EVENT_LENGTH\n");
			//printf("INPUT_EVENT_LENGTH %I64d \n", vlc_getLength(vp->vlc));
			//printf("INPUT_EVENT_LENGTH volume:%i\n", getVolume(vp));
		//setVolume(vp, vp->vlc->volume);
		//break;

	  /* A title has been added or removed or selected.
	  * It imply that chapter has changed (not chapter event is sent) */
	  case INPUT_EVENT_TITLE: 
		//printf("INPUT_EVENT_TITLE %i/%i\n", vlc_getTitle(vlc), vlc_getTitleCount(vlc));
		//printf("INPUT_EVENT_TITLE\n");
		//timerSet(vp, TIMER_CHAPTER_UPDATE, 100);
		break;
	  
      /* A chapter has been added or removed or selected. */
	  case INPUT_EVENT_CHAPTER: 
		//printf("INPUT_EVENT_CHAPTER %i/%i\n", vlc_getChapter(vlc), vlc_getChapterCount(vlc));
		//printf("INPUT_EVENT_CHAPTER\n");
		//timerSet(vp, TIMER_CHAPTER_UPDATE, 100);
		break;

      /* A program ("program") has been added or removed or selected:
       * or "program-scrambled" has changed.*/
	  //case INPUT_EVENT_PROGRAM: 
		//printf("INPUT_EVENT_PROGRAM %i \n", vlc_getProgram(vlc));
		//printf("INPUT_EVENT_PROGRAM\n");
		//break;
		
      /* A ES has been added or removed or selected */
	  //case INPUT_EVENT_ES:
	  	/*
		int i_categories = -1;
		void *pp_categories = NULL;

		input_item_t *p_item = input_GetItem(p_input);
		if (p_item){
			i_categories = p_item->i_categories;
			pp_categories = p_item->pp_categories;
		}
		printf("INPUT_EVENT_ES %i %p\n", i_categories, pp_categories);*/

	  	//printf("INPUT_EVENT_ES\n");
		//break;
	  
      /* "teletext-es" has changed */
	  //case INPUT_EVENT_TELETEXT:
	  //printf("INPUT_EVENT_TELETEXT\n");
	   //break;

      /* "record" has changed */
	  //case INPUT_EVENT_RECORD:
		//printf("INPUT_EVENT_RECORD\n");
		//break;

      /* input_item_t media has changed */
	  case INPUT_EVENT_ITEM_META:/*{
		input_item_t *p_item = input_GetItem(p_input);
		const int textra = vlc_meta_GetExtraCount(p_item->p_meta);
	  	printf("INPUT_EVENT_ITEM_META extra %i\n", textra);*/
	  	//printf("INPUT_EVENT_ITEM_META\n");
	  	//timerSet(vp, TIMER_META_UPDATE, 10);
	  //}
		break;
		
	  case INPUT_EVENT_ITEM_EPG:
		//printf("INPUT_EVENT_ITEM_EPG\n");
		
		//epgEventSig++;
		setSigEpgEvent();
		
		break;
		
      /* input_item_t info has changed */
	  case INPUT_EVENT_ITEM_INFO:
		//printf("INPUT_EVENT_ITEM_INFO\n");
		//timerSet(vp, TIMER_ES_UPDATE, 100);
//		timerSet(vp, TIMER_EPG_UPDATE, 1000);
		//timerSet(vp, TIMER_SUB_UPDATE, 100);
		break;
		
      /* input_item_t name has changed */
	  //case INPUT_EVENT_ITEM_NAME:
		//printf("INPUT_EVENT_ITEM_NAME\n");
		//break;
	   
      /* input_item_t epg has changed */
	  //case INPUT_EVENT_ITEM_EPG:
	  /*{
		int i_epg = -1;
		void *pp_epg = NULL;

		input_item_t *p_item = input_GetItem(p_input);
		if (p_item){
			i_epg = p_item->i_epg;
			pp_epg = p_item->pp_epg;
		}
		printf("INPUT_EVENT_ITEM_EPG %i %p\n", i_epg, pp_epg);
		}*/
		//break;

      /* Input statistics have been updated */
	  //case INPUT_EVENT_STATISTICS:
	  	//printf("INPUT_EVENT_STATISTICS\n");
		//break;
		
      /* At least one of "signal-quality" or "signal-strength" has changed */
	  //case INPUT_EVENT_SIGNAL: 
	    //printf("INPUT_EVENT_SIGNAL\n");
		//break;

      /* "audio-delay" has changed */
	  //case INPUT_EVENT_AUDIO_DELAY:
		//printf("INPUT_EVENT_AUDIO_DELAY\n");
		//break;
		
      /* "spu-delay" has changed */
	  //case INPUT_EVENT_SUBTITLE_DELAY:
		//printf("INPUT_EVENT_SUBTITLE_DELAY\n");
		//break;

      /* "bookmark" has changed */
	  //case INPUT_EVENT_BOOKMARK:
		//printf("INPUT_EVENT_BOOKMARK\n");
		//break;

      /* cache" has changed */
	  //case INPUT_EVENT_CACHE:
	  	//printf("INPUT_EVENT_CACHE volume\n");
	   //printf("INPUT_EVENT_CACHE volume %i\n", getVolume(vp));
		//if (getVolume(vp) == -1 /*&& vp->vlc->volume != -1*/)
		//	setVolume(vp, vp->vlc->volume, VOLUME_APP);
		//break;

      /* A audio_output_t object has been created/deleted by *the input* */
	  //case INPUT_EVENT_AOUT:
		//printf("INPUT_EVENT_AOUT\n");
		//break;
		
	  /* A vout_thread_t object has been created/deleted by *the input* */
	  case INPUT_EVENT_VOUT: 
	    //printf("INPUT_EVENT_VOUT\n");
	  //	timerSet(vp, TIMER_NEWTRACKVARS3, 0);
		break;

	//default:
		//printf("UNKNOWN %i\n", event);
    }

	return 1;
}

void vlc_inputEventCbDel (TVLCCONFIG *vlc, void *opaque)
{
	if (vlc->mp){
		input_thread_t *p_input = libvlc_get_input_thread(vlc->mp);
		if (p_input){
			var_DelCallback(p_input, "intf-event", input_event_changed, opaque);
			vlc_object_release(p_input);
		}
	}
}

void vlc_inputEventCbSet (TVLCCONFIG *vlc, void *opaque)
{
	if (vlc->mp){
		input_thread_t *p_input = libvlc_get_input_thread(vlc->mp);
		if (p_input){
			var_AddCallback(p_input, "intf-event", input_event_changed, opaque);
			vlc_object_release(p_input);
		}
	}
}

// chapter lock must be held
TTITLE *getTitles (TVLCCONFIG *vlc, int *tTitles)
{
	*tTitles = 0;
	TTITLE *titles = NULL;
	
	//printf("getTitles\n");
	
	if (vlc->mp){
		input_thread_t *p_input = libvlc_get_input_thread(vlc->mp);
		if (p_input){
			input_thread_private_t *p = p_input->p;
			if (p){
				vlcMutexLock(&p->lock_control);
				vlcMutexLock(&p->p_item->lock);
				titles = dupTitles(p->title, p->i_title, tTitles);
				vlcMutexUnlock(&p->p_item->lock);
				vlcMutexUnlock(&p->lock_control);
				/*if (titles)
					printf("titles out %i %p\n", *tTitles, titles->seekPoints);
				else
					printf("titles out %i\n", *tTitles);*/
			}
			vlc_object_release(p_input);
		}
	}	
	return titles;
}

TCATEGORY *getCategories (TVLCCONFIG *vlc, int *tCat)
{
	*tCat = 0;
	TCATEGORY *cat = NULL;
	int i_categories = 0;
	
	//printf("getCategories\n");
	
	if (vlc->mp){
		input_thread_t *p_input = libvlc_get_input_thread(vlc->mp);
		if (p_input){
			input_item_t *p_item = input_GetItem(p_input);	
			if (p_item){
				vlcMutexLock(&p_item->lock);

				i_categories = p_item->i_categories;				
				if (i_categories){
					cat = my_calloc(i_categories, sizeof(TCATEGORY));
					if (cat == NULL) abort();

					for (int i = 0; i < i_categories; i++){
						if (p_item->pp_categories[i]->psz_name)
							cat[i].name = my_strdup(p_item->pp_categories[i]->psz_name);
						else
							cat[i].name = my_strdup("-");
						cat[i].tInfos = p_item->pp_categories[i]->i_infos;
						
						if (cat[i].tInfos){
							cat[i].infos = my_calloc(cat[i].tInfos, sizeof(TCATEGORYINFO));
							if (!cat[i].infos) abort();

							for (int j = 0; j < p_item->pp_categories[i]->i_infos; j++){
								if (p_item->pp_categories[i]->pp_infos[j]->psz_name)
									cat[i].infos[j].name = my_strdup(p_item->pp_categories[i]->pp_infos[j]->psz_name);
								if (p_item->pp_categories[i]->pp_infos[j]->psz_value)
									cat[i].infos[j].value = my_strdup(p_item->pp_categories[i]->pp_infos[j]->psz_value);
							}
						}
					}
				}
				
				vlcMutexUnlock(&p_item->lock);
			}
			vlc_object_release(p_input);
		}
	}

	if (!cat)
		*tCat = 0;
	else
		*tCat = i_categories;
	return cat;
}

TAVTRACKS *getVideoTracks (TVLCCONFIG *vlc, int *currentVideoTrack)
{
	
	*currentVideoTrack = -1;
	if (!vlc->m || !vlc->mp) return NULL;
	
	TAVTRACKS *avts = my_calloc(1, sizeof(TAVTRACKS));
	if (!avts) return NULL;
	
	avts->totalTracks = vlc_getVideoTrackCount(vlc);
	if (avts->totalTracks < 1){
		avts->totalTracks = -1;
		my_free(avts);
		return NULL;
	}
	
	avts->track = my_calloc(avts->totalTracks+1, sizeof(TAVTRACK));
	if (!avts->track){
		avts->totalTracks = -1;
		my_free(avts);
		return NULL;
	}

	*currentVideoTrack = vlc_getVideoTrack(vlc);
	libvlc_track_description_t *trks = libvlc_video_get_track_description(vlc->mp);
	if (trks){
		libvlc_track_description_t *t = trks;
		TAVTRACK *trk = avts->track;
		int tTracks = 0;
		
		while(t && tTracks < avts->totalTracks){
			if (t->i_id != -1){		// ignore 'disable' track information, whose id == -1
				trk->name = my_strdup(t->psz_name);
				trk->id = t->i_id;
				if (*currentVideoTrack == trk->id) // *currentVideoTrack is an index
					*currentVideoTrack = tTracks;
				tTracks++;

				//printf("  '%i' '%s'\n", trk->id, trk->name);
				trk++;
			}
			t = t->p_next;
		}
		libvlcTrackDescriptionRelease(trks);
		
		avts->totalTracks = tTracks;
		//printf("current Video track: %i\n", *currentVideoTrack);
	}
	//printf("\n");

	return avts;
}

TAVTRACKS *getAudioTracks (TVLCCONFIG *vlc, int *currentAudioTrack)
{
	
	*currentAudioTrack = -1;
	if (!vlc->m || !vlc->mp) return NULL;
	
	TAVTRACKS *avts = my_calloc(1, sizeof(TAVTRACKS));
	if (!avts) return NULL;
	
	avts->totalTracks = vlc_getAudioTrackCount(vlc);
	if (avts->totalTracks < 1){
		avts->totalTracks = -1;
		my_free(avts);
		return NULL;
	}
	
	avts->track = my_calloc(avts->totalTracks+1, sizeof(TAVTRACK));
	if (!avts->track){
		avts->totalTracks = -1;
		my_free(avts);
		return NULL;
	}

	*currentAudioTrack = vlc_getAudioTrack(vlc);
	libvlc_track_description_t *trks = libvlc_audio_get_track_description(vlc->mp);
	if (trks){
		libvlc_track_description_t *t = trks;
		TAVTRACK *trk = avts->track;
		int tTracks = 0;
		
		while(t && tTracks < avts->totalTracks){
			if (t->i_id != -1){		// don't want 'disable' track information, whose id == -1
				trk->name = my_strdup(t->psz_name);
				trk->id = t->i_id;
				if (*currentAudioTrack == trk->id) // *currentAudioTrack is an index
					*currentAudioTrack = tTracks;
				tTracks++;

				//printf("  '%i' '%s'\n", trk->id, trk->name);
				trk++;
			}
			t = t->p_next;
		}
		libvlcTrackDescriptionRelease(trks);
		
		avts->totalTracks = tTracks;
		//printf("current audio track: %i\n", *currentAudioTrack);
	}
	//printf("\n");

	return avts;
}


void epg_freeEPGEvent (TVLCEPGEVENT *epgevent)
{
	if (epgevent){
		if (epgevent->name)
			my_free(epgevent->name);
		if (epgevent->description)
			my_free(epgevent->description);
		if (epgevent->detail)
			my_free(epgevent->detail);
		my_free(epgevent);
	}
}

static inline TVLCEPGEVENT *epg_copyEvent (TVLCEPGEVENT *vlc_epg_event)
{
	TVLCEPGEVENT *epgevent = my_calloc(1, sizeof(TVLCEPGEVENT));
	if (epgevent){
		epgevent->start = vlc_epg_event->start;
		epgevent->duration = vlc_epg_event->duration;
		
		if (vlc_epg_event->name)
			epgevent->name = my_strdup(vlc_epg_event->name);
		if (vlc_epg_event->description)
			epgevent->description = my_strdup(vlc_epg_event->description);
		if (vlc_epg_event->detail)
			epgevent->detail = my_strdup(vlc_epg_event->detail);
	}
	return epgevent;
}

TVLCEPGEVENT *epg_dupEvent (vlc_epg_event_t *vlc_epg_event)
{

	TVLCEPGEVENT *epgevent = my_calloc(1, sizeof(TVLCEPGEVENT));
	if (epgevent){
		epgevent->start = vlc_epg_event->i_start;
		epgevent->duration = vlc_epg_event->i_duration;
		
		if (vlc_epg_event->psz_name)
			epgevent->name = my_strdup(vlc_epg_event->psz_name);
		if (vlc_epg_event->psz_short_description)
			epgevent->description = my_strdup(vlc_epg_event->psz_short_description);
		if (vlc_epg_event->psz_description)
			epgevent->detail = my_strdup(vlc_epg_event->psz_description);
	}
	return epgevent;
}

TVLCEPGEVENT *epg_getCurrent (TVLCCONFIG *vlc)
{
	TVLCEPGEVENT *current = NULL;
	
	if (vlc->mp){
		input_thread_t *p_input = libvlc_get_input_thread(vlc->mp);
		if (p_input){
			input_item_t *p_item = input_GetItem(p_input);	
			if (p_item){
				vlcMutexLock(&p_item->lock);
				if (p_item->i_epg){
					if (p_item->pp_epg[0]->p_current)
						current = epg_dupEvent(p_item->pp_epg[0]->p_current);
				}
				vlcMutexUnlock(&p_item->lock);
			}
			vlc_object_release(p_input);
		}
	}
	return current;
}

TVLCEPGEVENT *epg_getCurrentFromEpg (TVLCEPG **vepg, const int total)
{
	TVLCEPGEVENT *current = NULL;
	
	if (total && *vepg){
		if ((*vepg)->p_current)
			current = epg_copyEvent((*vepg)->p_current);
	}
	return current;
}

TVLCEPGEVENT *epg_getGetProgramme (TVLCEPG **vepg, const int total, const int chanIdx, const int progIdx)
{
	TVLCEPGEVENT *prog = NULL;
	if (vepg && chanIdx < total && vepg[chanIdx] && vepg[chanIdx]->pp_event && progIdx < vepg[chanIdx]->i_event)
		prog = epg_copyEvent(vepg[chanIdx]->pp_event[progIdx]);
	return prog;
}

char *epg_getChannelName (TVLCEPG **vepg, const int total, const int chanIdx)
{
	if (vepg && chanIdx < total && vepg[chanIdx] && vepg[chanIdx]->psz_name)
		return my_strdup(vepg[chanIdx]->psz_name);
	return NULL;
}
			
TVLCEPG **epg_dupEpg (TVLCCONFIG *vlc, int *tepg)
{
	*tepg = 0;
	TVLCEPG **epg = NULL;
	
	//printf("epg_dupEpg\n");
	
	if (vlc->mp){
		input_thread_t *p_input = libvlc_get_input_thread(vlc->mp);
		if (p_input){
			input_item_t *p_item = input_GetItem(p_input);	
			
			if (p_item){
				vlcMutexLock(&p_item->lock);
				
				if (p_item->pp_epg && p_item->i_epg){
					*tepg = p_item->i_epg;
					
					epg = my_calloc(p_item->i_epg, sizeof(TVLCEPG*));
					if (epg){
						vlc_epg_t **vepg = p_item->pp_epg;

						for (int i = 0; i < p_item->i_epg; i++){
							epg[i] = my_calloc(1, sizeof(TVLCEPG));
							if (epg[i]){
								if (vepg[i]->psz_name)
									epg[i]->psz_name = my_strdup(vepg[i]->psz_name);
								if (vepg[i]->p_current){
									epg[i]->p_current = epg_dupEvent(vepg[i]->p_current);
									//printf("[%i] %p\n", i, vepg[i]->p_current);
								}
								
								epg[i]->i_event = vepg[i]->i_event;
								if (epg[i]->i_event){
									epg[i]->pp_event = my_calloc(epg[i]->i_event, sizeof(TVLCEPGEVENT*));
									
									if (epg[i]->pp_event){
										for (int j = 0; j < epg[i]->i_event; j++){
											epg[i]->pp_event[j] = epg_dupEvent(vepg[i]->pp_event[j]);
											//printf("[%i,%i] %p\n", i, j, vepg[i]->pp_event[j]);
										}
									}
								}
							}
						}
					}
				}
				vlcMutexUnlock(&p_item->lock);
			}
			vlc_object_release(p_input);
		}
	}

	return epg;
}

void epg_freeEpg (TVLCEPG **epg, const int total)
{
	if (!total) return;
	
	for (int i = 0; i < total; i++){
		if (epg[i]->psz_name)
			my_free(epg[i]->psz_name);
		
		if (epg[i]->p_current)
			epg_freeEPGEvent(epg[i]->p_current);
			
		if (epg[i]->i_event){
			for (int j = 0; j < epg[i]->i_event; j++)
				epg_freeEPGEvent(epg[i]->pp_event[j]);
			my_free(epg[i]->pp_event);
		}
		my_free(epg[i]);
	}
	my_free(epg);
}

const unsigned char *vlc_getVersion ()
{
	return (const unsigned char*)libvlc_get_version();
}

const unsigned char *vlc_getCompiler ()
{
	return (unsigned char*)libvlc_get_compiler();
}

libvlc_instance_t *vlc_init (TVLCCONFIG *vlc, int argc, char **argv)
{
	libvlc_instance_t *hLib = libvlc_new(argc, (const char **)argv);
	if (hLib)
		libvlc_set_user_agent(hLib, "Vlc", "2.x");
	return hLib;
}	

int vlc_getMute (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_audio_get_mute(vlc->mp);
	else
		return 0;
}

void vlc_setMute (TVLCCONFIG *vlc, const int status)
{
	if (vlc->mp)
		libvlc_audio_set_mute(vlc->mp, status);
}

int vlc_getVolume (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_audio_get_volume(vlc->mp);
	else
		return 0;
}

void vlc_setVolume (TVLCCONFIG *vlc, int volume)
{
	if (vlc->mp)
		libvlc_audio_set_volume(vlc->mp, volume);
}

void vlc_pause (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		libvlc_media_player_pause(vlc->mp);
}

void vlc_play (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		libvlc_media_player_play(vlc->mp);
}

void vlc_stop (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		libvlc_media_player_stop(vlc->mp);
}

float vlc_getPosition (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_media_player_get_position(vlc->mp);
	else
		return 0.0f;
}

void vlc_setPosition (TVLCCONFIG *vlc, float position)
{
	if (vlc->mp)
		libvlc_media_player_set_position(vlc->mp, position);
}

int vlc_getChapterCount (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_media_player_get_chapter_count(vlc->mp);
	else
		return -1;
}

int vlc_getChapter (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_media_player_get_chapter(vlc->mp);
	else
		return -1;
}

void vlc_setChapter (TVLCCONFIG *vlc, int chapter)
{
	if (vlc->mp)
		libvlc_media_player_set_chapter(vlc->mp, chapter);
}

void vlc_navigate (TVLCCONFIG *vlc, const int action)
{
#if (LIBVLC_VERSION_MAJOR >= 2)
	if (vlc->mp)
		libvlc_media_player_navigate(vlc->mp, action);
#endif
}

libvlc_time_t vlc_getLength (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_media_player_get_length(vlc->mp);
	else
		return 0;
}

int vlc_getState (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_media_player_get_state(vlc->mp);
	else
		return 0;
}

libvlc_media_t *vlc_new_mrl (TVLCCONFIG *vlc, const char *mediaPath)
{
	//printf("vlc_new_mrl '%s'\n", mediaPath);
	if (vlc->hLib)
		return libvlc_media_new_location(vlc->hLib, mediaPath);
	else
		return NULL;
}

libvlc_media_t *vlc_new_path (TVLCCONFIG *vlc, const char *mediaPath)
{
	//printf("vlc_new_path '%s'\n", mediaPath);
	if (vlc->hLib && *mediaPath)
		return libvlc_media_new_path(vlc->hLib, mediaPath);
	else
		return NULL;
}

libvlc_media_t *vlc_new_node (TVLCCONFIG *vlc, const char *mediaPath)
{
	//printf("vlc_new_node '%s'\n", mediaPath);
	if (vlc->hLib)
		return libvlc_media_new_as_node(vlc->hLib, mediaPath);
	else
		return NULL;
}

libvlc_media_player_t *vlc_newFromMedia (TVLCCONFIG *vlc)
{
	if (vlc->m)
		return libvlc_media_player_new_from_media(vlc->m);
	else
		return NULL;
}

void vlc_mediaRelease (TVLCCONFIG *vlc)
{
	if (vlc->m){
		libvlc_media_release(vlc->m);
		vlc->m = NULL;
	}
}

void vlc_mp_release (TVLCCONFIG *vlc)
{
	if (vlc->mp){
		libvlc_media_player_release(vlc->mp);
		vlc->mp = NULL;
	}
	vlc_mediaRelease(vlc);
}

void vlc_releaseLib (TVLCCONFIG *vlc)
{
	if (vlc->hLib){
		libvlc_release(vlc->hLib);
		vlc->hLib = NULL;
	}
}

int vlc_isSeekable (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_media_player_is_seekable(vlc->mp);
	else
		return 0;
}

int vlc_canPause (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_media_player_can_pause(vlc->mp);
	else
		return 0;
}

char *vlc_EventTypeToName (int event)
{
	return (char*)libvlc_event_type_name(event);
}

libvlc_event_manager_t *vlc_getMediaPlayerEventManager (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return vlc->emp = libvlc_media_player_event_manager(vlc->mp);
	else
		return NULL;
}

libvlc_event_manager_t *vlc_getMediaEventManager (TVLCCONFIG *vlc)
{
	if (vlc->m)
		return vlc->em = libvlc_media_event_manager(vlc->m);
	else
		return NULL;
}

void vlc_eventMediaAttach (TVLCCONFIG *vlc, libvlc_event_type_t eventType, libvlc_callback_t callBack, void *udata)
{
	if (vlc->em)
		libvlc_event_attach(vlc->em, eventType, callBack, udata);
}

void vlc_eventMediaDetach (TVLCCONFIG *vlc, libvlc_event_type_t eventType, libvlc_callback_t callBack, void *udata)
{
	if (vlc->em)
		libvlc_event_detach(vlc->em, eventType, callBack, udata);
}

void vlc_eventPlayerAttach (TVLCCONFIG *vlc, libvlc_event_type_t eventType, libvlc_callback_t callBack, void *udata)
{
	if (vlc->emp)
		libvlc_event_attach(vlc->emp, eventType, callBack, udata);
}

void vlc_eventPlayerDetach (TVLCCONFIG *vlc, libvlc_event_type_t eventType, libvlc_callback_t callBack, void *udata)
{
	if (vlc->emp)
		libvlc_event_detach(vlc->emp, eventType, callBack, udata);
}

int vlc_saveMeta (TVLCCONFIG *vlc)
{
	int ret = 0;
	if (vlc->m)
		ret = libvlc_media_save_meta(vlc->m);
	return ret;
}

void vlc_setMeta (TVLCCONFIG *vlc, libvlc_meta_t e_meta, const char *metaStr)
{
	if (vlc->m)
		libvlc_media_set_meta(vlc->m, e_meta, metaStr);
}

char *vlc_getMeta (TVLCCONFIG *vlc, libvlc_meta_t e_meta)
{
	char *cpy = NULL;
	if (vlc->m){
		char *tag = libvlc_media_get_meta(vlc->m, e_meta);
		//printf("libvlc_media_get_meta tag %p\n", tag);
		if (tag){
			if (*tag)
				cpy = my_strdup(tag);
				
			//printf("libvlc_media_get_meta tag free %p\n", tag);
			vlibvlc_free(tag);
		}
	}
	return cpy;
}
/*
int vlc_metaExtraGetCount (TVLCCONFIG *vlc)
{
	int ct = 0;

	if (vlc->mp){
		input_thread_t *p_input = libvlc_get_input_thread(vlc->mp);
		if (p_input){
			input_item_t *p_item = input_GetItem(p_input);
			if (p_item && p_item->p_meta)
				ct = vlc_meta_GetExtraCount(p_item->p_meta);
			vlc_object_release(p_input);
		}
	}
	return ct;
}
*/

vlc_meta_extra_t *vlc_metaExtraGet (TVLCCONFIG *vlc)
{
	vlc_meta_extra_t *extra = NULL;
	
	//printf("vlc_metaExtraGet\n");
	
	if (vlc->mp){
		input_thread_t *p_input = libvlc_get_input_thread(vlc->mp);
		if (p_input){
			input_item_t *p_item = input_GetItem(p_input);
			if (p_item && p_item->p_meta){
				vlcMutexLock(&p_item->lock);
				
				int ct = vlc_meta_GetExtraCount(p_item->p_meta);
				if (ct){
					char **pp_extra = vlc_meta_CopyExtraNames(p_item->p_meta);
					if (pp_extra){
						extra = my_calloc(1, sizeof(vlc_meta_extra_t));
						if (extra){
							extra->meta = my_calloc(ct, sizeof(vlc_meta_str_t));
							if (extra->meta){
								extra->total = ct;

								for (int i = 0; i < ct; i++){
									const char *meta = vlc_meta_GetExtra(p_item->p_meta, pp_extra[i]);
									if (!meta) break;
									extra->meta[i] = my_calloc(1, sizeof(vlc_meta_str_t));
									if (!extra->meta[i]) break;

									extra->meta[i]->name = my_strdup(pp_extra[i]);
									extra->meta[i]->value = my_strdup(meta);
									//printf("extra %i, '%s' '%s'\n", i, extra->meta[i]->name, extra->meta[i]->value);
									vlibvlc_free(pp_extra[i]);
								}
							}
						}
						vlibvlc_free(pp_extra);
					}
				}
				vlcMutexUnlock(&p_item->lock);
			}
			vlc_object_release(p_input);
		}
	}
	return extra;
}

void vlc_metaExtraFree (vlc_meta_extra_t *extra)
{
	if (extra){
		for (int i = 0; i < extra->total; i++){
			my_free(extra->meta[i]->name);
			my_free(extra->meta[i]->value);
			my_free(extra->meta[i]);
		}
		my_free(extra->meta);
		my_free(extra);
	}
}

void vlc_mediaParse (TVLCCONFIG *vlc)
{
	if (vlc->m)
		libvlc_media_parse(vlc->m);
}

void vlc_mediaParseAsync (TVLCCONFIG *vlc)
{
	if (vlc->m)
		libvlc_media_parse_async(vlc->m);
}

int vlc_getVideoSize (TVLCCONFIG *vlc, int *w, int *h)
{
	
	if (vlc->m && vlc->mp){
		*w = 0; *h = 0;
		vlc_getSize(vlc->mp, w, h);
		if (*w && *h) return 1;
		
		int ret = 0;
		int total = libvlc_video_get_track_count(vlc->mp);
		//printf("total video tracks %i\n", total);

		for (int i = 0; i < total; i++){
			*w = 0; *h = 0;
			ret = libvlc_video_get_size(vlc->mp, i, (unsigned int*)w, (unsigned int*)h);
			//printf("libvlc_video_get_size %i %i: %i %i\n", ret, i, *w, *h);
			if (*w && *h) return 1;
		}
		
		if ((!*w && !*h) || ret < 0){
			ret = 0;

#if (LIBVLC_VERSION_MAJOR == 2 && LIBVLC_VERSION_MINOR == 0)
			libvlc_media_track_info_t *tracks = NULL;
			int total = libvlc_media_get_tracks_info(vlc->m, &tracks);
			if (total > 0){
				//printf("total tracks %i\n", total);
				
				for (int i = 0; i < total; i++){
					//printf("track %i, type:%i\n", i, tracks[i].i_type);
					
					if (tracks[i].i_type == libvlc_track_video){
						*w = tracks[i].u.video.i_width;
						*h = tracks[i].u.video.i_height;
						//printf("%i: %i %i\n", i, *w, *h);
						if (*w && *h){
							ret = 1;
							break;
						}
					}
				}
				vlibvlc_free(tracks);
			}
#else
			libvlc_media_track_t **tracks = NULL;
			int total = libvlc_media_tracks_get(vlc->m, &tracks);
			if (total > 0){
				//printf("total tracks %i\n", total);
				
				for (int i = 0; i < total; i++){
					//printf("track %i, type:%i\n", i, tracks[i].i_type);
					
					if (tracks[i]->i_type == libvlc_track_video){
						*w = tracks[i]->video->i_width;
						*h = tracks[i]->video->i_height;
						//printf("%i: %i %i\n", i, *w, *h);
						if (*w && *h){
							ret = 1;
							break;
						}
					}
				}
				libvlc_media_tracks_release(tracks, total);
			}
#endif
		}
		return ret;
	}else{
		return -1;
	}
}

int vlc_getVideoTrackCount (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_video_get_track_count(vlc->mp);
	else
		return 0;
}

int vlc_getVideoTrack (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_video_get_track(vlc->mp);
	else
		return -1;
}

int vlc_setVideoTrack (TVLCCONFIG *vlc, const int id)
{
	if (vlc->mp)
		return libvlc_video_set_track(vlc->mp, id);
	else
		return -1;
}

int vlc_getAudioTrackCount (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_audio_get_track_count(vlc->mp);
	else
		return 0;
}

int vlc_getAudioTrack (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_audio_get_track(vlc->mp);
	else
		return -1;
}

int vlc_setAudioTrack (TVLCCONFIG *vlc, const int id)
{
	if (vlc->mp)
		return libvlc_audio_set_track(vlc->mp, id);
	else
		return -1;
}

int vlc_getSubtitleCount (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_video_get_spu_count(vlc->mp);
	else
		return -1;
}

libvlc_track_description_t *vlc_getSubtitleDescriptions (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_video_get_spu_description(vlc->mp);
	else
		return NULL;
}

int vlc_setSubtitle (TVLCCONFIG *vlc, int idx)
{
	if (vlc->mp)
		return libvlc_video_set_spu(vlc->mp, idx);
	else
		return -1;
}

int vlc_getSubtitle (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_video_get_spu(vlc->mp);
	else
		return -1;
}

int vlc_setSubtitleDelay (TVLCCONFIG *vlc, int delay)
{
	if (vlc->mp)
		return libvlc_video_set_spu_delay(vlc->mp, delay);
	else
		return -1;
}

int vlc_getSubtitleDelay (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_video_get_spu_delay(vlc->mp);
	else
		return -1;
}

int vlc_willPlay (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_media_player_will_play(vlc->mp);
	else
		return -1;
}

void vlc_nextChapter (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		libvlc_media_player_next_chapter(vlc->mp);
} 

void vlc_previousChapter (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		libvlc_media_player_previous_chapter(vlc->mp);
} 

void vlc_setAspectRatio (TVLCCONFIG *vlc, const char *aspect)
{
	if (vlc->mp)
		libvlc_video_set_aspect_ratio(vlc->mp, aspect);
}

char *vlc_getAspectRatio (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_video_get_aspect_ratio(vlc->mp);
	else
		return NULL;
}

int vlc_setProgram (TVLCCONFIG *vlc, const int newProg)
{
	if (vlc->mp){
		input_thread_t *p_input = libvlc_get_input_thread(vlc->mp);
		if (p_input){
			var_SetInteger(p_input, "program", newProg);
			vlc_object_release(p_input);
			return 1;
		}
	}
	return 0;
}

int vlc_getProgram (TVLCCONFIG *vlc)
{
	if (vlc->mp){
		input_thread_t *p_input = libvlc_get_input_thread(vlc->mp);
		if (p_input){
			int prog = var_GetInteger(p_input, "program");
			vlc_object_release(p_input);
			return prog;
		}
	}
	return -1;
}

void vlc_setTitle (TVLCCONFIG *vlc, int title)
{
	if (vlc->mp){
		libvlc_media_player_set_title(vlc->mp, title);
		//lSleep(10);		// let VLC change/load/process the media
	}
}

int vlc_getTitle (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_media_player_get_title(vlc->mp);
	else
		return 0;
}

int vlc_prevTitle (TVLCCONFIG *vlc)
{
	if (vlc->mp){
		int title = vlc_getTitle(vlc);
		if (--title >= 0)
			vlc_setTitle(vlc, title);
	}
	return vlc_getTitle(vlc);
}

int vlc_nextTitle (TVLCCONFIG *vlc)
{
	if (vlc->mp){
		int tTitle = vlc_getTitleCount(vlc);
		int title = vlc_getTitle(vlc);
		if (++title < tTitle)
			vlc_setTitle(vlc, title);
	}
	return vlc_getTitle(vlc);
}

int vlc_getTitleChapterCount (TVLCCONFIG *vlc, int title)
{
	if (vlc->mp)
		return libvlc_media_player_get_chapter_count_for_title(vlc->mp, title);
	else
		return 0;
}

int vlc_getTitleCount (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_media_player_get_title_count(vlc->mp);
	else
		return 0;
}

libvlc_track_description_t *vlc_getTitleDescriptions (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_video_get_title_description(vlc->mp);
	else
		return NULL;
}

char *vlc_getTitleDescription (TVLCCONFIG *vlc, const int titleId)
{
	libvlc_track_description_t *d = vlc_getTitleDescriptions(vlc);
	if (d){
		for (int i = 0; d; i++){
			if (i == titleId && d->psz_name && *(d->psz_name)){
				char *str = my_strdup(d->psz_name);
				libvlcTrackDescriptionRelease(d);
				return str;
			}
			d = d->p_next;
		}
		libvlcTrackDescriptionRelease(d);
	}
	return NULL;
}

libvlc_track_description_t *vlc_getChapterDescriptions (TVLCCONFIG *vlc, int title)
{
	if (vlc->mp)
		return libvlc_video_get_chapter_description(vlc->mp, title);
	else
		return NULL;
}

char *vlc_getChapterDescription (TVLCCONFIG *vlc, int chapterId, int titleId)
{
	libvlc_track_description_t *d = vlc_getChapterDescriptions(vlc, titleId);
	if (d){
		for (int i = 0; d; i++){
			if (i == chapterId && d->psz_name && *(d->psz_name)){
				char *str = my_strdup(d->psz_name);
				libvlcTrackDescriptionRelease(d);
				return str;
			}
			d = d->p_next;
		}
		libvlcTrackDescriptionRelease(d);
	}
	return NULL;
}

void vlc_setVideoCallbacks (TVLCCONFIG *vlc,
    void *(*lock) (void *data, void **plane),
    void (*unlock) (void *data, void *picture, void *const *plane),
    void (*display) (void *data, void *picture),
    void *data)
{

	if (vlc->mp)
		libvlc_video_set_callbacks(vlc->mp, lock, unlock, display, data);
}

void vlc_setVideoFormat (TVLCCONFIG *vlc, const char *chroma,
						unsigned int width, unsigned int height,
						unsigned int pitch)
{
	if (vlc->mp)
		libvlc_video_set_format(vlc->mp, chroma, width, height, pitch);
}

void vlc_addOption (TVLCCONFIG *vlc, const char *ppsz_options)
{
	if (vlc->m){
		//printf("vlc_addOption '%s'\n",ppsz_options);
		libvlc_media_add_option(vlc->m, ppsz_options);
	}
}

void vlc_setAdjustFloat (TVLCCONFIG *vlc, unsigned int option, const double value)
{
	if (vlc->mp)
		libvlc_video_set_adjust_float(vlc->mp, option, value);
}

void vlc_setAdjustInt (TVLCCONFIG *vlc, unsigned int option, const int value)
{
	if (vlc->mp)
		libvlc_video_set_adjust_int(vlc->mp, option, value);
}

void vlc_setMarqueeInt (TVLCCONFIG *vlc, unsigned int option, int val)
{
	if (vlc->mp)
		libvlc_video_set_marquee_int(vlc->mp, option, val);
}

void vlc_setMarqueeStr (TVLCCONFIG *vlc, unsigned int option, char *str)
{
	if (vlc->mp)
		libvlc_video_set_marquee_string(vlc->mp, option, str);
}

int vlc_isParsed (TVLCCONFIG *vlc)
{
	if (vlc->m)
		return libvlc_media_is_parsed(vlc->m);
	else
		return 0;
}

int vlc_getStats (TVLCCONFIG *vlc, libvlc_media_stats_t *stats)
{
	if (vlc->m && stats){
		memset(stats, 0, sizeof(libvlc_media_stats_t));
		return libvlc_media_get_stats(vlc->m, stats);
	}else{
		return 0;
	}
}

#if 0
//#if (LIBVLC_VERSION_MAJOR >= 2 && LIBVLC_VERSION_MINOR >= 1)
unsigned int vlc_equalizerPresetGetTotal ()
{
	return libvlc_audio_equalizer_get_preset_count();
}

const char *vlc_equalizerPresetGetName (const unsigned int index)
{
	return libvlc_audio_equalizer_get_preset_name(index);
}

unsigned int vlc_equalizerBandsGetCount ()
{
	return libvlc_audio_equalizer_get_band_count();
}

double vlc_equalizerBandsGetFrequency (const unsigned int index)
{
	return libvlc_audio_equalizer_get_band_frequency(index);
}

libvlc_equalizer_t *vlc_equalizerNew ()
{
	return libvlc_audio_equalizer_new();
}

libvlc_equalizer_t *vlc_equalizerNewFromPreset (const unsigned int index)
{
	return libvlc_audio_equalizer_new_from_preset(index);
}

void vlc_equalizerRelease (libvlc_equalizer_t *eq)
{
	if (eq)
		libvlc_audio_equalizer_release(eq);
}

int vlc_equalizerPreampSet (libvlc_equalizer_t *eq, const double preamp)
{
	if (eq)
		return libvlc_audio_equalizer_set_preamp(eq, preamp);
	else
		return -1;
}

double vlc_equalizerPreampGet (libvlc_equalizer_t *eq)
{
	if (eq)
		return libvlc_audio_equalizer_get_preamp(eq);
	else
		return 0.0;
}

int vlc_equalizerAmpSetByIndex (libvlc_equalizer_t *eq, const double amp, const unsigned int index)
{
	if (eq)
		return libvlc_audio_equalizer_set_amp_at_index(eq, amp, index);
	else
		return -1;
}

double vlc_equalizerAmpGetByIndex (libvlc_equalizer_t *eq, const unsigned int index)
{
	if (eq)
		return libvlc_audio_equalizer_get_amp_at_index(eq, index);
	else
		return 0.0;
}

int vlc_equalizerApply (libvlc_media_player_t *mp, libvlc_equalizer_t *eq)
{
	if (eq)
		return libvlc_media_player_set_equalizer(mp, eq);
	else
		return -1;
}
#endif


                                                    