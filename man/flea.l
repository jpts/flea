.TH  FLEA LOCAL
.UC 4
.SH NAME
flea \- plot magic or cif files.
.SH SYNTAX
.B flea [\-idcfFrlLstpo] [options\ arguments]\ file1\ [file2]\ [...]
.br
.SH DESCRIPTION
.I Flea
([F]un [L]oveable [E]ngineering [A]rtist)
is a program used to plot magic and cif design files to various output
devices. Parameters are passed to
.I flea
through the flags and flag data or through .flearc files and tech files.
The flag options, if they exist, immediately follow the command - any
arguments to the flag options follow the flag options list in the order
in which the flags were used.
.PP
Flea options:
.TP
.B \-i
Interactive, Prompt user for options and data.
.TP
.B \-d
Device, Specify device to plot to in option arguments
.br
  Devices:
.br
    HP    - HP7580 plotter
.br
    HPGL  - HP7550 hpgl file output
.br
    PL    - HP7550 plotter lpr output  (printer (pl))
.br
    PS    - Postscript file output
.br
    LW    - Laser Writer lpr output    (printer (lw))
.br
    VP    - Versatec versaplot random output
.TP
.B \-c
read Cif, This option used when you want to plot cif files instead of magic files.
If the filenames are not appended with .cif then flea will add them.
.TP
.B \-f
Fullpage, This option forces flea to scale to the largest plot size
rather than the nearest even number of units per inch.
.TP
.B \-F
Fastmode, This option causes flea to skip the processing required to
eliminate extra lines on the plot. For Chip size plots this may be
useful to speed up plot processing time.
.TP
.B \-r
Recurse level, This option specifies in the option data the number of
levels in which cells will be expanded (not drawn as bounding boxes).
.TP
.B \-l
Label recurse level, This option specifies in the option data the number
of levels in which labels will be drawn.
.TP
.B \-L
Layers, This option selects in the option data the layers to be plotted.
If left unspecified all layers are plotted. Layers listed are separated by
commas without spaces. This option must precede the option that reads
a techfile if given.
.TP
.B \-s
Scale, This option will force the scale to X-number of units per inch.
An integer in the options data specifies this number.
.TP
.B \-t
read Techfile, This option reads the techfile given by the filename in
the options data instead of a library techfile.
.TP
.B \-p
Paths, This option sets the search path list for magic libraries. Each
path must end with a '/' and be separated by a ':', again no spaces are
allowed.
.TP
.B \-o
Option, Set the option string. Used to pass parameters to the .flearc
file.

.br
.SH CUSTOMIZING FLEA
.PP
Flea is customizable via two avenues - 1) Techfiles 2) .flearc files.
The techfile describes how each layer will be considered by the plot
program, what fill patterns to use, and what colors to draw. The
individual user may create his own techfile to suit his own personal
tastes.

NOTE: The techfile describes highly device specific information:
Many parameters are directly passed to the devices with little or no
processing, and some devices do not support all of the features such as
fill patterns.

The techfile is free form: all leading spaces and tabs are ignored.
Lines beginning with a '#' are comment lines.
The three basic commands in the techfile are 1) the define layer
command, 2) the describe layer command, and 3) the include layer
command. The syntax of the define layer command is as follows:
.br
.br
> 'layer name' 'comment field'
.br
.br
This command builds a data space for a layer named 'layer name' - the
describe layer command is used to determine how this new layer will
behave with the output device. The syntax for the describe layer command
is as follows:
.br
.br
. 'layer variable' 'integer number'
.br
.br
The layer variables are: PenNumber, LineStyle, FillPen, FillStyle,
LabelSize, LabelPen. PenNumber defines the number of the pen used to
draw this layer (a -1 means do not draw the boundary of this layer).
LineStyle defines a line style for the output device such as dashed
lines; this parameter is ignored on some devices. FillPen defines the
pen used to fill a layer (a -1 means do not fill this layer, a -2 means
fill this layer with an 'X', a -3 means fill this layer with a
diamond.); fill patterns are not supported on all devices. FillStyle
describes the style in which the fill should take place.
.br
------------------------------------------------------
.br
.br
# The following is a sample of a techfile layer entry.
.br
# The layers poly and ndiff have already been defined.
.br
> ntran             * Defining n-transistor layer
.br
  . PenNumber -1    * Do not draw this layer's bounds
