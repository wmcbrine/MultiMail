MultiMail 1 "January 17, 2022"
==============================


NAME
----

mm - offline mail reader for Blue Wave, QWK, OMEN, SOUP and OPX packets


SYNOPSIS
--------

`mm` [`-option1 value`] [`-option2 value`] [...] [*file1*] [*file2*] [...]


DESCRIPTION
-----------

MultiMail is an offline mail packet reader, supporting the Blue Wave,
QWK, OMEN, SOUP and OPX formats. It uses a simple curses-based
interface.

SOUP is used for Internet email and Usenet. The other formats are
primarily used with dialup or telnet BBSes, to save connect time and to
provide a better interface to the message base.


USAGE
-----

On most screens, a summary of the available keystroke commands is
displayed in the lower part of the screen. (You can disable this, and
reclaim some screen real estate, by turning on "ExpertMode".) Note that
for lack of space, not all commands are listed on every screen where
they're available. For example, the search functions, which are
available everywhere, are summarized only in the packet list and address
book. The principle, albeit not one that's consistently implemented, is
that the summary need appear only on the first screen where the commands
are available. When in doubt, try one and see if it works. :-)

In the letter window or ANSI viewer, pressing F1 or '?' will bring up a
window listing the available commands.

The basic navigation keys, available throughout the program, consist of
the standard cursor and keypad keys, with Enter to select. For terminals
without full support for these keys, aliases are available for some of
them:

ESC   = Q  
PgDn  = B  
PgUp  = F  
Right = +  
Left  = -

(Although shown in capitals, these may be entered unshifted.)

With "Lynx-style navigation", activated by the "UseLynxNav" option, the
Left arrow key backs out from any screen, while the Right arrow key
selects. The plus and minus keys are no longer aliases for Right and
Left, but perform the same functions as in the traditional navigation
system.

Of special note is the space bar. In most screens, it functions as an
alias for PgDn; but in the letter window, it works as a combination
PgDn/Enter key, allowing you to page through an area with one key.

In the area list, the default view (selectable in the .mmailrc) is of
Subscribed areas only, or of Active areas (i.e., those with messages) if
the Subscribed areas are unknown. By pressing L, you can toggle between
Active, All, and Subscribed views. (Some formats, like plain QWK, don't
have any way to indicate subscribed areas. In other cases, you may have
received an abbreviated area list, so that the Subscribed and All views
are the same.) In all modes, areas with replies always appear, flagged
with an 'R' in the leftmost column.

In the letter list, only unread messages are displayed, by default; but
you can toggle this by pressing L. If there are any marked messages, L
first switches to a marked-only mode, then to all messages, then back to
unread-only. Also, the default mode -- unread or all -- can be set in
the .mmailrc.

Multiple sort modes are available in the packet and letter lists; you
can cycle through them by pressing '$'. The default sort modes are set
in the .mmailrc.

Options can be specified on the command line as well as in the .mmailrc.
Option names are the same as those which appear there, though they must
be prefaced by one or two dashes, and should not be followed by a colon.
There must be a space between the option name and the value; values
which include spaces must be quoted. All options must be specified
before any packet names or directories on the line. Finally, options
which take a filename or path should always include the full path. (This
is not, however, necessary for packet names.)

Packet names may be specified on the command line, bypassing the packet
menu. If multiple packets are named, they'll be opened sequentially. If
a directory is specified instead of a file, the packet window will by
opened on that directory, and no further items will be read from the
command line. 'T' in the packet menu may need clarification: it stamps
the highlighted file with the current date and time.

You can abort the program immediately from any screen with CTRL-X. You
won't be prompted to confirm the exit, but you will still be prompted to
save replies and pointers (unless autosaving is set). Note that if
you've specified multiple packets on the command line, this is the only
way to terminate the sequence prematurely.

