<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('apc_add', Boolean,
  array('key' => String,
        'var' => Variant,
        'ttl' => array(Int64, '0'),
        'cache_id' => array(Int64, '0')));

f('apc_store', Boolean,
  array('key' => String,
        'var' => Variant,
        'ttl' => array(Int64, '0'),
        'cache_id' => array(Int64, '0')));

f('apc_fetch', Variant,
  array('key' => Variant,
        'success' => array(Boolean | Reference, 'null'),
        'cache_id' => array(Int64, '0')));

f('apc_delete', Variant,
  array('key' => Variant,
        'cache_id' => array(Int64, '0')));

f('apc_compile_file', Boolean,
  array('filename' => String,
        'atomic' => array(Boolean, 'true'),
        'cache_id' => array(Int64, '0')));

f('apc_cache_info', Variant,
  array('cache_id' => array(Int64, '0'),
        'limited' => array(Boolean, 'false')));

f('apc_clear_cache', Boolean,
  array('cache_id' => array(Int64, '0')));

f('apc_define_constants', Boolean,
  array('key' => String,
        'constants' => String,
        'case_sensitive' => array(Boolean, 'true'),
        'cache_id' => array(Int64, '0')));

f('apc_load_constants', Boolean,
  array('key' => String,
        'case_sensitive' => array(Boolean, 'true'),
        'cache_id' => array(Int64, '0')));

f('apc_sma_info', VariantMap,
  array('limited' => array(Boolean, 'false')));

f('apc_filehits', StringVec);

f('apc_delete_file', Variant,
  array('keys' => Variant,
        'cache_id' => array(Int64, '0')));

f('apc_inc', Variant,
  array('key' => String,
        'step' => array(Int64, '1'),
        'success' => array(Boolean | Reference, 'null'),
        'cache_id' => array(Int64, '0')));

f('apc_dec', Variant,
  array('key' => String,
        'step' => array(Int64, '1'),
        'success' => array(Boolean | Reference, 'null'),
        'cache_id' => array(Int64, '0')));

f('apc_cas', Boolean,
  array('key' => String,
        'old_cas' => Int64,
        'new_cas' => Int64,
        'cache_id' => array(Int64, '0')));

f('apc_bin_dump', Variant,
  array('cache_id' => array(Int64, '0'),
        'filter' => array(Variant, 'null_variant')));

f('apc_bin_load', Boolean,
  array('data' => String,
        'flags' => array(Int64, '0'),
        'cache_id' => array(Int64, '0')));

f('apc_bin_dumpfile', Variant,
  array('cache_id' => Int64,
        'filter' => Variant,
        'filename' => String,
        'flags' => array(Int64, '0'),
        'context' => array(Resource, 'null')));

f('apc_bin_loadfile', Boolean,
  array('filename' => String,
        'context' => array(Resource, 'null'),
        'flags' => array(Int64, '0'),
        'cache_id' => array(Int64, '0')));
