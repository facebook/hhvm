<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
function mail($to, $subject, $message, $additional_headers = null, $additional_parameters = null) { }
function ezmlm_hash($addr) { }
function mailparse_msg_create() { }
function mailparse_msg_free($mimemail) { }
function mailparse_msg_parse_file($filename) { }
function mailparse_msg_parse($mimemail, $data) { }
function mailparse_msg_extract_part_file($mimemail, $filename, $callbackfunc = "") { }
function mailparse_msg_extract_whole_part_file($mimemail, $filename, $callbackfunc = "") { }
function mailparse_msg_extract_part($mimemail, $msgbody, $callbackfunc = "") { }
function mailparse_msg_get_part_data($mimemail) { }
function mailparse_msg_get_part($mimemail, $mimesection) { }
function mailparse_msg_get_structure($mimemail) { }
function mailparse_rfc822_parse_addresses($addresses) { }
function mailparse_stream_encode($sourcefp, $destfp, $encoding) { }
function mailparse_uudecode_all($fp) { }
function mailparse_determine_best_xfer_encoding($fp) { }
