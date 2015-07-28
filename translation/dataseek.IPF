:userdoc.
:docprof toc=12345.
:title.DataSeeker

.* htmlsource = dataseek.html
:h1 res=1 x=left y=top width=100% height=20% group=1 rules=border scroll=vertical titlebar=none.DataSeeker

:link auto reftype=hd res=6.
:font facename=default size=0x0.
:ul compact.
:li.:link reftype=hd res=2.License agreement:elink.
:li.:link reftype=hd res=3.Introduction:elink.
:li.:link reftype=hd res=4.What is new:elink.
:li.:link reftype=hd res=5.Installation:elink.
:li.:link reftype=hd res=6.Features and settings:elink.
:li.:link reftype=hd res=17.Planned features:elink.
:li.:link reftype=hd res=18.Registration:elink.
:eul.

.* License agreement
:h1 res=2 x=left y=bottom width=100% height=80% group=2 rules=border scroll=vertical titlebar=none.License agreement
:i1 id=license.License agreement
:i1 id=contact.Author address
:font facename='Helv' size=20x12.
License agreement
:font facename=default size=0x0.
:ul compact.
.br
:li.You are allowed to use this program as long you observe the following rules&per.
:li.The program may be installed on as many
machines as you like&per.
:li. You are free to distribute the program provided that you
include all the files in the original archive without any modification&per.
:li. You are not allowed to sell the program, but can charge a reasonable
amount to cover the cost of the distribution media&per.
:li.Under no circumstances will be the authors liable for any loss
or damage that may be derived from the use of the program&per.
:li.You are not allowed to bitch about the authors. You got this piece of
software for free and there is no guarantee for any kind of feature or
smooth operation&per.
:li.Modified BSD License applies as stated in COPYING. Portions of code subject to Sybase Open Watcom Public License.
&per.
:li.ACDataSeeker / DataSeeker is the copyrighted property of Alessandro Felice
Cantatore (Bitonto, Bari, Italy) and Andreas Buchinger (Austria)&per. :eul.
.br
:hp2.The use of the program implies acceptance of the conditions stated above!:ehp2.
:p.
Alessandro Cantatore, Andreas Buchinger
.br
:link reftype=fn refid=2.andi.os2.software@gmx.net:elink.
.br
:link reftype=fn refid=3.(acantatore&atsign.tin&per.it):elink.
.br
:link reftype=fn refid=4.http&colon.&slr.&slr.dataseeker.netlabs.org&slr.en&slr.site&slr.index.xml:elink.

