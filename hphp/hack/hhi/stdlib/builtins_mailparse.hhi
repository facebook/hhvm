<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function mail($to, $subject, $message, $additional_headers = null, $additional_parameters = null) { }
<<__PHPStdLib>>
function ezmlm_hash($addr) { }
<<__PHPStdLib>>
function mailparse_msg_create() { }
<<__PHPStdLib>>
function mailparse_msg_free($mimemail) { }
<<__PHPStdLib>>
function mailparse_msg_parse_file($filename) { }
<<__PHPStdLib>>
function mailparse_msg_parse($mimemail, $data) { }
<<__PHPStdLib>>
function mailparse_msg_extract_part_file($mimemail, $filename, $callbackfunc = "") { }
<<__PHPStdLib>>
function mailparse_msg_extract_whole_part_file($mimemail, $filename, $callbackfunc = "") { }
<<__PHPStdLib>>
function mailparse_msg_extract_part($mimemail, $msgbody, $callbackfunc = "") { }
<<__PHPStdLib>>
function mailparse_msg_get_part_data($mimemail) { }
<<__PHPStdLib>>
function mailparse_msg_get_part($mimemail, $mimesection) { }
<<__PHPStdLib>>
function mailparse_msg_get_structure($mimemail) { }
<<__PHPStdLib>>
function mailparse_rfc822_parse_addresses($addresses) { }
<<__PHPStdLib>>
function mailparse_stream_encode($sourcefp, $destfp, $encoding) { }
<<__PHPStdLib>>
function mailparse_uudecode_all($fp) { }
<<__PHPStdLib>>
function mailparse_determine_best_xfer_encoding($fp) { }
