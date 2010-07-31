<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// fb extension

k("FB_UNSERIALIZE_NONSTRING_VALUE", Int64);
k("FB_UNSERIALIZE_UNEXPECTED_END", Int64);
k("FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE", Int64);
k("FB_UNSERIALIZE_UNEXPECTED_ARRAY_KEY_TYPE", Int64);

f('fb_thrift_serialize', Variant,
  array('thing' => Variant));

f('fb_thrift_unserialize', Variant,
  array('thing' => Variant,
        'success' => Boolean | Reference,
        'errcode' => array(Int32 | Reference, 'null_variant')));

f('fb_serialize', Variant,
  array('thing' => Variant));

f('fb_unserialize', Variant,
  array('thing' => Variant,
        'success' => Boolean | Reference,
        'errcode' => array(Int32 | Reference, 'null_variant')));

f('fb_renamed_functions', NULL,
  array('names' => StringVec));

f('fb_rename_function', Boolean,
  array('orig_func_name' => String,
        'new_func_name' => String));

f('fb_utf8ize', Boolean,
  array('input' => String | Reference));

f('fb_call_user_func_safe', VariantVec,
  array('function' => Variant),
  VariableArguments, 'hphp_opt_fb_call_user_func');

f('fb_call_user_func_safe_return', Variant,
  array('function' => Variant,
        'def' => Variant),
  VariableArguments, 'hphp_opt_fb_call_user_func');

f('fb_call_user_func_array_safe', VariantVec,
  array('function' => Variant,
        'params' => VariantVec), DefaultFlags);

f('fb_get_code_coverage', Variant);

///////////////////////////////////////////////////////////////////////////////
// xhprof

k("XHPROF_FLAGS_NO_BUILTINS", Int64);
k("XHPROF_FLAGS_CPU", Int64);
k("XHPROF_FLAGS_MEMORY", Int64);
k("XHPROF_FLAGS_VTSC", Int64);

f('xhprof_enable',  NULL,
  array('flags' => Int32,
        'args' => array(StringVec, 'null_array')));

f('xhprof_disable', Variant);

f('xhprof_sample_enable',  NULL);
f('xhprof_sample_disable',  Variant);

///////////////////////////////////////////////////////////////////////////////
// php_query

f('fb_load_local_databases', NULL,
  array('servers' => VariantMap));

f('fb_parallel_query', VariantMap,
  array('sql_map' => VariantMap,
        'max_thread' => array(Int32, '50'),
        'combine_result' => array(Boolean, 'true'),
        'retry_query_on_fail' => array(Boolean, 'true'),
        'connect_timeout' => array(Int32, '-1'),
        'read_timeout' => array(Int32, '-1'),
        'timeout_in_ms' => array(Boolean, 'false')));

f('fb_crossall_query', VariantMap,
  array('sql' => String,
        'max_thread' => array(Int32, '50'),
        'retry_query_on_fail' => array(Boolean, 'true'),
        'connect_timeout' => array(Int32, '-1'),
        'read_timeout' => array(Int32, '-1'),
        'timeout_in_ms' => array(Boolean, 'false')));

/////////////
// tainting

f('fb_taint', null, array('str' => String));

f('fb_untaint', null, array('str' => String));

f('fb_is_tainted', Boolean, array('str' => String));

///////////////////////////////////////////////////////////////////////////////
// const index functions

f('fb_const_fetch', Variant,
  array('key' => Variant));