.* Introduction
:h1 res=3 x=left y=bottom width=100% height=80% group=2 rules=border scroll=vertical titlebar=none.Introduction
:i1 id=intro.Introduction
:font facename='Helv' size=20x12.
Introduction
:font facename=default size=0x0.
:p.
As a forgetful developer, I tend to use quite often the old &cdq.Seek and Scan
Files&cdq. (PMSEEK&per.EXE) included with OS&slr.2, to find text in the system
header files or other source files, or just to find some old news message I
had saved a few years ago&per.
.br
Unfortunately PMSEEK&per.EXE has serious flaws and using it is not always a pleasent experience, so I decided to code a more functional and feature rich program&per.
.br
:p.
The program is currently quite basic as it is still lacks some essential
PMSEEK features like the ability to drag files to an Enhanced editor window
or to edit the selected file&per. On the other hand it already has some little
extra option and a more attractive (I hope) interface&per.
.br
:p.
Even if the program is far from complete, I decided to make it available
in its current stage, as I want to start to get some feedback so I can
easily modify it, while working on implementing the still missing features&per.
.br
:p.
:hp2.Although this is the help file for version 1&per.00 main parts of the help system are not updated since v0&per.46&per. This is cause I (Andreas Buchinger)
do not like/know this &per.ipf stuff&per. Another reason is my limited time&per. DataSeeker is now open source and hosted at netlabs&per.org&per.
http&colon.&slr.&slr.dataseeker.netlabs.org&slr.en&slr.site&slr.index.xml. Feel free to contribute&per. Sayed that it's obvious
some parts of this help is not accurate any more&per. See &csq.What&csq.s new&csq. section for latest infos&per.
:ehp2.
.* What&csq.s new
:h1 res=4 x=left y=bottom width=100% height=80% group=2 rules=border scroll=vertical titlebar=none.What&csq.s new
:i1 id=new.What's new
:font facename='Helv' size=20x12.
What&csq.s new
:font facename=default size=0x0.
:ul compact.
.br
:li.Version 1&per.14
:ul compact.
:li.fixed Trap with minimize/maximize for release builds
:li.fixed Exceptq logs now have valid symbols
:li.changed license to 'Modified BSD License' as described in COPYING
:eul.
.br
:li.Version 1&per.13
:ul compact.
:li.fixed icons (bug introduced with some 1.0x version)
:eul.
.br
:li.Version 1&per.12
:ul compact.
:li.added some usefull accelerators, F2 to open command line, F4 to edit file, F3 or ENTER to start searching
:li.changed cmd shell setting now supports parameter passing too
.br
f.i. 'p&colon.\4os2\4os2.exe &atsign.p&colon.\4os2\4os2.ini "p&colon.\4os2\4start.cmd &amp. mode 81,101"'
:li.fixed trap with DEBUG build and no search result
:eul.
.br
:li.Version 1&per.11
:ul compact.
:li.fixed display Cyrillic characters
:eul.
.br
:li.Version 1&per.10
:ul compact.
:li.added FOC - File Open Container for selecting search file/path
:li.added Dutch version (including .hlp, thanks Jacques van Leeuwen)
:li.added Russian version (thanks Eugene Gorbunoff and Andrea). Though not sure if it works cause on my machine I see no cyrillic characters
:li.added foc.nls with corrected german translation
:li.added new option to find directory names too when searching for files
:li.reworked Drives dialog, now updates drives on the fly even when dialog is visible, size now depends on number of available drives
:li.cosmetic changes in product info dialog
:li.changed ini file format, sorry old history lists can not be imported
:li.changed exception handling from xwp code to more powerful exceptq v7.1
:li.fixed wrong text in 'Drives' button as long as not all drives are available
:li.fixed trap when maximize/minimize
:li.fixed wrong calculation for decision if moving window is required at startup
:eul.
.br
:li.Version 1&per.01
:ul compact.
:li.added Spanish version (thanks Salvador Parra Camacho)
:li.added bldlevel string
:li.changed memory allocation grow step size from 512kB to 1MB
:li.fixed translation for some strings was missing
:eul.
.br
:li.Version 1&per.00
:ul compact.
:li.added National Language Support via (eco)lange - Requirements: eCo
Software runtime (base) -- http&colon.&slr.&slr.ecomstation&per.ru&slr.ecosoft&slr.runtime&per.php
:li.added Pop-up menus
:li.<ESC> now closes list boxes (Combo-Boxes) and Drives window
:li.reenabled CTRL-D / CTRL-SHFT-D to delete items in Combo-Boxes
:li.new drive type RSJ '*' (CDWFS)
:li.changed Settings dialog
:li.double click on found files now opens file instead of edit file
:li.fixed crash when pressing <DEL> in history field
:li.fixed position/size restore after WPS restart
:li.fixed copy filename (Selected - edit/open...) bug after changing sort order
:li.fixed ALT-F4 / ALT-F7 behaviour immediately at startup (ticket #3)
:li.fixed crash when pressing <F1> while menu is open
:li.fixed refresh of WinDrives dialog (with removed and erroneous drives)
:eul.
.br
:li.Version 0&per.54
 :ul compact.
:li.fixed crash when drive is specified and only 1 file search criteria
introduced with 0&per.53
:li.fixed - no drives but backslash defined leads to wrong double \\

:eul.
:li.Version 0&per.53
.br
Sorry I've found so much memory related problems in 0&per.52 that we need another beta release. Due to the newly implemented exception handler stuff,
from now on it should be easier to track down problems. In case of a crash look into DataSeek.log at the root of your boot drive.
:ul compact.
:li.increased max. | (OR) operations on 'files to search' from 2 to 50
:li.max. characters in list boxes increased to 512
:li.linked with IBMCPP3.65 multi-threading libraries
:li.exception handling with xwphelpers functions - create a log file (helpers36.lib)
:li.DosCreateThread changed to _beginthread
:li.switch to standard library memory management
:li.list handling - max. number of iterations added
:li.various bug fixes which destroyed heap and so produced weird behaviour (beside others - crash when switching to maximum size, thanks Martin)
:li.cleaned up 'LFS capable system detection' code
:li.info area added (error display)
:li.tested with more than 15 drives and more than 16.000 matching files
:li.fixed options safe routine
:li.work around for ComboBox bug with more than 256 chars per entry
:eul.

:li.Version 0&per.52
.br
This is a major step towards 1.0. There are only a few cosmetic changes left I
like to do before I think 1.0 is ready. As the Large File support was more
complex than expected, I like to make this version public for feedback if
I've broken something. So give feedback
:ul compact.
:li.Added large file (>= 2GB) support available since Warp Server for e-business
:li.Max. line length for text searches is now 5MB (text search in binary files)
:li.Expand some limits to work with big files (tested with files up to 9.5GB and more than 30000 text matches)
:li.Limited testing has been done with Warp3. Mostly to ensure that the 'Large File' wrapper fuctions works with older OS/2 versions
:li.fixed crash when try to start editor and no .ini entry is set for it
:eul.
:li.Version 0&per.51
:ul compact.
:li.check on start if window is in visible area and move it if not (f.i.
if it is in another xpager window)
:li.Drive selection window which is used when no drive specifier is entered in
 'Files to search for'
:li.fixed ticket #2 - search options (recursive, case...) did not work without
 close and reopen DataSeeker
:eul.
:li.Version 0&per.50
:ul compact.
:li.leading blanks in directory specification are now ignored (This may
lead to a problem, when you want to search in a directory which name starts with
a blank. In this case, start searching with the parent directory instead.)
:li.prevent automatic selection of first entry in 'Files found' at the end
of search when user has scrolled or selected a file during search
:li.removed another few '&colon.' in strings. Hope I've found them all
finally ;-)
:li.cosmetic changes on Files found... and Elapsed time strings
:eul.
:li.Version 0&per.49
:ul compact.
:li.enclose filename with " before opening in MED or VS (support file
names with blanks)
:li.fixed <CTRL><P> (upper case) accelerator which did not work
:li.added <CTRL><C> - copies filename enclosed with " to clipboard
:li.added <CTRL><SHIFT><C> - copies selected line to clipboard
:eul.
:li.Version 0&per.48
:ul compact.
:li.disable 'Selected' menu item on startup (crash when no file is
selected)
:li.reenabled CTRL-E and CTRL-P accelerators
:li.added new flag to appStart to distinguish between editor/cli and WinSwitchToProgram
  (window list pops up when starting editor, probably introduced with 0.47)
