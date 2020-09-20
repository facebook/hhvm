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
function mail(string $to, string $subject, string $message, string $additional_headers = "", string $additional_parameters = "");
<<__PHPStdLib>>
function ezmlm_hash(string $addr);
<<__PHPStdLib>>
function mailparse_msg_create();
<<__PHPStdLib>>
function mailparse_msg_free(resource $mimemail);
<<__PHPStdLib>>
function mailparse_msg_parse_file(string $filename);
<<__PHPStdLib>>
function mailparse_msg_parse(resource $mimemail, string $data);
<<__PHPStdLib>>
function mailparse_msg_extract_part_file(resource $mimemail, $filename, $callbackfunc = "");
<<__PHPStdLib>>
function mailparse_msg_extract_whole_part_file(resource $mimemail, $filename, $callbackfunc = "");
<<__PHPStdLib>>
function mailparse_msg_extract_part(resource $mimemail, $msgbody, $callbackfunc = "");
<<__PHPStdLib>>
function mailparse_msg_get_part_data(resource $mimemail);
<<__PHPStdLib>>
function mailparse_msg_get_part(resource $mimemail, string $mimesection);
<<__PHPStdLib>>
function mailparse_msg_get_structure(resource $mimemail);
<<__PHPStdLib>>
function mailparse_rfc822_parse_addresses(string $addresses);
<<__PHPStdLib>>
function mailparse_stream_encode(resource $sourcefp, resource $destfp, string $encoding);
<<__PHPStdLib>>
function mailparse_uudecode_all(resource $fp);
<<__PHPStdLib>>
function mailparse_determine_best_xfer_encoding(resource $fp);