You can obtain a temporary command shell anywhere by pressing CTRL-Z. In
the DOSish ports (MS-DOS, OS/2, Windows), it spawns a command shell, and
you return to MultiMail via the "exit" command. In Unix, it relies on
the shell to put MultiMail in the background; you return with "fg".
(This has always been available in the Unix versions; however, it won't
work if MultiMail wasn't launched from an interactive shell, or if the
shell doesn't support it.)


MOUSING
-------

MultiMail is mousable on many platforms: X, SDL, the Linux console (with
gpm), Windows, DOS and OS/2. (You can still use selection with X and
gpm, too; to select or paste, hold down the shift key.)

In each list window, button 1 highlights a line, or selects it (the same
as pressing Enter) if it's already highlighted. Double-click to select
it immediately. Click on the scrollbar to page up or down, or on the
line just above or below it to scroll a line at a time. In the packet,
area, and letter lists, click on the appropriate part of the window
title to change the sort or list type.

In the letter window, page up by clicking in the top half of the message
text, or down (and on to the next message) by clicking in the bottom
half (equivalent to the space bar). Scroll the message a single line up
or down by clicking on the status bars at top and bottom. The status
flags "Read" and "Marked" can be toggled by clicking on them; clicking
on "Save" saves, clicking on "Repl" starts a reply (followup; i.e., the
same as 'R'), and "Pvt" starts a private reply (email or netmail; i.e.,
same as 'N').

In text-entry windows, button 1 works the same as the Enter key; and the
dialog boxes work in the obvious way.

Button 3 backs out of any screen, equivalent to ESC.


SEARCHING
---------

A case-insensitive search function is available on all screens. Press
'/' to specify the text to look for, or '>' or '.' to repeat the last
search.

New searches (specified with '/') always start at the beginning of the
list or message. Repeat searches (with '>' or '.') start with the line
below the current one. You can take advantage of this to manually adjust
the starting point for the next search.

Searches started in the letter, area or packet lists allow the searches
to extend below the current list. "Full text" searches all the way
through the text of each message; "Headers" searches only the message
headers (the letter list), "Areas" only the area list, and "Pkt list"
only the packet list. So, a "Full text" search started from the packet
list will search every message in every packet (but only in the current
directory).

When scanning "Full text", the automatic setting of the "Read" marker is
disabled. However, if you find a search string in the header of a
message and then select it manually, the marker will be set. But if you
start scanning from the packet list, and exit the packet via a repeat
search, the last-read markers won't be saved.

Scans of "Headers" or "Full text" that start from the area list or
packet list will automatically expand the letter lists they descend
into. Similarly, scans that start at the packet list will expand the
area lists. Otherwise, if you're viewing the short list, that's all that
will be searched.

I hope the above makes some sense. :-) The searching functions are
difficult to explain, but easy to use.


FILTERING
---------

A new twist on searching, as of version 0.43, is filtering. This is
available in all of the list windows, but not the letter or ANSI viewer.
Unlike searching, it always applies only to the current list.

Press '|' to bring up the filter prompt, and specify the text to filter
on. To clear a filter, press '|', and then press return at a blank
filter prompt. (A string that's not found in the list will have the same
effect.) Press ESC to leave the filter as it was.

The list will now be limited to those items that contain the text you
entered, and that text will appear at the end of the window's title as a
reminder. The filter will be retained through lower levels, but will be
cleared by exiting to a higher level. Note that a search in, e.g., the
letter list will search only the message headers (and only those which
are visible in the list), and not the bodies.

When the filter is active in the letter list, the "All" option in the
Save menu will save only the items that match the filter. This can be
used as a quick alternative to marking and saving. You can also combine
filtering and marking.

Changing modes and sort types will not clear the filter. A search in a
filtered list will search only the items that match the filter.


OFFLINE CONFIGURATION
---------------------

Offline config is limited to subscribe (add) and unsubscribe (drop)
functions. The Blue Wave, OPX, OMEN, QWKE, and QWK Add/Drop (with
DOOR.ID) methods are supported. (The QMAIL "CONFIG" method is not
supported.) Offline config is not available in SOUP mode.