:li.removed scroll bars on main window
:li.added '...' to Settings menu items and removed '&colon.' on various
strings
:li.removed sub menus from 'View'
:li.reduced size for Files/Text to search drop down boxes to not overlap
Search button and do not sort entries alphapeticaly any more
:li.calling EPM now with 'postme'
:eul.
:li.Version 0&per.47
:ul compact.
:li.first build from Andreas Buchinger. Changed GUI to build with
URE. Some changes to the menus to make it consistent with usual OS/2 look and feel
:li.fixed crash when no editor or cmd shell is found in ini file
:li.fixed crash when no editor or cmd shell is entered in settings dialog
:li.added support for MED and VS (VisualSlickEdit) as editors (beside EPM)
:li.select first found text entry at end of search to start editor with
approbiate line even when double clicking on the file name
:li.when opening a cmd shell, switch to it (put focus on the cli window)
:li.add '*' to file search string if it is a directory (simple
check only - end with \ or : )
:li.added scroll bars for Files/Text found list boxes
:eul.
:li.Version 0&per.45-46
:ul compact.
:li.minor bug fixes
:eul.
:li.Version 0&per.44
:ul compact.
:li.opening files via a double click on the entry in the listbox did not work
if the file name contained spaces&per.
:li.searching on files containing NULs (i&per.e&per. characters with an ASCII value
of 0) stopped at the first NUL&per. Now, in the search buffer, all NULs are
replaced with spaces and so the whole file is searched&per.
:li.the product information dialog has been added&per.
:li.most help menu items now work&per.
:eul.
:li.Version 0&per.43
:ul compact.
:li.if a folder is dragged on the program icon, the folder path is inserted in the :hp1.Files to search for&colon. :ehp1. field and the search pattern
:font facename='System VIO' size=12x5.
&cdq.&bsl.&asterisk.&cdq.
:font facename=default size=0x0.
is automatically appended to the path&per.
:li.some features of the :hp1.Selected :ehp1. menu item (Edit, Open, Settings, Folder, Command line) have now been implemented&per.
They can be performed either via the :hp1.Selected :ehp1. menu or via the mouse
(double click to edit the selected file in a new window, Shift + double click
to edit the file in an already opened EPM window, Ctrl + double click to
open the folder containing the file, Alt + double click to open the file
settings notebook,)&per.
:li.the View and Options menu attributes have been changed so that
the menu is not dismissed once an option has been set (so it is easier to
change multiple options)&per.
:li.the View menu has been reorganized&per.
:li.to change the font of the dialog you can now drop a font on any caption window (i&per.e&per. the text :hp1.Files to search for&colon.:ehp1., :hp1.Text to search for (if any)&colon. :ehp1., etc&per.)&per.:li.when
a search operation is performed the wait pointer is no longer used if the
mouse moves over the window border so now it is easier to resize the window
while searching&per.
:li.the problem of the scrambled window layout (on first program execution) has been finally solved&per.
:eul.
:li.Version 0&per.42&colon.
:ul compact.
:li. the wildcard matching routine was buggy&per. It has now been replaced with a new correct and very fast algorithm&per.
:li.the program settings were not saved if the program was closed
via the window list or via system shutdown&per.
:li.the algorithm used for the case insensitive, and boolean searches have been completely rewritten as they were not working properly&per.
:li.in some systems, on the first execution, the program window has an incorrect size&per. I failed to reproduce the problem on my machine, anyway made some little modifications which I hope will solve that problem&per.
:li.an elapsed time indicator showing the time employed to perform a search operation has been addded&per.
:li.some notes about
how to use metacharacters (alsko known as wildcards or global characters)
in the  names of the files to search for have been added&per.
:eul.
:li. Version 0&per.41&colon.
:ul compact.
:li. a serious memory leak problem has been corrected&per.
:eul.
:eul.

.* Installation
:h1 res=5 x=left y=bottom width=100% height=80% group=2 rules=border scroll=vertical titlebar=none.Installation
:i1 id=install.Installation
:font facename='Helv' size=20x12.
Installation
:font facename=default size=0x0.
:p.
At the moment the program just consists in a single executable and
since it does not use any entry in the CONFIG&per.SYS or in the system INI
files, you can copy the file anywhere you like and use the WPS to create
a program object on your desktop or where you prefer&per.
.br
The program, once started will create an initialization file in the executable path&per.
:p.
To uninstall the program just delete DATASEEK&per.EXE, DATASEEK&per.INI and DATASEEK&per.HLP&per.

.* Features and settings
:h1 res=6 x=left y=60% width=100% height=20% group=2 scroll=vertical titlebar=none rules=border.Features and settings
:link auto reftype=hd res=7.
:font facename=default size=0x0.
:ul compact.
:li.:link reftype=hd res=7.Features:elink.
:li.:link reftype=hd res=8.Main window:elink.
:li.:link reftype=hd res=9.Menu :hp1.File:ehp1.:elink.
:li.:link reftype=hd res=10.Menu :hp1.Selected:ehp1.:elink.
:li.:link reftype=hd res=11.Menu :hp1.View:ehp1.:elink.
:li.:link reftype=hd res=12.Menu :hp1.Options:ehp1.:elink.
:li.:link reftype=hd res=13.Menu :hp1.Help:ehp1.:elink.
:li.:link reftype=hd res=14.Mouse features:elink.
:li.:link reftype=hd res=15.Startup parameters:elink.
:li.:link reftype=hd res=16.Notes on metacharacters usage:elink.
:eul.

