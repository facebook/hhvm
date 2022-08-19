#!/bin/bash

#  Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; version 2 of the License.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

OUTPUTFILE="../mysql_sys_schema.sql"

sed -e "s/^/-- /" LICENSE > ${OUTPUTFILE}
echo "" >> ${OUTPUTFILE}

cat >> ${OUTPUTFILE} << HERE_DOC
--
-- WARNING: THIS IS A MANUALLY GENERATED FILE,
-- CHANGES NEED TO BE MADE IN THE scripts/sys_schema/*.sql FILES.
--
-- SEE scripts/sys_schema/generate_sql_file_57.sh TO GENERATE THIS FILE
--
HERE_DOC

echo "" >> ${OUTPUTFILE}

#
# Concatenate all files listed in file sys_57.txt
# Remove individual file copyrights
# Replace newlines in COMMENTs with literal \n
# Drop added trailing \n closing the COMMENT string
# Remove leading spaces
# Remove -- line comments *after removing leading spaces*
# Collapse code limited by $$
# Replace '$$' delimiter with ;
# Collapse code limited by ;
# Remove leading spaces (again), to clean up after the collapses.
# Append to the resulting file
# #

cat `grep -v "#" sys_57.txt` | \
sed -e "/Copyright/,/51 Franklin St/d" | \
sed -e "/^ *COMMENT/,/^ *'/{G;s/\n/\\\n/g;}" | \
sed -e "s/^'\\\n$/'/g" | \
sed -e "s/^ *//g" | \
sed -e "/^--/d" | \
awk -f collapse-delimiter.awk | \
sed -e "s/\\$\\$/;\n/g" | \
awk -f collapse-semicolon.awk | \
sed -e "s/^ *//g" \
>> ${OUTPUTFILE}

echo "$OUTPUTFILE updated"

