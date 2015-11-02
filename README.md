Florilege README
================

Florilege is a simple FFmpeg multimedia player meant to be as minimalistic as possible.
It uses SDL for video display.
It is *not* efficient, as it does not use hardware acceleration and the application is not multi-threaded.
It was made both for quick testing and educational purposes.

It is possible to:
------------------

* Play a media;
* Modify the software at your will to test various features.

It is not possible to:
----------------------

* Control the media, including seeking and play/pausing;
* Resize the media;
* Apply filters etc.

Using Florilege
===============

Building
--------

Just run:

    make

Running
-------

Run:

	./florilege <stream>

where <stream> is any media stream (video file, real-time stream over RTSP etc.)

Contributing
============

Contributing is encouraged, mostly if you find bugs.
The features will probably not grow beyond being able to play a media without artefacts of any kind.

TODO
----

* Make a generic Makefile
* Fix memory leaks
* Implement audio decoding

Credits
-------

Credits go mostly to Stéphane Ranger and his famous [FFmpeg tutorial](http://dranger.com/ffmpeg).
