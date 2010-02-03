<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// mail

f('mail', Boolean,
  array('to' => String,
        'subject' => String,
        'message' => String,
        'additional_headers' => array(String, 'null_string'),
        'additional_parameters' => array(String, 'null_string')));

f('ezmlm_hash', Int32,
  array('addr' => String));

///////////////////////////////////////////////////////////////////////////////
// mailparse

f('mailparse_msg_create', Resource);

f('mailparse_msg_free', Boolean,
  array('mimemail' => Resource));

f('mailparse_msg_parse_file', Resource,
  array('filename' => String));

f('mailparse_msg_parse', Boolean,
  array('mimemail' => Resource,
        'data' => String));

f('mailparse_msg_extract_part_file', String,
  array('mimemail' => Resource,
        'filename' => String,
        'callbackfunc' => array(String, 'null_string')));

f('mailparse_msg_extract_whole_part_file', String,
  array('mimemail' => Resource,
        'filename' => String,
        'callbackfunc' => array(String, 'null_string')));

f('mailparse_msg_extract_part', NULL,
  array('mimemail' => Resource,
        'msgbody' => String,
        'callbackfunc' => array(String, 'null_string')));

f('mailparse_msg_get_part_data', VariantMap,
  array('mimemail' => Resource));

f('mailparse_msg_get_part', Resource,
  array('mimemail' => Resource,
        'mimesection' => String));

f('mailparse_msg_get_structure', VariantMap,
  array('mimemail' => Resource));

f('mailparse_rfc822_parse_addresses', StringVec,
  array('addresses' => String));

f('mailparse_stream_encode', Boolean,
  array('sourcefp' => Resource,
        'destfp' => Resource,
        'encoding' => String));

f('mailparse_uudecode_all', StringVec,
  array('fp' => Resource));

f('mailparse_determine_best_xfer_encoding', String,
  array('fp' => Resource));
