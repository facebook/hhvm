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
#pragma once

#include "hphp/util/assertions.h"
#include "hphp/zend/zend-html.h"
#include <unordered_map>
#include <map>

namespace HPHP {

enum class entity_doctype {
  html401,
  html5,
  xhtml,
  xml1,
};

typedef std::unordered_map<int, std::string> entity_doctype_table_t;
const entity_doctype_table_t* get_doctype_entity_table(entity_doctype doctype);

typedef std::map<std::pair<int, int>, std::string> entity_multicode_table_t;
const entity_multicode_table_t* get_multicode_table();

typedef std::unordered_map<int, int> charset_table_t;
const charset_table_t* get_charset_table(entity_charset charset);

}
