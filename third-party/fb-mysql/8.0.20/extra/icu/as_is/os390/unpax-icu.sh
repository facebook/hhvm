#!/bin/sh
# Copyright (C) 2016 and later: Unicode, Inc. and others.
# License & terms of use: http://www.unicode.org/copyright.html
# Copyright (C) 2001-2010, International Business Machines
#   Corporation and others.  All Rights Reserved.
#
# Authors:
# Ami Fixler
# Steven R. Loomis
# George Rhoten
#
# Shell script to unpax ICU and convert the files to an EBCDIC codepage.
# After extracting to EBCDIC, binary files are re-extracted without the
# EBCDIC conversion, thus restoring them to original codepage.
#
# Set the following variable to the list of binary file suffixes (extensions)

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
tar_file=$1
if [ ! -r $tar_file ]; then
    echo "$tar_file does not exist or cannot be read."
    usage
    exit
fi

echo ""
echo "Extracting from $tar_file ..."
echo ""
# extract files while converting them to EBCDIC
pax -rvf $tar_file -o to=IBM-1047,from=ISO8859-1 -o setfiletag

#****************************************************************************
# For files we have restored as CCSID 37, check the BOM to see if they    
# should be processed as 819.  Also handle files with special paths. Files
# that match will be added to binary files lists.  The lists will in turn
# be processed to restore files as 819.
#****************************************************************************
echo ""
echo "Determining binary files by BOM ..."
echo ""

# When building in ASCII mode, text files are converted as ASCII
if [ "${ICU_ENABLE_ASCII_STRINGS}" -eq 1 ]; then
    binary_suffixes="$binary_suffixes txt TXT ucm UCM"
elif [ -f icu/as_is/bomlist.txt ];
then
    echo 'Using icu/as_is/bomlist.txt'
    binary_files=$(cat icu/as_is/bomlist.txt)
else
    echo "Analyzing files .."
	for file in `find ./icu \( -name \*.txt -print \) | sed -e 's/^\.\///'`; do
		bom8=`head -c 3 $file|\
			od -t x1|\
			head -n 1|\
			sed 's/  */ /g'|\
			cut -f2-4 -d ' '|\
			tr 'A-Z' 'a-z'`;
		#Find a converted UTF-8 BOM
		if [ "$bom8" = "57 8b ab" ]
		then
			binary_files="$binary_files $file";
		fi
	done
fi

echo "Looking for binary suffixes.."

for i in $(pax -f $tar_file 2>/dev/null)
do
	case $i in
	*/) ;;		# then this entry is a directory
	*.*)		# then this entry has a dot in the filename
		for j in $binary_suffixes
		do
			# We substitute the suffix more than once
			# to handle files like NormalizationTest-3.2.0.txt
			suf=${i#*.*}
			suf=${suf#*.*}
			suf=${suf#*.*}
			if [ "$suf" = "$j" ]
			then
				binary_files="$binary_files $i"
				break
			fi
		done
		;;
	*) ;;		# then this entry does not have a dot in it
    esac
done

# now see if a re-extract of binary files is necessary
if [ ${#binary_files} -eq 0 ]; then
    echo ""
    echo "There are no binary files to restore."
else
    echo "Restoring binary files ..."
    echo ""
    rm $binary_files
    pax -rvf $tar_file $binary_files
    # Tag the files as binary for proper interaction with the _BPXK_AUTOCVT
    # environment setting
    chtag -b $binary_files
fi
echo ""
echo "$0 has completed extracting ICU from $tar_file."
