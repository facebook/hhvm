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
#
# This script detects if any UTF-8 files were incorrectly converted to EBCDIC, and 
# converts them back.

if [ -z "$QSH_VERSION" ];
then
	QSH=0
    echo "QSH not detected (QSH_VERSION not set) - just testing."
else
	QSH=1
	#echo "QSH version $QSH_VERSION"
fi
export QSH

tar_file=$1
echo ""
echo "Determining binary files by BOM ..."
echo ""
bin_count=0
binary_files=""
# Process BOMs
   for file in `find ./icu/source/data/unidata \( -name \*.txt -print \)`; do
    bom8=`od -t x1 -N 3 $file|\
          head -n 1|\
          cut -c10-18`;
    #Find a converted UTF-8 BOM
    echo "file $file bom /${bom8}/"
    if [ "$bom8" = "57 8b ab" ]
    then
        file="`echo $file | cut -d / -f2-`"
        echo "converting ${file}"
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
    if [ `echo $binary_files | wc -w` -gt 0 ]
      then
        echo restoring
        rm $binary_files
               pax -C 819 -rvf $tar_file $binary_files
       fi



