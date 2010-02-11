<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('iconv_mime_encode', Variant,
  array('field_name' => String,
        'field_value' => String,
        'preferences' => array(Variant, 'null_variant')));

f('iconv_mime_decode', Variant,
  array('encoded_string' => String,
        'mode' => array(Int32, '0'),
        'charset' => array(String, 'null_string')));

f('iconv_mime_decode_headers', Variant,
  array('encoded_headers' => String,
        'mode' => array(Int32, '0'),
        'charset' => array(String, 'null_string')));

f('iconv_get_encoding', Variant,
  array('type' => array(String, '"all"')));

f('iconv_set_encoding', Boolean,
  array('type' => String,
        'charset' => String));

f('iconv', Variant,
  array('in_charset' => String,
        'out_charset' => String,
        'str' => String));

f('iconv_strlen', Variant,
  array('str' => String,
        'charset' => array(String, 'null_string')));

f('iconv_strpos', Variant,
  array('haystack' => String,
        'needle' => String,
        'offset' => array(Int32, '0'),
        'charset' => array(String, 'null_string')));

f('iconv_strrpos', Variant,
  array('haystack' => String,
        'needle' => String,
        'charset' => array(String, 'null_string')));

f('iconv_substr', Variant,
  array('str' => String,
        'offset' => Int32,
        'length' => array(Int32, 'INT_MAX'),
        'charset' => array(String, 'null_string')));

f('ob_iconv_handler', String,
  array('contents' => String,
        'status' => Int32));
