<?php

include_once 'base.php';

p(
<<<END
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/multi.h>
END
);

///////////////////////////////////////////////////////////////////////////////

f('curl_init', Variant,
  array('url' => array(String, 'null_string')));

f('curl_copy_handle', Variant,
  array('ch' => Resource));

f('curl_version', Variant,
  array('uversion' => array(Int32, 'CURLVERSION_NOW')));

f('curl_setopt', Boolean,
  array('ch' => Resource,
        'option' => Int32,
        'value' => Variant));

f('curl_setopt_array', Boolean,
  array('ch' => Resource,
        'options' => VariantVec));

f('curl_exec', Variant,
  array('ch' => Resource));

f('curl_getinfo', Variant,
  array('ch' => Resource,
        'opt' => array(Int32, '0')));

f('curl_errno', Variant,
  array('ch' => Resource));

f('curl_error', Variant,
  array('ch' => Resource));

f('curl_close', Variant,
  array('ch' => Resource));

///////////////////////////////////////////////////////////////////////////////

f('curl_multi_init', Resource);

f('curl_multi_add_handle', Variant,
  array('mh' => Resource,
        'ch' => Resource));

f('curl_multi_remove_handle', Variant,
  array('mh' => Resource,
        'ch' => Resource));

f('curl_multi_exec', Variant,
  array('mh' => Resource,
        'still_running' => Int32 | Reference));

f('curl_multi_select', Variant,
  array('mh' => Resource,
        'timeout' => array(Double, '1.0')));

f('curl_multi_getcontent', Variant,
  array('ch' => Resource));

f('curl_multi_info_read', Variant,
  array('mh' => Resource,
        'msgs_in_queue' => array(Int32 | Reference, 'null')));

f('curl_multi_close', Variant,
  array('mh' => Resource));

///////////////////////////////////////////////////////////////////////////////

f('evhttp_set_cache', NULL,
  array('address' => String,
        'max_conn' => Int32,
        'port' => array(Int32, '80')));

f('evhttp_get', Variant,
  array('url' => String,
        'headers' => array(StringVec, 'null_array'),
        'timeout' => array(Int32, '5')));

f('evhttp_post', Variant,
  array('url' => String,
        'data' => String,
        'headers' => array(StringVec, 'null_array'),
        'timeout' => array(Int32, '5')));

f('evhttp_async_get', Variant,
  array('url' => String,
        'headers' => array(StringVec, 'null_array'),
        'timeout' => array(Int32, '5')));

f('evhttp_async_post', Variant,
  array('url' => String,
        'data' => String,
        'headers' => array(StringVec, 'null_array'),
        'timeout' => array(Int32, '5')));

f('evhttp_recv', Variant,
  array('handle' => Object));
