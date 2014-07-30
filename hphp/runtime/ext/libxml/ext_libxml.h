/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_LIBXML_H_
#define incl_HPHP_EXT_LIBXML_H_

#include "hphp/runtime/base/base-includes.h"

#include <libxml/parser.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool libxml_use_internal_error();
void libxml_add_error(const std::string& msg);
String libxml_get_valid_file_path(const String& source);
String libxml_get_valid_file_path(const char* source);

int libxml_streams_IO_read(void* context, char* buffer, int len);
int libxml_streams_IO_write(void* context, const char* buffer, int len);
int libxml_streams_IO_close(void* context);

void php_libxml_node_free(xmlNodePtr node);
void php_libxml_node_free_resource(xmlNodePtr node);

#define LIBXML_SAVE_NOEMPTYTAG 1<<2

///////////////////////////////////////////////////////////////////////////////
}
#endif