In the area list, press 'U' or 'Del' to unsubscribe from the highlighted
area. To subscribe to a new conference, first expand the list ('L'),
then highlight the appropriate area and press 'S' or 'Ins'. Dropped
areas are marked with a minus sign ('-') in the first column; added
areas with a plus ('+'). In the expanded area list, already-subscribed
areas are marked with an asterisk ('*'). (This and also applies to the
little area list. With plain QWK packets, the asterisk should not be
relied upon; other areas may also be subscribed.) Added or dropped areas
are highlighted in the "Area_Reply" color. Yeah, I'll have to change
that name now. ;-)

Pressing 'S' on an area marked with '-', or 'U' on an area marked '+'
turns the flag off again.

In Blue Wave, OPX, OMEN or QWKE mode, the list of added and dropped
areas is read back in when the reply packet is reopened. If the reply
packet has already been uploaded, and you're reading a packet with the
altered area list, this is benign. If it's an older packet, you can
alter the list before uploading, as with reply messages. In QWK Add/Drop
mode, the changed area flags are converted to reply messages when the
reply packet is saved. Note: Adding or dropping areas sets the "unsaved
replies" flag, like entering a reply message, but does not invoke
automatic reply packet saving until you exit the packet.

Unfortunately, the OMEN mode has not actually been tested; but I believe
it conforms to the specs. Reports welcome.


HIDDEN LINES AND ROT13
----------------------

In the letter window, you can toggle viewing of Fidonet "hidden" lines
(marked with a CTRL-A in the first position) by pressing 'x'. The lines
are shown as part of the text, but in a different color. In Internet
email and Usenet areas, the full headers of the messages are available
in the same way (if provided in the packet -- generally, full headers
are available in SOUP, and partial extra headers in Blue Wave).

Pressing 'd' toggles rot13 encoding, the crude "encryption" method used
for spoiler warnings and such, primarily on Usenet.


ANSI VIEWER
-----------

If a message contains ANSI color codes, you may be able to view it as
originally intended by activating the ANSI viewer. Press 'v' to start
it. Press 'q' to leave the ANSI viewer; the navigation keys are the same
as in the mail-reading window.

The ANSI viewer includes support for animation. While within the ANSI
viewer, press 'v' again to animate the picture. Press any key to abort
the animation.

The ANSI viewer is also used to display the new files list and
bulletins, if any are present.

New in version 0.43 is support for the '@' color codes used by PCBoard
and Wildcat. This is on by default in the ANSI viewer, but it can be
toggled to strip the codes, or pass them through untranslated, by
pressing '@'.

As of version 0.46, the ANSI viewer also includes limited support for
AVATAR (level 0) and BSAVE (text only) screens. These can be toggled via
CTRL-V and CTRL-B, respectively.


CHARACTER SETS
--------------

MultiMail supports automatic translation between two character sets: the
IBM PC set (Code Page 437), and Latin-1 (ISO 8859-1). Messages can be in
either character set; the set is determined by the area attributes --
Internet and Usenet areas default to Latin-1, while all others default
to IBM -- and by a CHRS or CHARSET kludge, if one is present. OMEN
packets indicate their character set in the INFOxy.BBS file. MultiMail
translates when displaying messages and creating replies.

The Unix versions of MultiMail assume that the console uses Latin-1,
while the DOSish versions (DOS, OS/2, and Windows) assume the IBM PC
set. You can override this via the .mmailrc option "Charset", or on a
temporary basis by pressing 'c'.

You can also use a different character set by disabling the conversion
in MultiMail, and letting your terminal handle it. For SOUP packets, and
for Internet or Usenet areas in other packets, everything will be passed
through unchanged if you set MultiMail to "Latin-1". For most other
packet types, setting MultiMail to "CP437" will have the same effect.

Beginning with version 0.33, a new character set variable is available:
"outCharset". This is a string which MultiMail puts into the MIME
identifier lines in SOUP replies if the text includes 8-bit characters.
It's also used for the pseudo-QP headers which are generated under the
same conditions; and when displaying such headers, MultiMail only
converts text back to 8-bit if the character set matches. The default is
"iso-8859-1".