.br
  . FillPen   3     * Fill this layer with this pen
.br
  . FillStyle 803   * HP device specific fill pattern code
.br
  . LabelSize 1     * Use a small label for transistors
.br
  . LabelPen  3     * Use transistor color for label
.br
    & poly          * Also insert on poly layer
.br
    & ndiff         * And n-diffusion layer
.br
------------------------------------------------------
.PP
Before flea begins plotting it searches for a .flearc file which sets up
default parameters. If no .flearc file exists then flea operates in a
default mode. The .flearc file in the current working directory (if it
exists) has priority over the .flearc file in the home directory. The
main purpose of the .flearc file is to set up default operation of flea.
The command that reflects this purpose most is the 'set' command.
SYNTAX: set 'variable name' 'value'. The variables that can be set are:
.br
    Variable      Type    
 --------------------------
 |  cif           BOOLEAN |
 |  fullpage      BOOLEAN |
 |  interactive   BOOLEAN |
 |  fastmode      BOOLEAN |
 |  cellrecurse   INTEGER |
 |  labelrecurse  INTEGER |
 |  plotlayers    STRING  |
 |  path          STRING  |
 |  techlib       STRING  |
 |  right-margin  FLOAT   |
 |  left-margin   FLOAT   |
 |  top-margin    FLOAT   |
 |  bottom-margin FLOAT   |
 --------------------------
.PP
.TP
cif \-
The cif boolean informs flea to use the cif
reader and not the magic reader.
.br
.TP
fullpage \-
The fullpage boolean tells the scale routine to scale to fit the
full page rather than the nearest 10 units/inch.
.br
.TP
interactive \-
The interactive boolean tells flea to enter the
interactive mode.
.br
.TP
fastmode \-
The fastmode boolean tells flea not to completely process
it's data so that extra lines are not eliminated. This is useful for
speeding up chip sized plots in which plotting the boxes is
satisfactory.
.br
.TP
cellrecurse \-
This integer value is used to determine at which level
cells will begin to be drawn as bounding boxes.
.br
.TP
labelrecurse \-
This integer value is used to determine at which level a
cells labels will stop being drawn.
.br
.TP
plotlayers \-
This string is a list of layers followed by commas which
describes which layers to be processed and plotted. When reading cif
these layers must be cif layers. The plotlayers string must be set
correctly before the techfile is read in.
.br
.TP
path \-
This string contains a list of directorys separated by ':'. The
magic reader will search these paths for files.
.br
.TP
techlib \-
This string contains a list of directorys separated by ':'.
The techfile reader will search these paths for a technology name.
.br
.TP
right-margin \-
This floating point number describes the right margin on
the plot in inches.
.br
.TP
left-margin \-
This floating point number describes the left margin on
the plot in inches.
.br
.TP
top-margin,bottom-margin \-
The same as above.
.PP
A boolean variable is set with a 'Yes' 'No' or 'Toggle'. Example:
.br
set interactive toggle
.br
set fullpage yes
.br
The toggle is useful for inverting the meaning of booleans on the
command line. The line 'set interactive toggle' toggles the meaning of
the '-i' option on the command line. If a 'set interactive yes' is used
then flea would always assume the interactive mode regardless of the
\'-i' option.
The set command can also read its data from the terminal. SYNTAX:
.br
set fullpage ?
.br
This command will get the input for fullpage from the terminal.
.PP
The device command is used to select the default device of flea. The
command syntax is:
.br
device devicename
.br
EXAMPLE:
.br
device HP
.PP
A techfile can be specified in the .flearc file with the techfile
command. command syntax is:
.br
techfile filename
.br
EXAMPLE:
.br
techfile ~/msu2.techfile
.br
This command reads the techfile named msu2.techfile from the users home
directory.
.PP
The Echo and Write are commands used to send messages to the terminal
from the .flearc file. Echo echos a string plus a newline and
write echos only the string.
.br
EXAMPLE:
.br
echo Bill's Interactive mode.
.br
echo
.br
write Use cif?:
.br
set cif ?
.br
.PP
Adding text and lines to your plots is accomplished through the use of
the text and line commands in conjunction with the 'set <left-margin,
right-margin, top-margin, bottom-margin>' command. The positions are
relative to the top, bottom, left, and right margins. The X-positions
are relative to the left and right - Y-positions to the top and bottom.
The first character of the position value determines the relation: '<'
is left, '>' is right, '\' is top, '/' is bottom, and '.' is center.
Syntax of the line command is as follows:
.br
line x1,y1,x2,y2
.br
EXAMPLE:
.br
line <0.0,\0.0,>0.0,\0.0    #draw a line across the top
.br
The x1,y1,x2,y2 values are composed of an initial character describing
the relative position and a floating point number describing the number
of inches from the offending position the point will be placed.
.PP
The text command supports text which may or may not be rotated by 90
degrees; center, left, or right justified text; and different text
sizes( size values passed directly to the device drivers ). SYNTAX:
.br
text <rotate>,<center,left,right>,(label size),x,y,TEXT
.br
EXAMPLE:
.br
text rotate,center,4,<0.35,.0.0,DESIGN DIVISION
.br
The text may also have an imbedded % command which can allow the user to
include information such as date, scale, user name, and filename. 
Text options:
.TP
.B %f \-
The root filename of file being plotted is substituted for the %f.
.TP
.B %l \-
The complete filename of the file being plotted is substituted for the
%l.
.TP
.B %o \-
The options data string is substituted for the %o. The options string is
set by the -o flag on the command line or with the 'set option' command.
.TP
.B %d \-
The date is substituted for the %s.
.TP
.B %t \-
The date and time is substituted for the %t.
.TP
.B %u \-
The current user name is substituted for the %u.
.TP
.B %s \-
The scale of the plot rounded to the nearest integer is substituted for
the %s. The scale is in lambda/inch for magic files and microns/inch for
cif files.
.PP
The 'interact' command forces flea to enter the interactive mode so
that the flearc file can act on the entries made by the user
interaction.
.PP
The 'source' command causes flea to include another flearc file. This
command is useful for maintaining a global flearc file for such things
as a groups techtable library and also having the personal customization
of the flearc file in the home directory.
.PP
The 'test' command allows the flearc file to test the condition of
Integer, Boolean, and String data. The syntax is:
.br
test {value or $VARIABLE} {= or <>} {value or $VARIABLE}
.br
	{Commands}
