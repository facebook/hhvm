<?hh     /* -*- php -*- */
/**
 * Copyright(c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
const int FILEINFO_NONE = 0;
const int FILEINFO_SYMLINK = 2;
const int FILEINFO_MIME = 1040;
const int FILEINFO_MIME_TYPE = 16;
const int FILEINFO_MIME_ENCODING = 1024;
const int FILEINFO_DEVICES = 8;
const int FILEINFO_CONTINUE = 32;
const int FILEINFO_PRESERVE_ATIME = 128;
const int FILEINFO_RAW = 256;
function finfo_open($options = null, $magic_file = null) {}
function finfo_close($finfo) {}
function finfo_set_flags($finfo, $options) {}
function finfo_file($finfo, $file_name, $options = null, $context = null) {}
function finfo_buffer($finfo ,$string, $options = FILEINFO_NONE, $context = NULL) {}
function mime_content_type($filename) {}