By default, if a header line in a SOUP reply contains 8-bit characters,
MultiMail now writes it out with RFC 2047 (pseudo-QP) encoding. You can
disable this for mail and/or news replies via the "UseQPMailHead" and
"UseQPNewsHead" options, though I don't recommend it. The bodies can
also be encoded in quoted-printable; this is now on by default for mail,
and off for news. The options "UseQPMail" and "UseQPNews" toggle QP
encoding. (The headers and bodies of received messages will still be
converted to 8-bit.)

QP decoding is temporarily disabled when you toggle the display of
hidden lines ('X') in the letter window, so that you can see the raw
text of the message.


ADDRESS BOOK
------------

The address book in MultiMail is intended primarily for use with
Fido-style Netmail or Internet email areas, in those packet types which
support these. When entering a message (other than a reply) into such an
area, the address book comes up automatically. It's also possible to use
the name portion of an address from the address book even when
Fido/Internet addressing isn't available, by starting a new message via
CTRL-E instead of 'E'.

You can pull up the address book from most screens by pressing 'A',
which allows you to browse or edit the list. While reading in the letter
window, you can grab the current "From:" address by invoking the address
book and pressing 'L'.


TAGLINE WINDOW
--------------

From most screens, you can pull up the tagline window to browse or edit
the list by pressing CTRL-T. As of version 0.43, you can toggle sorting
of the taglines by pressing '$' or 'S'.


REPLY SPLITTING
---------------

Replies may be split, either automatically, or manually via CTRL-B in
the reply area. For automatic splitting, the default maximum number of
lines per part is set in the .mmailrc. The split occurs whenever the
reply packet is saved. This allows you to defer the split and still
re-edit the whole reply as one. However, with autosave on, the split
will occur immediately after entering a reply (because the save does,
too). Setting MaxLines in the .mmailrc to 0 disables automatic
splitting; manual splitting is still allowed. Attempts to split at less
than 20 lines are assumed to be mistakes and are ignored.


ENVIRONMENT
-----------

MultiMail uses the HOME or MMAIL environment variable to find its
configuration file, .mmailrc; and EDITOR for the default editor. MMAIL
takes precedence over HOME if it's defined. If neither is defined, the
startup directory is used.

The use of EDITOR can be overridden in .mmailrc; however, environment
variables can't be used within .mmailrc.

You should also make sure that your time zone is set correctly. On many
systems, that means setting the TZ environment variable. A typical value
for this variable is of the form "EST5EDT" (that one's for the east
coast of the U.S.A.).


FILES
-----

The only hardwired file is the configuration file: `.mmailrc`
(`mmail.rc` in DOS, OS/2 or Windows). It's used to specify the pathnames
to MultiMail's other files, and the command lines for external programs
(the editor and the archivers).

By default, the other files are placed in the MultiMail home directory
($HOME/mmail or $MMAIL). Directories specified in the .mmailrc are
created automatically; the default Unix values are shown here:

`~/mmail`
  To store the tagline file, netmail addressbook, etc.

`taglines`
  A plain text file, one tagline per line.

`addressbook` (address.bk in DOS, OS/2 or Windows)
  A list of names and corresponding Fido netmail or Internet email
  addresses. Note that Internet addresses are prefaced with an 'I'.

`colors`
  Specifies the colors to use. (See [COLORS.md].)

`~/mmail/down`
  To store the packets as they came from the bbs.

`~/mmail/up`
  To store the reply packets which you have to upload to the bbs.

`~/mmail/save`
  The default directory for saving messages.


CONFIG FILE
-----------

The config file (see above) is a plain text file with a series of
values, one per line, in the form "KeyWord: Value". The case of the
keywords is not signifigant. Additional, comment lines may be present,
starting with '#'; you can remove these or add your own. (But note that
the comments are replaced by the defaults when you upgrade to a new
version.) If any of the keywords are missing, default values will be
used.

As of version 0.41, any of these keywords except "Version" may also be
specified on the command line. Command-line options take precedence over
those in the config file, but their effect is not guaranteed -- some
internal pathnames are initialized before the command line is read, for
example.

Here are the keywords and their functions:

`Version`
  Specifies the version of MultiMail which last updated the file. This
  is used to check whether the file should be updated and the "new
  version" prompt displayed. Note that old values are preserved when the
  file is updated; the update merely adds any keywords that are new.
  This keyword is also used in the colors file.

