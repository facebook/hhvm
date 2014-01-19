/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Wez Furlong <wez@thebrainroom.com>                          |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#include "php.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/vm/request-arena.h"

PHPAPI php_stream *_php_stream_fopen_tmpfile(int dummy STREAMS_DC TSRMLS_DC) {
  FILE *f = tmpfile();
  if (f) {
    auto* file = NEWOBJ(HPHP::PlainFile)(f);
    auto* stream = new (HPHP::request_arena()) php_stream(file);
    stream->hphp_file->incRefCount();
    return stream;
  }
  return nullptr;
}