.* Features
:h1 res=7 x=left y=bottom width=100% height=60% group=3 scroll=vertical titlebar=none rules=border.Features
:i1 id=features.Features
:i1 id=settingsdlg.Settings dialog
:font facename='Helv' size=20x12.
Features
:font facename=default size=0x0.
:p.
The current working features are&colon.
:ul compact.
:li.:hp2.Multithreading:ehp2.
.br
All disk operations (scanning the disk, reading files, etc&per.) are performed
via a separate thread&per.
.br
:li.:hp2.No drive scan on startup:ehp2.
.br
Unlikely PMSEEK&per.EXE you will not see any  &cdq.Drive not
ready&cdq. message when starting multiple instances of DataSeeker&per.
.br
:li.:hp2.Multiple file names:ehp2.
.br
You can specify multiple file names in the :hp1.Files to search for&colon.:ehp1.
 field using the an OR symbol (a vertical bar &csq. | &csq.) to separate the
file masks&per.
.br
For instance&colon.
:xmp.:font facename='System VIO' size=12x5.
D&colon.&bsl.&asterisk.TXT|D&colon.&bsl.&asterisk.HTM&asterisk.|E&colon.&bsl.&asterisk.TXT|E&colon.&bsl.&asterisk.HTM&asterisk.
:exmp.:font facename=default size=0x0.
will find all the files in the drives D&colon. and E&colon. ending with
the TXT, HTM or HTML (and other extension starting with HTM)&per.
.br
:li.:hp2.Multiple text strings
.br
:ehp2.You can specify multiple text strings separating them with
the OR symbol ( | ) or with the AND symbol ( &amp. )&per. You can also use
the escape character &csq. &caret. &csq. to make the program interpret  |, &amp.
and &caret. as real characters rather than symbol&per.
.br
For instance if you specify int the :hp1.Text to search for (if any)&colon. :ehp1.field &colon.
:xmp.:font facename='System VIO' size=12x5.
PFNWP|MIA_CHECKED|MIS_SINGLE
:exmp.:font facename=default size=0x0.
the program will find all files which contain at least one occurrence of PFNWP or MIA_CHECKED or MIS_SINGLE&per.
.br
If you instead specify&colon.
:xmp.:font facename='System VIO' size=12x5.
PFNWP&amp.MIA_CHECKED&amp.MIS_SINGLE
:exmp.:font facename=default size=0x0.
the program will find all files which contain at least one occurrence of all the specified words&per.
.br
You cannot use mixed or complex boolean logic&colon. the text  strings to be searched must either be separated by OR symbols or by AND symbols&per.
.br
So in the following search string&colon.
:xmp.:font facename='System VIO' size=12x5.
PFNWP|MIA_CHECKED&amp.MIS_SINGLE
:exmp.:font facename=default size=0x0.
only the first  OR  character is interpreted as a boolean symbol and the program will look for the strings &cdq.PFNWP&cdq. or &cdq.MIA_CHECKED|MIS_SINGLE&cdq.&per.
.br
:li.:hp2.File and text to search history:ehp2.
.br
The data of the searched files and text strings are saved in
drop down boxes, so in case you have to make a new search of some text
in a different path or of new text in a path you had already researched,
you just have to select the data from the drop down lists&per.
.br
:li.:hp2.Result history
.br
:ehp2.A drop down box showing both the details of the searched files
and text allows you to recall the result of a previous file search&per.
.br
:li.:hp2.Separate file and text lists
.br
:ehp2.There is a separate list for displaying the file with met the search criteria&per. Once a file is selected the text list is filled with all the occurrences of the found text&per.
.br
Two arrow buttons allow to resize the two lists according to your needs&per.
.br
:li.:hp2.Optionally display file date, time and size:ehp2.
.br
You can choose from the menu what level of details you need&per.
.br
:li.:hp2.Optionally sort the found files list:ehp2.
.br
You can display them unsorted, sorted ascending or descending&per. Sorting
depends on the full file data, so if the file date are displayed the files
are sorted according to the current date, if only the file size is displayed
the file are sorted according to their size, if no additional details
is displayed the file are sorted by name&per.
.br
:li.:hp2.Optionally display line numbers and hide leading spaces:ehp2.
.br
You can display line numbers in the list of found text&per. You can also optionally strip the leadings spaces (but you have to repeat the research to do that)&per.
.br
:li.:hp2.Resizeable and customizable interface:ehp2.
.br
You can use the font palette to change the font used by the main window&colon.
drag a font from the palette and drop it on the window border&per. The window
size and layout will change to adapt to the new font size&per.
.br
You can also use the font palette to change the font used by the entry
fields and result lists&per.
.br
In order to get aligned entries in the result windows you should select
a monospaced font (Courier, System VIO, etc&per.) and drop it on any entry
field or list window, the other entry field&slr.list windows will automatically
adopt the new font&per.
.br
You can also resize and move the window where you like&per. The layout
of the inner controls will adapt to the new window size&per.
.br
You can also drop a color on the Search , Stop  or Help buttons&per.
.br
The window size and position, the fonts and the button colors are saved
in the DATASEEK&per.INI file and restored the next time you start the program&per.
.br
:li.:hp2.Subdirectory search:ehp2.
.br
Like with PMSEEK&per.EXE you can recursively search throught subdirectories&per.
.br
:li.:hp2.Exclude binary files:ehp2.
.br
You can optionally exclude files containing NUL characters (i&per.e&per. characters
with ASCII value 0)&per.
.br
:li.:hp2.Case option:ehp2.
.br
Like with PMSEEK&per.EXE you can perform case sensitive or case insensitive searches&per.
.br
:li.:hp2.Boolean operators:ehp2.
.br
See the previous notes about  multiple
file names and multiple text strings&per.
.br
:li.:hp2.Settings dialog:ehp2.
.br
A settings dialog is provided in order to set the default view and
search options, the size of the result history and the names of the default
editor and command shell&per.
:eul.

