epGuide - A DVB-T Programme Guide viewer.

Displays Digital TV listings as retrieved from an attached DVB-T USB dongle.



This project is the product a task I set myself to determine how rapidly could I produced a usable & to spec application.
Secondary goal as a demonstration of the uFont library.


Not intended for public release!

For a general release few things:
  Remove all static objects (throw them in a data structure to be forgotten..).
  Programme data shoudl be stored in a sorted list on a per channel basis. Currently its where ever the single pane control throws it. This will improve UI performance.
  Removed all hardcoded freq. and pid data, replace with something read from a config file. 

Would be nice to have:
  A programme meta search/highlight feature.
  Make use of the ufont renders animation render feature to highlight various metadata.





PID's set in guide.c
Transponder freq's set in epg/provider.c