.br
endtest
.br
EXAMPLE:
.br
test $device = "HP"
.br
	echo Plotting to HP device.
.br
endtest
.br
.SH THE DEVICES
.PP
This section explains the parameters and limitations of each device
supported by flea. The first devices covered will be the HP7580 and the
HP7550.
.br
DeviceName Device
.br
HP         HP7580
.br
PL         HP7550
.br
The parameters in the techfile for the HP and PL devices describes the
following:
.br
.TP
.B PenNumber \-
PenNumber describes the plotter pen (0-8) to use to plot a layer. A
PenNumber of -1 tells flea not to draw this layer.
.TP
.B LineStyle \-
LineStyle describes the style in which a lines will be drawn. A
linestyle of -1 is a solid line. The LineStyle consists of two
parameters - the dash type (first 3 bits) and the dash length (last 5
bits). The dash type is described in the HP manuals under the LT
instruction.
.TP
.B FillPen \-
FillPen specifies the plotter pen (0-8) to use when filling a layer. A
FillPen of -1 tells flea not to fill this layer.
.TP
.B FillStyle \-
FillStyle describes the fill pattern to be used to fill a layer. The
fill type (3 = single hatch,4 = cross hatched) is encoded into the first
three bits. The hatch spacing is encoded into the next 5 bits and the
angle ( a increment of 45 degrees) is encoded into the last 2 bits.
.TP
.B LabelSize \-
LabelSize describes the size of text attached to this layer in 100-ths
of inches. 
.TP
.B LabelPen \-
LabelPen describes which pen to use when drawing text attached to this
layer.
.SH CHANGING PRINTER NAMES
.PP
Environment variables can be used to change the default printer names
for PL, and LW.  The PLOUTPUT, and LWOUTPUT variables define what program 
will be executed for an output filter.  The defaults are PLOUTPUT = "lpr -Ppl", 
and LWOUTPUT = "lpr -Plw".