.* Main window
:h1 res=8 x=left y=bottom width=100% height=60% group=3 rules=border scroll=vertical titlebar=none.Main window
:i1 id=mainwin.Main window
:font facename='Helv' size=20x12.
Main window
:font facename=default size=0x0.
:p.
This first version of the program is based on a single resizeable window&per.
A future version (2 ?) might be based on multiple windows in order to
provide more and promptly available search options&per.
Here is a description of the various window controls&per.
.br
:ul compact.
:li.:hp2.Files to search for&colon.:ehp2.
.br
You enter the specifications of the searched files here&per. Wildcard characters
as &csq. &asterisk. &csq. and &csq. ? &csq. are allowed as usual&per.
.br
You can specify multiple file names&per. See the notes above for more details&per.
.br
Once you click on the :hp1.Search:ehp1. button the
entered data are saved in the embedded list and can be recalled any time
later by selecting them&per.
.br
To delete an entry from the list, select it and press :hp2.Ctrl + D:ehp2.&per.
To delete the whole list, give it the focus and then press :hp2.Ctrl + Shift
+ D&per.:ehp2.
.br
:li.:hp2.Text to search for (if any)&colon.:ehp2.
.br
You optionally enter the text to be searched in the files(s) you specified&per.
Currently no wildcard characters are allowed for searched text, although
I might add GREP and E-GREP search capabilities in near future&per.
.br
You can use boolean operators to search
for multiple text sentences&per. See the notes
above for more details&per.
.br
Once you click on the :hp1.Search:ehp1. button the
entered data are saved in the embedded list and can be recalled any time
later by selecting them there&per.
.br
To delete an entry from the list, select it and press :hp2.Ctrl + D:ehp2.&per.
To delete the whole list, give it the focus and then press
:hp2.Ctrl + Shift + D&per.:ehp2.
.br
:li.:hp2.Search results history&colon.:ehp2.
.br
Once a search operation completes succesfully, the search criteria
are added to this drop down list in the following format&colon.
:xmp.:font facename='System VIO' size=12x5.
[XXXX]  FILE_SPECIFICATIONS < TEXT_SPECIFICATIONS >
:exmp.:font facename=default size=0x0.
where
:font facename='System VIO' size=12x5.
XXXX
:font facename=default size=0x0.
is the search attribute mask, i&per.e&per. the mask of the options used to perform the search operation&colon.
:dl compact.
:dt.
:font facename='System VIO' size=12x5.
R
:font facename=default size=0x0.
:dd.
:hp5.R:ehp5.ecursive search&per.
:dt.
:font facename='System VIO' size=12x5.
T
:font facename=default size=0x0.
:dd.
:hp5.T:ehp5.ext files (i&per.e&per. the :hp1.Exclude binary files :ehp1. option)&per.
:dt.
:font facename='System VIO' size=12x5.
I
:font facename=default size=0x0.
:dd.
Case :hp5.i:ehp5.nsensitive&per.
:dt.
:font facename='System VIO' size=12x5.
B
:font facename=default size=0x0.
:dd.
:hp5.B:ehp5.oolean operators&per.
:edl.
:p.
When an option is not set it is displayed with a dash &csq. - &csq. (e&per.g&per.
:font facename='System VIO' size=12x5.
[R-I-]
:font facename=default size=0x0.
means that the recursive and case insensitive options were set)&per.
.br
The list of found files and text is also internally stored by the program
so you can select any time a previous search criteria to get back the
results&per.
.br
To delete an entry from the list, select it and press :hp2.Ctrl + D:ehp2.&per.
To delete the whole list, give it the focus and then press :hp2.Ctrl + Shift
+ D&per.:ehp2.
.br
The default limit for this list is 10 entries&per. You can increase this
limit up to 100 via the Settings dialog&per.
.br
:li.:hp2.Files found&colon.:ehp2.
.br
The files satisfying the search criteria are listed here&per.
.br
Use the View menu items to display more or less details or to sort
the list items:hp2.&per.:ehp2.
.br
:li.:hp2.Down&slr.Up arrows&colon.:ehp2.
.br
These buttons change the height of the :hp1.Files found:ehp1.  listbox&per.
For instance, if you are more interested in the list of the text lines
matching the search criteria, you can reduce the :hp1.Files found :ehp1.
list  height to the height of one text line, or you can just hide
the :hp1.Text found :ehp1. list if you are only looking for files&per.
.br
:li.:hp2.Text found&colon.:ehp2.
.br
The lines of text contained in the currently selected file and matching
the search criteria are listed here&per.
.br
You can optionally display the line number and remove the leading spaces&per.
.br
:li.:hp2.Search:ehp2.
.br
Press this button to start the search operation&per.
.br
This button is disabled if the :hp1.Files
to search for :ehp1. field is empty&per.
.br
:li.:hp2.Stop:ehp2.
.br
This button is enabled only if the program is executing a file
search&per.
.br
You must use it to interrupt a search operation&per.
.br
:li.:hp2.Help:ehp2.
.br
Display the program documentation&per.
:eul.

