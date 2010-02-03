/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_UPLOAD_H__
#define __HPHP_UPLOAD_H__

#include <string>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define MULTIPART_CONTENT_TYPE "multipart/form-data"
#define MULTIPART_EVENT_START    0
#define MULTIPART_EVENT_FORMDATA  1
#define MULTIPART_EVENT_FILE_START  2
#define MULTIPART_EVENT_FILE_DATA  3
#define MULTIPART_EVENT_FILE_END  4
#define MULTIPART_EVENT_END    5

class Variant;

typedef struct _multipart_event_start {
  size_t  content_length;
} multipart_event_start;

typedef struct _multipart_event_formdata {
  size_t  post_bytes_processed;
  char  *name;
  char  **value;
  size_t  length;
  size_t  *newlength;
} multipart_event_formdata;

typedef struct _multipart_event_file_start {
  size_t  post_bytes_processed;
  char  *name;
  char  **filename;
} multipart_event_file_start;

typedef struct _multipart_event_file_data {
  size_t  post_bytes_processed;
  off_t  offset;
  char  *data;
  size_t  length;
  size_t  *newlength;
} multipart_event_file_data;

typedef struct _multipart_event_file_end {
  size_t  post_bytes_processed;
  char  *temp_filename;
  int  cancel_upload;
} multipart_event_file_end;

typedef struct _multipart_event_end {
  size_t  post_bytes_processed;
} multipart_event_end;

void rfc1867PostHandler(Variant &post, Variant &files, int content_length,
                        const char *data, int size,
                        const std::string boundary);

bool is_uploaded_file(const std::string filename);

///////////////////////////////////////////////////////////////////////////////
}
#endif // __HPHP_UPLOAD_H__
