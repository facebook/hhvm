#!/usr/bin/qsh
# Copyright (C) 2016 and later: Unicode, Inc. and others.
# License & terms of use: http://www.unicode.org/copyright.html
#   Copyright (C) 2000-2011, International Business Machines
#   Corporation and others.  All Rights Reserved.
#
# Authors:
# Ami Fixler
# Barry Novinger
# Steven R. Loomis
# George Rhoten
# Jason Spieth
#
# Shell script to unpax ICU and convert the files to an EBCDIC codepage.
# After extracting to EBCDIC, binary files are re-extracted without the
# EBCDIC conversion, thus restoring them to original codepage.

if [ -z "$QSH_VERSION" ];
then
	QSH=0
    echo "QSH not detected (QSH_VERSION not set) - just testing."
else
	QSH=1
	#echo "QSH version $QSH_VERSION"
fi
export QSH

# set this to "v" to list files as they are unpacked (default)
VERBOSE_UNPACK="v"

# Set the following variable to the list of binary file suffixes (extensions)


#****************************************************************************
#binary_suffixes='ico ICO bmp BMP jpg JPG gif GIF brk BRK'
#ICU specific binary files
#****************************************************************************
binary_suffixes='brk BRK bin BIN res RES cnv CNV dat DAT icu ICU spp SPP xml XML nrm NRM utf16be UTF16BE'
data_files='icu/source/data/brkitr/* icu/source/data/locales/* icu/source/data/coll/* icu/source/data/rbnf/* icu/source/data/mappings/* icu/source/data/misc/* icu/source/data/translit/* icu/source/data/unidata/* icu/source/test/testdata/*'

#****************************************************************************
# Function:     usage
# Description:  Prints out text that describes how to call this script
# Input:        None
# Output:       None
#****************************************************************************
usage()
{
  echo "Enter archive filename as a parameter: $0 icu-archive.tar"
}

#****************************************************************************
# first make sure we at least one arg and it's a file we can read
#****************************************************************************

# check for no arguments
if [ $# -eq 0 ]; then
  usage
  exit
fi

# tar file is argument 1
tar_file=$1

# check that the file is valid
if [ ! -r $tar_file ]; then
  echo "$tar_file does not exist or cannot be read."
  usage
  exit
fi

# treat all data files as ebcdic
ebcdic_data=$data_files

#****************************************************************************
# Extract files.  We do this in two passes.  One pass for 819 files and a
# second pass for 37 files
#****************************************************************************
echo ""
echo "Extracting from $tar_file ..."
echo ""

# extract everything as iso-8859-1 except these directories
pax -C 819 -rc${VERBOSE_UNPACK}f $tar_file $ebcdic_data

# extract files while converting them to EBCDIC
echo ""
echo "Extracting files which must be in ibm-37 ..."
echo ""
pax -C 37 -r${VERBOSE_UNPACK}f $tar_file $ebcdic_data

#****************************************************************************
# For files we have restored as CCSID 37, check the BOM to see if they    
# should be processed as 819.  Also handle files with special paths. Files
# that match will be added to binary files lists.  The lists will in turn
# be processed to restore files as 819.
#****************************************************************************
echo ""
echo "Determining binary files by BOM ..."
echo ""
bin_count=0
# Process BOMs
if [ -f icu/as_is/bomlist.txt ];
then
    echo "Using icu/as_is/bomlist.txt"
    pax -C 819 -rvf $tar_file `cat icu/as_is/bomlist.txt`
else 
   for file in `find ./icu \( -name \*.txt -print \)`; do
    bom8=`head -n 1 $file|\
          od -t x1|\
          head -n 1|\
          sed 's/  */ /g'|\
          cut -f2-4 -d ' '|\
          tr 'A-Z' 'a-z'`;
    #Find a converted UTF-8 BOM
    if [ "$bom8" = "057 08b 0ab" -o "$bom8" = "57 8b ab" ]
    then
        file="`echo $file | cut -d / -f2-`"

        if [ `echo $binary_files | wc -w` -lt 200 ]
        then
            bin_count=`expr $bin_count + 1`
            binary_files="$binary_files $file";
        else
            echo "Restoring binary files by BOM ($bin_count)..."
            rm $binary_files;
            pax -C 819 -rvf $tar_file $binary_files;
            echo "Determining binary files by BOM ($bin_count)..."
            binary_files="$file";
            bin_count=`expr $bin_count + 1`
        fi
    fi
  done
  # now see if a re-extract of binary files is necessary
  if [ `echo $binary_files | wc -w` -gt 0 ]
  then
      echo "Restoring binary files ($bin_count) ..."
      rm $binary_files
      pax -C 819 -rvf $tar_file $binary_files
  fi
fi

echo "# Processing special paths."
# Process special paths
more_bin_opts=$(echo $binary_suffixes | sed -e 's%[a-zA-Z0-9]*%-o -name \*.&%g')
# echo "Looking for additional files: find ... $more_bin_opts"
more_bin_files=$(find icu -type f \( -name '*.zzz' $more_bin_opts \)  -print)
echo "Restoring binary files by special paths ($bin_count) ..."
rm $more_bin_files
pax -C 819 -rvf $tar_file $more_bin_files

#****************************************************************************
# Generate and run the configure script
#****************************************************************************

echo ""
echo "Generating qsh compatible configure ..."
echo ""

sed -f icu/as_is/os400/convertConfigure.sed icu/source/configure > icu/source/configureTemp
del -f icu/source/configure
mv icu/source/configureTemp icu/source/configure
chmod 755 icu/source/configure

echo ""
echo "$0 has completed extracting ICU from $tar_file - $bin_count binary files extracted."

