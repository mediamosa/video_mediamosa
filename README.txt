// $Id$

Introduction
------------
This module provides the Video module with an additional transcoder using
the MediaMosa platform.

MediaMosa is a video encoding platform developed by MadCap:
 http://www.madcap.nl.

A full description of the MediaMosa platform is available at:
 http://www.mediamosa.org/
 
Quote from mediamosa.org: "MediaMosa is open source software to build a Full 
Featured, Webservice Oriented Media Management and Distribution platform. 
MediaMosa is based on the Drupal CMS. It offers user-level and 
administrator-level access to (shared) data repositories, metadata databases, 
and transcoding and streaming... "

Extended documentation is available on the project page.


Explanation on uploading a video
--------------------------------
This section gives some insight into the internal workings of the transcoder.

The global process of uploading a video to the website and transcoding it with 
MediaMosa
 
1. upload the video to the website
2. create a record in the video_mediamosa table with status = queued
3. the cron video->processQueue will create an asset, a mediafile and an 
   upload ticket for each queued file
4. the files are uploaded to mediamosa, each file first set to `uploading` and 
   after uploading `encoding`
5. mediamosa reports via a http request that the encoding for a mediafile has 
   started, a record in video_mediamosa_mediafile is created with the status
   `encoding`. When all mediafiles associated with this asset reach this status
   the asset itself in video_mediamosa is updated to this status.
6. mediamosa reports via a http request that the encoding of a mediafile is 
   finished, the record in video_mediamosa_mediafile is updated to the status
   `encoded`. When all mediafiles associated with this asset reach this status
   the asset itself in video_mediamosa is updated to this status.
7. the website will download the encoded file and add it to 
   video_mediamosa_mediafile and if available, the stills will be downloaded 
   and placed in the video_mediamosa.thumbnails field. The status will be 
   updated to `success`


Extented features
-----------------
When using this module with the provided video_mediamosa_upload upload steps 
2-4 are replaced with the following steps:

1. The website will request an upload ticket
2. On submitting the form the video's will first be uploaded directly to the 
   MediaMosa platform. When uploading the user will be notified of the upload 
   progress via a progressbar.
   
This module saves data traffic by not having to upload the video twice (to the
website and to MediaMosa).

Requirements
------------
Prerequisites (not including module requirements):

- This module assumes that you have basic knowledge about the MediaMosa 
  platform
- A working MediaMosa server with a valid account and available transcoding
  profiles. 

Future
------

The following features are planned:

- Video's now require downloading to the local website. This must be made 
  optional
- Option to upload another thumb, other than the generated thumbs by MediaMosa.

- ...

Maintainers
-----------
This module is fully sponsered and developed in-house at Synetic BV,
 http://www.synetic.nl/

Synetic BV is a fuil service internet company with extensive experience in 
building dynamic webapplications and enterprise websites with Drupal.

Maintainers:
	Michiel Nugter : michiel at synetic dot nl
	Bart van der Hoek : barth at synetic dot nl