`UserName`
  Your name in plain text, e.g., "UserName: William McBrine". This is
  used together with InetAddr to create a default "From:" line for SOUP
  replies; and by itself in OMEN for display purposes (the actual From
  name is set on upload), and for matching personal messages.

`InetAddr`
  Your Internet email address, e.g., "InetAddr: wmcbrine @ gmail.com".
  This is combined with the UserName in the form "UserName \<InetAddr\>"
  ("William McBrine \<wmcbrine @ gmail.com\>") to create a default
  "From:" line for SOUP replies. Note that if neither value is
  specified, and nothing is typed manually into the From: field when
  creating a message, no From: line will be generated -- which is
  perfectly acceptable to at least some SOUP programs, like UQWK.

`QuoteHead, InetQuote`
  These strings are placed at the beginning of the quoted text when
  replying in normal or Internet/Usenet areas, respectively. (The
  distinction is made because the quoting conventions for BBSes and the
  Internet are different.) Replaceable parameters are indicated with a
  '%' character, as follows:

    %f = "From" in original message  
    %t = To  
    %d = Date (of original message)  
    %s = Subject  
    %a = Area  
    %n = newline (for multi-line headers)  
    %% = insert an actual percent character

  Note that you can't put white space at the start of one of these
  strings (it will be eaten by the config parser), but you can get
  around that by putting a newline first.

`mmHomeDir`
  MultiMail's home directory.

`TempDir`
  This is the directory where MultiMail puts its temporary files -- by
  default, as of 0.45, the same as mmHomeDir. The files are actually
  created within a subdirectory of this directory; the subdirectory is
  named "workNNNN", where NNNN is a random number (checked against any
  existing files or directories before being created).

`signature`
  Path to optional signature file, which should be a simple text file.
  If specified, it will be appended to every message you write. You
  should give the full path, not just the name.

`editor`
  The editor MultiMail uses for replies, along with any command-line
  options. This may also be a good place to insert spell-checkers, etc.,
  by specifying a batch file here. Note that the default value is just
  the editor that's (almost) guaranteed to be available, for a given OS
  (although the Unix "EDITOR" environment variable is checked first),
  and is in no way a preferred editor; you can and should change it.

`PacketDir`
  Default packet directory.

`ReplyDir`
  Default reply packet directory.

`SaveDir`
  Default directory for saved messages.

`AddressBook`
  Path and filename of the address book. (You might change this to share
  it with another installation, but basically this keyword isn't too
  useful.)

`TaglineFile`
  Path and filename of the tagline file. This could be altered from a
  batch file to swap between different sets of taglines. (But note that
  this value is only read at startup.) You could also share taglines
  with another program, but be careful with that; MultiMail truncates
  the lines at 76 characters.

`ColorFile`
  Path and filename of the colors file. See [COLORS.md].

`UseColors`
  Yes/No. This governs whether color is used, or monochrome. When colors
  are disabled, the terminal's default foreground and background colors
  are used. It's also a crude way to implement transparency (the only
  way, if you're not using ncurses or PDCurses/SDL) -- the entire
  background will be transparent when using an appropriate terminal.

`Transparency`
  Yes/No. Only available in ncurses or PDCurses for SDL. (The option
  will appear, but not work, in non-ncurses, non-PDCurses platforms.)
  When this is set to Yes, all areas where the background color is the
  same as the background color set in the "Main_Back" line, in the
  colors file, are instead set to the default background color, and thus
  become transparent areas in those terminal programs, like Eterm and
  Gnome Terminal, that support this.

`BackFill`
  Yes/No. Normally the background area is filled with a checkerboard
  pattern (ACS_BOARD characters, in curses terms). You can disable that
  here, leaving those areas as flat background color. This option is
  intended mostly to make transparency more effective, but it might help
  with any color scheme.

`*UncompressCommand, *CompressCommand`
  Command lines (program name, options, and optionally the path) for the
  archivers to compress and uncompress packets and reply packets. ZIP,
  ARJ, RAR, LHA and tar/gzip are recognized. The "unknown" values are a
  catch-all, attempted for anything that's not recognized as one of the
  other four types; if you have to deal with ARC or ZOO files, you might
  define the archiver for them here.