.* Menu File
:h1 res=9 x=left y=bottom width=100% height=60% group=3 rules=border scroll=vertical titlebar=none.Menu file
:i1 id=menufile.Menu File
.* :i2 refid=menufile.Open
.* :i2 refid=menufile.Save
.* :i2 refid=menufile.Exit
:font facename='Helv' size=20x12.
Menu File
:font facename=default size=0x0.
:p.
Currently not fully implemented (only Exit works)&per.
:ul compact.
.br
:li.:hp2.Open:ehp2.
.br
This will allow to load previously saved search criteria (i&per.e&per. what was in the :hp1.Files to search :ehp1. and :hp1.Text to search :ehp1.  lists) or previously saved search results&per.
.br
:li.:hp2.Save:ehp2.
.br
This will allow to save the current content of the :hp1.Files to search :ehp1. and :hp1.Text to search :ehp1. lists, the currently
displayed search result or all the full list of search results&per.
.br
Search results will optionally be save as readable text files or as re-loadable binary files&per.
.br
You can use Ctrl + S as keyboard shortcut for this command&per.
.br
:li.:hp2.Exit
.br
:ehp2.Just terminate the program&per. You can use F3 as keyboard shortcut
for this command&per.
:eul.

.* Menu Selected
:h1 res=10 x=left y=bottom width=100% height=60% group=3 rules=border scroll=vertical titlebar=none.Menu selected
:i1 id=menuselected.Menu Selected
:font facename='Helv' size=20x12.
Menu Selected
:font facename=default size=0x0.
:p.
:hp2.Note &colon.:ehp2. the features :hp5.not yet:ehp5. implemented are prefixed with an asterisk (&asterisk.)&per.
:ul compact.
.br
:li.:hp2.Edit:ehp2.
.br
Starts EPM&per.EXE or any other editor according to the user preferences&per.
.br
You can use Ctrl + E as keyboard shortcut for this command&per.
.br
If you use EPM&per.EXE and keep the Shift key down while selecting :hp1.Edit :ehp1. or press Ctrl + Shift + E, the selected file will be opened in an previous EPM window (if any window is present)&per.
.br
If an item in the list of :hp1.Text found :ehp1. is selected, and you use EPM as editor, the editor will automatically scroll to the selected line of text&per.
.br
:li.:hp2.Open:ehp2.
.br
Opens the selected file according to the current WPS association&per.
.br
You can use Ctrl + O as keyboard shortcut for this command&per.
.br
:li.:hp2.Settings:ehp2.
.br
Opens the settings notebook of the currently selected file&per.
.br
You can use Ctrl + P as keyboard shortcut for this command&per.
.br
:li.:hp2.(&asterisk.) Copy to clipboard:ehp2.
.br
Will copy the full name of the currently selected file to the clipboard&per.
.br
You can use Ctrl + C as keyboard shortcut for this command&per.
.br
:li.:hp2.(&asterisk.) Copy All to Clipboard:ehp2.
.br
Will copy the whole list of found files to the clipboard&per.
.br
You can use Ctrl + Shift + C as keyboard shortcut for this command&per.
.br
:li.:hp2.Folder:ehp2.
.br
Opens the folder containing the selected file&per.
.br
You can use Ctrl + F as keyboard shortcut for this command&per.
.br
:li.:hp2.Command line:ehp2.
.br
Starts the command shell selected by the user in the Settings dialog
setting the path of the currently selected file as the working directory&per.
.br
You can use Ctrl + L as keyboard shortcut for this command&per.
.br
:li.:hp2.(&asterisk.) As parameter &per.&per.&per.:ehp2.
.br
Will allow to execute a command againgst the currently selected file&per.
.br
You can use Ctrl + M as keyboard shortcut for this command&per.
:eul.

.* Menu View
:h1 res=11 x=left y=bottom width=100% height=60% group=3 rules=border scroll=vertical titlebar=none.Menu View
:i1 id=menuview.Menu View
:font facename='Helv' size=20x12.
Menu View
:font facename=default size=0x0.
:p.
Through the items in this menu you can modify the display of the found
files and text lines&per.
:ul compact.
.br
:li.:hp2.Show file date:ehp2.
.br
The list of the found files includes the file last write date before
the file name&per.
.br
:li.:hp2.Show fle time:ehp2.
.br
The list of the found files includes the file last write time before
the file name&per.
.br
:li.:hp2.Show file size:ehp2.
.br
The list of the found files includes the file size before the file
name&per.
.br
:li.:hp2.Show line numbers:ehp2.
.br
The list of the found lines of text includes the line number&per.
.br
:li.:hp2.Strip spaces:ehp2.
.br
Leading spaces are stripped from the found lines of text&per.
.br
In this case, to change the display of already found lines of text
you have to repeat the search operation&per.
.br
:li.:hp2.Do not sort:ehp2.
.br
List the names of the files in the order they are found&per.
.br
:li.:hp2.Sort ascending:ehp2.
.br
Sort the file names according to the first detail displayed (file date,
time, size or name)&per.
.br
:li.:hp2.Sort descending:ehp2.
.br
Reverse sort the file names according to the first detail displayed (file date, time, size or name)&per.
:eul.

