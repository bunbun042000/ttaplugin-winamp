in_tta.dll Ver3.2 Modified(Beta12) 
=====================================================

*This plugin 
is based on TAU Software(http://en.true-audio.com/)
TTA plug-in ver 3.2 for Winamp 2,5.
The modification is ID3v2 Tag read/write function using 
taglib(http://developer.kde.org/~wheeler/taglib.html).
And add read metadata (include album art) for MediaLibray.
From Beta9, decode engine was replaced by
TTA encoder/decoder/decoder multiplatform library, C++ version
(http://en.true-audio.com/)
This plugin performs Winamp Ver.2.9 or newer.

*Install
 Please copy in_tta.dll to Plugins folder in Winamp and also copy tag.dll(https://github.com/downloads/bunbun042000/taglib-modified/tag.dll) to Plugins folder in Winamp and Winamp base folder.

*Release Notes
2011-11-27 Beta12 Fix freeze when invalid file will open.
                  Fix hung up when reading file with no album art.
2011-11-21 Beta11 Change base library to libtta++2.1.
                  Change taglib included to use external taglib dll.
                  Fix memory leak when reading album art.
2011-11-17 Beta10 Change base taglib 1.7.0.
                  Change compiler to VS2010.
2010-11-29 Beta9  Add reading album art.
2009-04-09 Beta8  Change compiler to VC2008.
                  Change Year tag from TDRL to TDRC in ID3 Ver2.4.
2007/10/23 Beta7  Fix incorrect Seek position.
                  (Thanks to h0shu (http://hoshustep.hp.infoseek.co.jp/dust.html))
                  Add format conversion function.
2006/07/03 Beta6  Change read year metadata.
2006/02/23 Beta5  Fix incorrect Beta4
2005/12/19 Beta4  Fix problem that other program cannot access playing tta file.
2005/12/16 Beta3  Fix reading ID3v2 Ver2.3 (incorrect decoding frame size)
2005/12/14 Beta2  Initial release.

*Copying
This libray is distributed under LGPL2.1

*Acknowledgement

Refer in_mpg123.dll source code made by Otachan(http://www3.cypress.ne.jp/otachan/)in reading metadata for media library.

Refer STEP(SuperTagEditor-kai Plugin Version (Nightmare)) source code made by Haseta(http://haseta2003.hp.infoseek.co.jp/) in ID3v2 related.

Refer patch made by h0shu(http://hoshustep.hp.infoseek.co.jp/dust.html) in fixing seek problem.

Yamagata Fumihiro
mailto:bunbun042000@gmail.com
