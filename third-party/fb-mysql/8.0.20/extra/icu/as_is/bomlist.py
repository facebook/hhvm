#!/usr/bin/python

# Copyright (C) 2016 and later: Unicode, Inc. and others.
# License & terms of use: http://www.unicode.org/copyright.html
# Copyright (C) 2011 IBM Corporation and Others. All Rights Reserved.
#
# run in icu/
# will create file icu/as_is/bomlist.txt
#
# Usage: 
#   ( python as_is/bomlist.py > as_is/bomlist.txt ) || rm -f as_is/bomlist.txt

import os
import codecs

tree = os.walk(".")

nots=0
notutf8=0
noprops=0
utf8=0
fixed=0
tfiles=0
bom=codecs.BOM_UTF8


for ent in tree:
    (path,dirs,files) = ent
    if(path.find("/.svn") != -1):
        continue
    for file in files:
        tfiles=tfiles+1
        fp = (path + "/" + file)
        if not os.path.isfile(fp):
            continue
        f = open(fp, 'rb')
        bytes=f.read(3)
        if bytes and (bytes == bom):
            print 'icu/'+fp[2::]
        f.close()
