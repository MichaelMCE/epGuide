epGuide - epGuide - A DVB Programme Guide aggregator and launcher.

Displays Digital TV listings as retrieved from an attached DVB-T USB dongle.



This was a product not intended for public release, but here it is anyway.


For a general release few things:
  Remove all static objects (throw them in a data structure to be forgotten..).
  Programme data shoudl be stored in a sorted list on a per channel basis. Currently its where ever the single pane control throws it. This will improve UI performance.
  Removed all hardcoded freq. and pid data, replace with something read from a config file. 

Would be nice to have:
  A programme meta search/highlight feature.
  Make use of the ufont renders animation render feature to highlight various metadata.





PID's set in guide.c
Transponder freq's set in epg/provider.c