.* Menu Options
:h1 res=12 x=left y=bottom width=100% height=60% group=3 rules=border scroll=vertical titlebar=none.Menu Options
:i1 id=menuoptions.Menu Options
:font facename='Helv' size=20x12.
Menu Options
:font facename=default size=0x0.
:p.
The items of this menu allow to change some search options or to display
the settings dialog&per.
:ul compact.
.br
:li.:hp2.Recursive search:ehp2.
.br
Search the specified file names recursively in the subdirectories of
the starting paths&per.
.br
:li.:hp2.Exclude binary files:ehp2.
.br
Skip text search in files which contain NUL character i&per.e&per. characters with ASCII  value of 0&per.
.br
:li.:hp2.Case insensitive:ehp2.
.br
Do a case insensitive search&per. If this item is unchecked the search
is case sensitive, that is &cdq.CaSe&cdq. will not match &cdq.case&cdq.&per.
.br
:li.:hp2.Boolean operators:ehp2.
.br
Allow to find multiple text sentences with an OR or an AND logic&per.
.br
See the previous notes for more details&per.
.br
:li.:hp2.Settings &per.&per.&per.:ehp2.
.br
Display the settings dialog&per.
:eul.

.* Menu Help
:h1 res=13 x=left y=bottom width=100% height=60% group=3 rules=border scroll=vertical titlebar=none.Menu Help
:i1 id=menuhelp.Menu Help
:font facename='Helv' size=20x12.
Menu Help
:font facename=default size=0x0.
:p.
Currently only the :hp1.Help index:ehp1., :hp1.General help :ehp1. and
:hp1.Product information :ehp1. menu items work&per.

.* Mouse features
:h1 res=14 x=left y=bottom width=100% height=60% group=3 rules=border scroll=vertical titlebar=none.Mouse features
:i1 id=mouse.Mouse features
:font facename='Helv' size=20x12.
Mouse features
:font facename=default size=0x0.
:p.
As mouse features are intended commands executed by mouse button clicks
or by mouse drag and drop&per.
:p.
:hp2.Note &colon.:ehp2. the features :hp5.already:ehp5. implemented are prefixed with an asterisk&per.
:ul compact.
:li.:hp2.(&asterisk.) Double click on an item in the :hp1.Files found :ehp1. or:ehp2.:hp2.:hp1.Text found :ehp1.:ehp2.:hp2.list:ehp2.
.br
The performed action depends on the state of the keyboard&colon.
:ul compact.
:li.when :hp5.no keyboard modificator:ehp5. is used the selected file is opened via the editor&per.
.br
If the :hp1.Text found :ehp1. list contains a selected entry the cursor is automatically moved to the corresponding line of text&per.
:li.when the :hp5.Shift key is pressed:ehp5. the selected file is opened in the already running editor window (if any)&per.
:li.when the :hp5.Ctrl key is pressed:ehp5. the folder containing the selected file is opened&per.
:li.when the :hp5.Alt key is pressed:ehp5. the settings notebook of the selected file is opened&per.
:eul.
:p.
:hp5.Notes&colon.:ehp5.:hp2.
:ehp2.
:ul compact.
.br
:li.If the selected editor is not EPM some features might not work
properly&per. This problem might be solved in a future version allowing specifications
of particular editor parameters&per.
:li.A future version will also allow the user to select which functions are to be associated to the various mouse events&per.
:eul.
.br
:li.:hp2.Drag of the selected item from the :hp1.Files found :ehp1.
list to an EPM&per.EXE window:ehp2.
.br
The file is loaded in the EPM&per.EXE window or is brought to the top if
already present in the EPM&per.EXE files ring&per.
.br
:li.:hp2.Drag of the selected item from the :hp1.Text found :ehp1.
list to an EPM&per.EXE window:ehp2.
.br
The file is loaded in the EPM&per.EXE window or is brought to the top if
already present in the EPM&per.EXE files ring&per. The cursor is moved to the
selected line&per.
.br
:li.:hp2.Drag of the selected item from the :hp1.Files found :ehp1. list
or the :hp1.Text found :ehp1. list to another WPS window:ehp2.
.br
The file is treated as a WPS object&colon. it can be copied, moved, deleted (if dragged to the shredder)&per.
.br
:li.:hp2.Drop a file (no additional key) on the :hp1.Files to search
for :ehp1. entry:ehp2.
.br
The name of the file (including drive and path) is inserted into the
entry field replacing the previous content&per.
.br
:li.:hp2.Drop a file (+ Ctrl key) on the :hp1.Files to search for :ehp1.
entry:ehp2.
.br
The name of the file (including drive and path) is appended to the
previous content of the entry field using &csq.|&csq. as separator&per.
.br
:li.:hp2.Drop a file (+ Shift key) on the :hp1.Files to search for :ehp1.
entry:ehp2.
.br
The file extension (including drive and path - D&colon.&bsl.PATH&bsl.&asterisk.&per.EXT) is inserted
in the entry field replacing the previous content&per.
.br
If the Ctrl key is also pressed (drop + Ctrl + Shift) the file extension
is appended to the previous content of the entry field using &csq.|&csq. as
separator&per.
.br
:li.:hp2.Drop a file (+ Alt key) on the :hp1.Files to search for :ehp1.
entry:ehp2.
.br
The name of the file, including the drive letter and excluding the
path, is inserted in the entry field replacing the previous content&per.
.br
If the Shift key is also pressed the extension of the file, including the drive letter and the excluding the path is used&per.
.br
If the Ctrl key is also pressed the data of the current file are appended
to the previous entry field content&per.
.br
:li.:hp2.Drop a file (no additional key) on the :hp1.Text to search for :ehp1.
entry:ehp2.
.br
The file content (the first line of text) is inserted in the entry field replacing the previous content&per.
.br
:li.:hp2.Drop a file (+ Ctrl key) on the :hp1.Text to search for :ehp1.
entry:ehp2.
.br
The file content (the first line of text) is appended to the entry field content using &csq.&amp.&csq. as separator and the :hp1.Boolean
operators :ehp1. option is automatically activated&per.
.br
:li.:hp2.Drop a file (+ Shift key) on the :hp1.Text to search for :ehp1.
entry:ehp2.
.br
The file content (the first line of text) is appended to the entry field content using &csq.|&csq. as separator and the :hp1.Boolean
operators :ehp1. option is automatically activated&per.
.br
:li.:hp2.Drop a file (+ Ctrl + Shift key) on the :hp1.Text to search
for :ehp1. entry:ehp2.
.br
The file content (the first line of text) is appended to the entry field content without any separator&per.
.br
:li.:hp2.Drop a file (+ Alt key) on the :hp1.Text to search for :ehp1.
entry:ehp2.
.br
The full file name is inserted in the entry field&per.
:eul.

