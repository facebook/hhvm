<?hh /* -*- php -*- */
/**
 * Copyright(c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int FILEINFO_NONE;
const int FILEINFO_SYMLINK;
const int FILEINFO_MIME;
const int FILEINFO_MIME_TYPE;
const int FILEINFO_MIME_ENCODING;
const int FILEINFO_DEVICES;
const int FILEINFO_CONTINUE;
const int FILEINFO_PRESERVE_ATIME;
const int FILEINFO_RAW;

<<__PHPStdLib>>
function finfo_open(
  int $options = FILEINFO_NONE,
  ?string $magic_file = null,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function finfo_close(resource $finfo): bool;
<<__PHPStdLib>>
function finfo_set_flags(
  resource $finfo,
  int $options,
): bool {}
<<__PHPStdLib>>
function finfo_file(
  resource $finfo,
  ?string $file_name,
  int $options = FILEINFO_NONE,
  ?resource $context = null,
): string {}
<<__PHPStdLib>>
function finfo_buffer(
  resource $finfo,
  ?string $string,
  int $options = FILEINFO_NONE,
  ?resource $context = null,
): string;
<<__PHPStdLib>>
function mime_content_type(
  HH\FIXME\MISSING_PARAM_TYPE $filename,
): string {}

class finfo {
  // Methods
  public function __construct(
    int $options = FILEINFO_NONE,
    ?string $magic_file = null,
  );
  public function buffer(
    ?string $string = null,
    int $options = FILEINFO_NONE,
    ?resource $context = null,
  ): string;
  public function file(
    ?string $file_name = null,
    int $options = FILEINFO_NONE,
    ?resource $context = null,
  ): string;
  public function set_flags(int $options): bool;

}
