/*  REXX script to generate a buildlevel string
    creates a buillevel.txt file which can be linked with
    wlink ... option description @..\buildlevel.txt

    For use with 4os2. Uses enviroment variable %hostname% as build machine entry.
    Date/Time is extracted from msg.obj to match ver /r output.
    Version is taken from version.h and build.h

    20100515 AB initial
    20100527 AB adopted to DataSeeker

*/

VendorName = 'AB'       /* no more than 31 chars */
BuildMachine = value('hostname',,'OS2ENVIRONMENT')
ASD      = ''
Language = ''           /* f.i. EN or empty string */
Country  = ''           /* f.i. AT or empty string */
CPU      = ''           /* U for uniprocessor or empty string */
FixPack  = ''           /* */
Description = 'File and text search utility - http://dataseeker.netlabs.org/'



/* limit vendor name */
IF LENGTH(VendorName) > 31 THEN DO
    VendorName = LEFT(VendorName,31)
END

/* get version info from pmseek.h */
file = 'pmseek.h'
DO WHILE lines(file) > 0
    line = LINEIN(file)
    pos = WORDPOS('DATASEEKER_VERSION' , line)
    IF pos > 0 THEN DO
        ver_maj = WORD(line, pos + 1)
        ver_maj = SUBSTR(ver_maj, 2, LENGTH(ver_maj) - 2)
        /*SAY 'Major='ver_maj*/
    END
/*    pos = WORDPOS('VER_MINOR' , line)
    IF pos > 0 THEN DO
        ver_min = WORD(line, pos + 1)
        SAY 'Minor='ver_min
    END */
END
vers = ver_maj/*'.'ver_min*/
/*SAY 'Version='vers*/

/* get build level from build.h */
file = 'build.h'
ver_build = '0'
DO WHILE lines(file) > 0
    line = LINEIN(file)
    pos = WORDPOS('VER_BUILD', line)
    IF pos > 0 THEN DO
        ver_build = WORD(line, pos + 1)
        /*SAY 'Build='ver_build*/
    END
END
IF LENGTH(ver_build) > 7 THEN ver_build = LEFT(ver_build,7)

/* extract compile date/time from msg.obj */
file = 'prodinfo.obj'
SearchText = 'BUILD_DATE_TIME'
SearchTextOffs = LENGTH(SearchText) + 1
DO WHILE lines(file) > 0
    line = LINEIN(file)
    pos = POS(SearchText , line)
    IF pos > 0 THEN DO
        /*SAY SearchText' found at offset 'pos*/
        pos = pos + SearchTextOffs
        line = SUBSTR(line, pos, 50)
        PARSE VAR line month line
        PARSE VAR line day line
        PARSE VAR line year line
        PARSE VAR line time line
        /*SAY 'Date="'date'" Time="'time'"'*/
        month=WORDPOS(month, 'Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec')
        IF LENGTH(month) < 2 THEN month = INSERT('0',month)
        IF LENGTH(day) < 2 THEN day = INSERT('0',day)
        /*SAY 'Tag='day' Monat='month' Jahr='year*/
    END
    /* check if it's a debug build */
    IF POS('debug_line', line) > 0 THEN ver_min=ver_min'.DEBUG'
END
time = SUBSTR(time, 1, 5)
datetime = year'-'month'-'day' 'time
/*SAY 'datetime='datetime*/

/* compose Feature string (:ASD:EN:US:4b:U:8101) */
Feature=':'ASD':'Language':'Country':'ver_build':'CPU':'FixPack
/* SAY 'Feature='Feature */

/* build time stamp und build machine string */
/* date/time have to be 26 chars (leading ' ' required) */
IF LENGTH(BuildMachine) > 11 THEN DO
    BuildMachine = LEFT(BuildMachine, 11)
    END
bldmachlen = LENGTH(BuildMachine)
/*SAY 'bldmach='BuildMachine'<-- len='bldmachlen*/
datimmach = LEFT(datetime, 24)
datimmach = datimmach' 'BuildMachine
/*SAY datimmach'<--'*/

FullString = "Description '@#"VendorName':'ver_maj'#@##1## 'datimmach''Feature'@@  'Description"'"
'@CALL del dataseeker.def 2>NUL >NUL'
file = 'dataseeker.def'
IF LINEOUT(file,FullString,1) <> 0 THEN SAY "ERROR - can't write buildlevel.txt"
ELSE SAY FullString