.* Startup parameters
:h1 res=15 x=left y=bottom width=100% height=60% group=3 rules=border scroll=vertical titlebar=none.Startup parameters
:i1 id=startupparms.Startup parameters
:i1 id=parameters.Parameters
:font facename='Helv' size=20x12.
Startup parameters
:font facename=default size=0x0.
:p.
The program supports a limited set of parameters as this is a PM program
and there are already various different command line programs to search
for file and text&per.
:xmp.:font facename='System VIO' size=12x5.
DATASEEK -L&colon.<filename>
:exmp.:font facename=default size=0x0.
will load <filename> where <filename> is a binary file
containing previously saved search criteria or search results&per.
:xmp.:font facename='System VIO' size=12x5.
DATASEEK -P&colon.<filename>
:exmp.:font facename=default size=0x0.
will use <filename> as an initialization file instead of looking
for DATASEEK&per.INI in the program path&per.
:xmp.:font facename='System VIO' size=12x5.
DATASEEK <file to search for> [text to search for]
:exmp.:font facename=default size=0x0.
will start the program, auto-filling the :hp1.Files to search for :ehp1.
and :hp1.Text to search for :ehp1. fields&per.
.br
If the file to search for is a directory the
:font facename='System VIO' size=12x5.
&cdq.&bsl.&asterisk.&cdq.
:font facename=default size=0x0.
search pattern is automatically appended to the directory name&per.

.* Notes on metacharacters usage
:h1 res=16 x=left y=bottom width=100% height=60% group=3 rules=border scroll=vertical titlebar=none.Notes on metacharacters usage
:i1 id=metachars.Metacharacters
:i1 id=wildcard.Wildcards
:font facename='Helv' size=20x12.
Notes on metacharacters usage
:font facename=default size=0x0.
:p.
DataSeeker implements metacharacters pattern matching in a stricter way
than the OS&slr.2 DIR command&per.
.br
As stated in the OS&slr.2 :hp1.Control program programming guide and reference
:ehp1.&colon.
:p.
:lm margin=10.
Metacharacters are characters that can be used to represent placeholders
in a file name&per. The asterisk (&asterisk.) and the question mark (?) are the two metacharacters&per.
:p.
The asterisk matches one or more characters, including blanks&per.
:p.
The question mark matches exactly one character, unless that character
is a period&per. To match a period, the original name must contain a period&per.
Metacharacters are illegal in all but the last component of a path&per.
:lm margin=1.
:p.
The main difference is in the usage of the question mark character&per. For instance while
:font facename='System VIO' size=12x5.
DIR ?&per.zip
:font facename=default size=0x0.
lists files named
:font facename='System VIO' size=12x5.
&per.zip
:font facename=default size=0x0.
and,
:font facename='System VIO' size=12x5.
A&per.zip
:font facename=default size=0x0.
, DataSeeker will list only
:font facename='System VIO' size=12x5.
A&per.zip
:font facename=default size=0x0.
as it makes the question mark match only:hp1. exactly one character, unless
that character is a period:ehp1. &per.

.* Planned features
:h1 res=17 x=left y=bottom width=100% height=80% group=2 rules=border scroll=vertical titlebar=none.Planned features
:i1 id=plannedfeatures.Planned features
:font facename='Helv' size=20x12.
Planned features
:font facename=default size=0x0.
:p.
No big features planned any more. See ToDo list at netlabs svn for some thoughs for the future&per.

.* Registration
:h1 res=18 x=left y=bottom width=100% height=80% group=2 rules=border scroll=vertical titlebar=none.Registration
:i1 id=registration.Registration
:font facename='Helv' size=20x12.
Registration
:font facename=default size=0x0.
:hp2.
:p.
As this program is open source now, there will be never any registration few&per.
:ehp2.


.* mailto:acantatore@tin.it
:fn id=2.
:p.The link you selected points to an external resource&per. Click the URL below to launch Netscape Navigator
:p.
:link reftype=launch object='netscape.exe ' data='mailto:acantatore@tin.it'.
mailto&colon.acantatore&atsign.tin&per.it
:elink.
:efn.

.* mailto:alscant@libero.it
:fn id=3.
:p.The link you selected points to an external resource&per. Click the URL below to launch Netscape Navigator
:p.
:link reftype=launch object='netscape.exe ' data='mailto:alscant@libero.it'.
mailto&colon.alscant&atsign.libero&per.it
:elink.
:efn.

.* http://space.tin.it/scienza/acantato
:fn id=4.
:p.The link you selected points to an external resource&per. Click the URL below to launch Netscape Navigator
:p.
:link reftype=launch object='netscape.exe ' data='http://space.tin.it/scienza/acantato'.
http&colon.&slr.&slr.space&per.tin&per.it&slr.scienza&slr.acantato
:elink.
:efn.

:euserdoc.