# @lint-avoid-python-3-compatibility-imports.


# this script uses ents_*.txt and mappings/* for generating ../html-table.cpp
# USAGE:
#         python generate-html-table.py > ../html-table.cpp
#

import sys
from collections import defaultdict

basic_codes = set()


def read_doc_type(file_name):
    res = []
    ents = {}
    with open(file_name, 'rt') as f:
        for s in f.xreadlines():
            ss = s.strip('\n').split()
            code_point = int(ss[1], 16)
            ent = ss[0]
            if len(ss) > 2:
                continue

            # we skip basic code as they are added explicitly
            if code_point in basic_codes:
                continue
            res.append((code_point, ent))
    return res

def add_basic_codes(file_name):
    data = read_doc_type(file_name)
    for code_point, entity in data:
        basic_codes.add(code_point)


def write_table(table_name, table_data):
    table_data.sort()
    print 'static const entity_doctype_table_t %s {' % table_name
    for code_point, entity in table_data:
        print '  {%s, "%s"},' % (hex(code_point), entity)
    print '};'
    print

def write_mapping_table(table_name, table_data):
    print 'static const charset_table_t %s {' % table_name
    for a, b in table_data:
        print '  {%s, %s},' % (hex(a), hex(b))
    print '};'
    print


def read_mapping(file_name):
    res = []
    with open(file_name, 'rt') as f:
        for s in f.xreadlines():
            if s[0] == '#':
                continue
            ss = s.strip('\n').split('\t')
            if 'x' not in ss[1]:
                continue
            a = int(ss[0], 16)
            b = int(ss[1], 16)
            res.append((a, b))
    return res


def output_mapping(file_name, table_name):
    data = read_mapping(file_name)
    write_mapping_table(table_name, data)

def make(file_name, table_name):
    data = read_doc_type(file_name)
    write_table(table_name, data)

def filter_clashing_codes(data, good_values):
    data.sort()
    num_times = defaultdict(int)
    for code_point, ent in data:
        num_times[code_point] += 1

    res = []
    for code_point, ent in data:
        if num_times[code_point] > 1:
            if ent not in good_values:
                continue
        res.append((code_point, ent))
    return res


# Some of the entities in html5 share the same code points
# (e.g. "midast" and "ast" share 0x42)
# In zend's php which one to use is chosen by usort,
# which has undefined behaviour. To mimic this behaviour we use
# full list of html5 entities, prdocued by following script:
# <?php
# $a = get_html_translation_table(HTML_ENTITIES, ENT_HTML5 | ENT_QUOTES);
# foreach($a as $k => $v) {
#     echo "$v\n";
# }
def read_values(file_name):
    ents = set()
    with open(file_name) as f:
        for s in f.xreadlines():
            ent = s.strip('&;\n')
            ents.add(ent)
    return ents

good_values = read_values('zend_html5_values.txt')

def make_html5(file_name, table_name):
    data = read_doc_type(file_name)
    filtered_data = filter_clashing_codes(data, good_values)
    write_table(table_name, filtered_data)


def read_multicode_table(file_name):
    table_data = []
    with open(file_name, 'rt') as f:
        for s in f.xreadlines():
            ss = s.strip('\n').split()
            if len(ss) != 3:
                continue

            ent = ss[0]
            code1 = int(ss[1], 16)
            code2 = int(ss[2], 16)

            table_data.append(((code1, code2), ent))
    return table_data

def write_multicode_table(table_name, table_data):
    table_data.sort()
    print 'static const entity_multicode_table_t %s {' % table_name
    for ((code1, code2), entity) in table_data:
        print '  {{%s, %s}, "%s"},' % (hex(code1), hex(code2), entity)
    print '};'
    print


def make_multicode_table(file_name, table_name):
    data = read_multicode_table(file_name)
    filtered_data = filter_clashing_codes(data, good_values)
    write_multicode_table(table_name, filtered_data)


header = """
/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/zend/html-table.h"

/**************************************************************************
***************************************************************************
**        THIS FILE IS AUTOMATICALLY GENERATED. DO NOT MODIFY IT.        **
***************************************************************************
** Please change html_tables/generate-html-table.py instead and then     **
** run it in order to generate this file                                 **
***************************************************************************
**************************************************************************/

namespace HPHP {
"""

print header

add_basic_codes('ents_basic.txt')
add_basic_codes('ents_basic_apos.txt')

output_mapping('mappings/8859-1.TXT', 'entity_table_cs_8859_1')
output_mapping('mappings/CP1252.TXT', 'entity_table_cs_cp1252')

# table for encoding csjis is esentially empty
write_mapping_table('entity_table_cs_sjis', [])

make('ents_html401.txt', 'entity_table_html401')
make('ents_xhtml.txt', 'entity_table_xhtml')

# xml1 table essentially doesn't contain anything except basic symbols
# which are added separately
write_table('entity_table_xml1', [])

make_html5('ents_html5.txt', 'entity_table_html5')
make_multicode_table('ents_html5.txt', 'entity_multicode_table_html5')

footer = """
const entity_doctype_table_t* get_doctype_entity_table(entity_doctype doctype) {
  switch(doctype) {
    case entity_doctype::html401: return &entity_table_html401;
    case entity_doctype::html5: return &entity_table_html5;
    case entity_doctype::xml1: return &entity_table_xml1;
    case entity_doctype::xhtml: return &entity_table_xhtml;
  }
  not_reached();
}

const charset_table_t* get_charset_table(entity_charset charset) {
  using namespace entity_charset_enum;
  switch (charset) {
    case cs_8859_1: return &entity_table_cs_8859_1;
    case cs_cp1252: return &entity_table_cs_cp1252;
    case cs_sjis: return &entity_table_cs_sjis;
    default: return &entity_table_cs_8859_1;
  }
}

const entity_multicode_table_t* get_multicode_table() {
  return &entity_multicode_table_html5;
}

} // namespace HPHP
"""
print footer
