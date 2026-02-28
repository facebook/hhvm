<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function mail(
  string $to,
  string $subject,
  string $message,
  string $additional_headers = "",
  string $additional_parameters = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ezmlm_hash(string $addr): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mailparse_msg_create(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mailparse_msg_free(resource $mimemail): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mailparse_msg_parse_file(
  string $filename,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mailparse_msg_parse(
  resource $mimemail,
  string $data,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mailparse_msg_extract_part_file(
  resource $mimemail,
  HH\FIXME\MISSING_PARAM_TYPE $filename,
  HH\FIXME\MISSING_PARAM_TYPE $callbackfunc = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mailparse_msg_extract_whole_part_file(
  resource $mimemail,
  HH\FIXME\MISSING_PARAM_TYPE $filename,
  HH\FIXME\MISSING_PARAM_TYPE $callbackfunc = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mailparse_msg_extract_part(
  resource $mimemail,
  HH\FIXME\MISSING_PARAM_TYPE $msgbody,
  HH\FIXME\MISSING_PARAM_TYPE $callbackfunc = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mailparse_msg_get_part_data(
  resource $mimemail,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mailparse_msg_get_part(
  resource $mimemail,
  string $mimesection,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mailparse_msg_get_structure(
  resource $mimemail,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mailparse_rfc822_parse_addresses(
  string $addresses,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mailparse_stream_encode(
  resource $sourcefp,
  resource $destfp,
  string $encoding,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mailparse_uudecode_all(resource $fp): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mailparse_determine_best_xfer_encoding(
  resource $fp,
): HH\FIXME\MISSING_RETURN_TYPE;
