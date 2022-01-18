MultiMail Offline Reader
========================

MultiMail is an offline mail packet reader for Unix / Linux, MS-DOS,
OS/2, Windows, macOS, and other systems, using a curses-based interface.
It supports the Blue Wave, QWK, OMEN, SOUP and OPX formats.

MultiMail is free, open source software, distributed under the [GNU
General Public License][gpl], version 3 or later.

You can get the latest version at:

   <https://wmcbrine.com/MultiMail/>

See [INSTALL] for the installation procedure, and the [man page] for
information on usage.


Version 0.52 - 2019-04-01
-------------------------

* Long To and From lines in QWKE replies
* Blue Wave tear lines improved
* Build and documentation cleanups

See the [HISTORY] file for other changes.


Downloads
---------

* Source code
    - [TAR]
    - [ZIP]
* MS-DOS
    - [x86-32][dos] Also works under Windows 9x, etc.
    - [x86-16][xt]
* OS/2
    - [x86-32 VIO][os2] Needs at least OS/2 2.0.
* Windows
    - [x86-32 console][w32] Needs 9x or later (tested through 10).
    - [x86-64 console][w64] Only tested on Windows 10.
* macOS
    - [x86-64 Terminal][mac] Only tested on macOS 10.14.


Mailing lists
-------------

Join the MultiMail [announce] list and/or the [discussion] list.


Other links
-----------

* [Screen shots]
* Format specifications:
    - [Blue Wave] ([archive][bwarc])
    - [QWK] (including QWKE) ([archive][qwkarc])
    - [OMEN]
    - [SOUP]
* [GitHub Page]
* [SourceForge Page]


Credits
-------

MultiMail was originally developed under Linux by Kolossvary Tamas and
Toth Istvan. John Zero was the maintainer for versions 0.2 through 0.6;
since version 0.7, the maintainer is [William McBrine].

Additional code has been contributed by Peter Krefting, Mark D. Rejhon,
Ingo Brueckl, Robert Vukovic, and Frederic Cambus.

Bug reports and suggestions are noted in the [HISTORY] file.


[gpl]: LEGAL.md
[HISTORY]: HISTORY.md
[INSTALL]: INSTALL.md
[man page]: MANUAL.md

[TAR]: https://github.com/wmcbrine/MultiMail/archive/refs/tags/0.52.tar.gz
[ZIP]: https://github.com/wmcbrine/MultiMail/archive/refs/tags/0.52.zip

[dos]: https://github.com/wmcbrine/MultiMail/releases/download/0.52/mmdos052.zip
[xt]: https://github.com/wmcbrine/MultiMail/releases/download/0.52/mmxt052.zip
[os2]: https://github.com/wmcbrine/MultiMail/releases/download/0.52/mmos2052.zip
[w32]: https://github.com/wmcbrine/MultiMail/releases/download/0.52/mmwin052.zip
[w64]: https://github.com/wmcbrine/MultiMail/releases/download/0.52/mmw64052.zip
[mac]: https://github.com/wmcbrine/MultiMail/releases/download/0.52/mmmac052.zip

[Screen shots]: https://wmcbrine.com/mmail/snaps.html

[Blue Wave]: https://wmcbrine.com/mmail/specs/bwdev300.txt
[bwarc]: https://wmcbrine.com/mmail/specs/bwdev300.tar.gz
[QWK]: https://wmcbrine.com/mmail/specs/qwkspecs.txt
[qwkarc]: https://wmcbrine.com/mmail/specs/qwkspecs.tar.gz
[OMEN]: https://wmcbrine.com/mmail/specs/omen-i.txt
[SOUP]: https://wmcbrine.com/mmail/specs/soup12.md

[GitHub Page]: https://github.com/wmcbrine/MultiMail
[SourceForge Page]: https://sourceforge.net/projects/multimail/

[announce]: https://lists.sourceforge.net/lists/listinfo/multimail-announce
[discussion]: https://lists.sourceforge.net/lists/listinfo/multimail-user

[William McBrine]: https://wmcbrine.com/