`PacketSort`
  The packet list can be sorted either in inverse order of packet date
  and time (the newest at the top), or in alphabetical order by
  filename. "Time" specifies the former, and "Name" the latter.
  (Actually only the first letter is checked, and case is not
  signifigant. This applies to the other keywords of this type (the kind
  that have a fixed set of values to choose from) as well.) The sort
  type specified here is only the default, and can be toggled from the
  packet window by pressing '$'.

`AreaMode`
  The default mode for the area list: "All", "Subscribed", or "Active".
  This is the mode that will be used on first opening a packet, but it
  can be changed by pressing L while in the area list or little area
  list. For a description of the modes, see USAGE.

`LetterSort`
  The sort used by default in the letter list. Can be "Subject"
  (subjects sorted alphabetically, with a case-insensitive compare),
  "Number" (sorted by message number), "From" or "To". (This can be
  overridden, as in the packet list.)

`LetterMode`
  The default mode for the letter list: "All" or "Unread". This is the
  mode used on first opening an area; it can be toggled by pressing L.
  (The Marked view is also available in the letter list, but cannot be
  set as the default here.)

`ClockMode`
  The display mode for the clock in the upper right corner of the letter
  window: "Time" (of day), "Elapsed" (since MultiMail started running),
  or "Off".

`Charset`
  The character set that the console is assumed to use. Either "CP437"
  (code page 437, the U.S. standard for the IBM PC and clones) or
  "Latin-1" (aka ISO-8859-1, the standard for most other systems). Note
  that the character set of messages is determined separately (q.v.).

`UseTaglines`
  Yes/No. If no, the tagline window is not displayed at all when
  composing a message.

`AutoSaveReplies`
  Yes/No. If yes, the reply packet is saved automatically -- the
  equivalent of pressing F2, but without a confirmation prompt --
  whenever the contents of the reply area are changed. This can be
  convenient, and even a safety feature if your power supply is
  irregular, but it provides less opportunity to take back a change
  (like deleting a message). If no, you're prompted whether to save the
  changes on exiting the packet. Note that if you say no to that prompt,
  nothing that you wrote during that session will be saved (unless you
  saved it manually with F2).

`StripSoftCR`
  Yes/No. Some messages on Fido-type networks contain spurious instances
  of character 141, which appears as an accented 'i' in code page 437.
  These are really so-called "soft returns", where the message was
  wrapped when composing it, but not indicating a paragraph break.
  Unfortunately, the character can also appear legitimately as that
  accented 'i', so this option defaults to no. It can be toggled
  temporarily via the 'I' key in the letter window, and it doesn't apply
  to messages in the Latin-1 character set. This is now applied only in
  Blue Wave mode.

`BeepOnPers`
  Yes/No. If yes, MultiMail beeps when you open a message addressed to
  or from yourself in the letter window. (These are the same messages
  which are highlighted in the letter list.)

`UseLynxNav`
  Yes/No. See the description under USAGE.

`ReOnReplies`
  Yes/No. By popular demand. :-) Setting this to "No" will disable the
  automatic prefixing of "Re: " to the Subject when replying -- except
  in areas flagged as Internet email or Usenet, where this is the
  standard, and is still upheld.

`QuoteWrapCols`
  Numeric. The right margin for quoted material in replies (including
  the quote indicator).

`MaxLines`
  Numeric. See the description under REPLY SPLITTING.

`outCharset`
  String. See the description under CHARACTER SETS.

`UseQPMailHead`
  Yes/No. Controls the use of RFC 2047 encoding in outgoing mail
  headers.

`UseQPNewsHead`
  Yes/No. Controls the use of RFC 2047 encoding in outgoing news
  headers.

`UseQPMail`
  Yes/No. Controls the use of quoted-printable encoding in outgoing
  mail.

`UseQPNews`
  Yes/No. Controls the use of quoted-printable encoding in outgoing
  news.

