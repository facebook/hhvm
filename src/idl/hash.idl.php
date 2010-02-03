<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('hash', Variant,
  array('algo' => String,
        'data' => String,
        'raw_output' => array(Boolean, 'false')));

f('hash_algos', StringVec);

f('hash_init', Variant,
  array('algo' => String,
        'options' => array(Int32, '0'),
        'key' => array(String, 'null_string')));

f('hash_file', Variant,
  array('algo' => String,
        'filename' => String,
        'raw_output' => array(Boolean, 'false')));

f('hash_final', String,
  array('context' => Resource,
        'raw_output' => array(Boolean, 'false')));

f('hash_hmac_file', Variant,
  array('algo' => String,
        'filename' => String,
        'key' => String,
        'raw_output' => array(Boolean, 'false')));

f('hash_hmac', Variant,
  array('algo' => String,
        'data' => String,
        'key' => String,
        'raw_output' => array(Boolean, 'false')));

f('hash_update_file', Boolean,
  array('init_context' => Resource,
        'filename' => String,
        'stream_context' => array(Resource, 'null')));

f('hash_update_stream', Int32,
  array('context' => Resource,
        'handle' => Resource,
        'length' => array(Int32, '-1')));

f('hash_update', Boolean,
  array('context' => Resource,
        'data' => String));

