<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('base64_decode', Variant,
  array('data' => String,
        'strict' => array(Boolean, 'false')));

f('base64_encode', String,
  array('data' => String));

f('get_headers', Variant,
  array('url' => String,
        'format' => array(Int32, '0')));

f('get_meta_tags', StringVec,
  array('filename' => String,
        'use_include_path' => array(Boolean, 'false')));

f('http_build_query', String,
  array('formdata' => Variant,
        'numeric_prefix' => array(String, 'null_string'),
        'arg_separator' => array(String, '"&"')));

f('parse_url', Variant,
  array('url' => String,
        'component' => array(Int32, '-1')));

f('rawurldecode', String, array('str' => String));
f('rawurlencode', String, array('str' => String));
f('urldecode',    String, array('str' => String));
f('urlencode',    String, array('str' => String));