`ExpertMode`
  Yes/No. If set to No, the onscreen help menus are not shown; instead,
  the space is used to extend the size of info windows by a few lines.

`IgnoreNDX`
  Yes/No. This option applies only to QWK packets. If set to yes, the
  *.NDX files are always ignored, in favor of the "new" indexing method
  that depends only on MESSAGES.DAT. This method is slightly slower than
  the *.NDX-based indexing method (though the delay is dwarfed by packet
  decompression time), but the most common problem with QWK packets is
  corrupt *.NDX files. MultiMail now recognizes some cases where the
  *.NDX files are corrupt and switches automatically, but it doesn't
  catch them all.


UPGRADING
---------

The basic upgrade procedure is to simply copy the new executable over
the old one. No other files are needed. When you run a new version of
MultiMail for the first time, it automatically updates your .mmailrc and
ColorFile with any new keywords. (Old keywords, and the values you've
set for them, are preserved. However, comments are lost.)


NOTES
-----

Unlike the other archive types, tar/gzip recompresses the entire packet
when updating the .red flags, so it can be a bit slow. Also, the
supplied command lines assume GNU tar, which has gzip built-in.
Separated gunzip/tar and tar/gzip command lines are possible, but would
require a (simple) external script. MultiMail only checks for the gzip
signature, and does not actually verify that the gzipped file is a tar
file.

OPX reply packets are always created with a .rep extension, which
differs from the behavior of some other readers. If you switch from QWK
packets to OPX packets on the same board, MultiMail will _not_ open an
old QWK .rep in OPX mode, nor vice versa. (It will try, and will
terminate with "Error opening reply packet".)

SOUP reply packets are created with the name "basename.rep", where
basename is the part of the original packet name before the first
period. (Unlike other formats, there's no actual standard for this in
SOUP, but this seems to be the most common form among the SOUP readers I
surveyed.) Also, not that I expect anyone to try this, but currently
MultiMail is only able to read reply packets generated by other SOUP
readers if the replies are in 'b' or 'B' mode, and are one to a file
within the packet. Most readers meet the first criterion, but some of
them batch all mail and news replies into a single file for each type.

When re-editing a reply, it gets pushed to end of the list of replies.

The R)ename function in the packet window can also be used to move files
between directories; however, the destination filename must still be
specified along with the path.

If you're using the XCurses (PDCurses) version, and your editor isn't an
X app, it will work better if you set MultiMail's "editor" keyword to
"xterm -e filename" (instead of just "filename"). I decided not to do
this automatically because someone might actually use it with an X
editor.

Editing and deletion of old replies are available through the REPLY
area, which always appears at the top of the area list. This differs
from Blue Wave and some other readers.

The Escape key works to back out from most screens, but after you press
it, you'll have to wait a bit for it to be sensed (with ncurses; not
true with PDCurses).

Only Blue Wave style taglines (beginning with "...") are recognized by
the tagline stealer. The tagline must be visible on the screen to be
taken.

Netmail only works in Blue Wave, OMEN and OPX modes, and is still
slightly limited. Netmail from points includes the point address.
Internet email is available in Blue Wave and OPX modes, for those doors
that support it, and in SOUP mode, using the same interface as Fido
netmail.


AUTHORS
-------

MultiMail was originally developed under Linux by Kolossvary Tamas and
Toth Istvan. John Zero was the maintainer for versions 0.2 through 0.6;
since version 0.7, the maintainer is [William McBrine].

Additional code has been contributed by Peter Krefting, Mark D. Rejhon,
Ingo Brueckl, Robert Vukovic and Mark Crispin.


BUGS AND KNOWN PROBLEMS
-----------------------

SOUP area type 'M' is not recognized. I have yet to find a program that
can generate one. :-)

The ANSI viewer eats a lot less memory than it used to, but it can still
be a problem. (Each character/attribute pair takes up four bytes in
memory. But lines which have the same attribute throughout are stored as
plain text.)

The new file list and bulletin viewer is a hack.

If you find any bugs, please write to me.


[COLORS.md]: colors/COLORS.md
[William McBrine]: https://wmcbrine.com/
