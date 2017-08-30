
# Flea

flea is a layout printer for use with [magic VLSI](http://www.opencircuitdesign.com/magic/) files.

## Usage
```
flea [ -dicfrlLtp ] [ options arguments ] file1 file2 ...

options:
    d - Specify device in flag data
        Devices:
            PL   - HP7550 plotter    (Output piped to command given in PLOUTPUT
            PL-B - For B sized paper  environment variable)
            PL-C - For C sized paper
            PL-D - For D sized paper

            HPGL - hpgl output
            HPGL-B
            HPGL-C
            HPGL-D

            LW   - Laser Printer    (Postscript Output piped to command given in
                                     LWOUTPUT environment variable)
            PS   - Post Script output

            VP   - Versatec Plotter

    i - Use interactive mode.
    c - Read file as cif file. Program appends .cif to filename.
    f - Scale to full page size.
    r - Specify cell recursion level in flag data
    l - Specify label recursion level in flag data
    L - Plot only layers listed in flag data. Layers seperated by commas
        only - no spaces. (Note: must occur before techfile.)
    t - Specify tech file in flag data.
    p - Search these paths for files when reading magic files. Paths
        seperated by ':' and end in a '/'.

EXAMPLES:

This plots the file test.mag to the HP7550 plotter, only plotting the listed
layers.

flea -dL PL poly,metal1,metal2,ndiff,pdiff test

```


## Original Readme:

Files that need to be changed:

flea.h: includes paths that may need to be customised for your system.
Specifically the #define TECHFILES needs to contain the full path to
the directory under which the included lib directory will be installed.


Environment variable PLOUTPUT is the program executed to plot to the
HP plotter, the default is "lpr -Ppl"
(The device name for flea is PL)

Environment variable LWOUTPUT is the program executed to plot to the
laser printer, the default is "lpr -Plw"
(The device name for flea is LW) 

You need to set up the lib directory included in the place specified 
in flea.h,  If you are not using the mosis CIF layer names, or the
berkley scmos techfile, then you may need to change some of the files
in this directory.  The man pages describes the format of those files.

You may also need to update or change the file 'flearc' in the lib
directory.

If you have problems installing this software, I can be contacted at:
lush@erc.msstate.edu

Ed Luke


