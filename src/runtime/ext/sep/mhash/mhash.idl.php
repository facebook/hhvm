<?php

require_once '../base.php';

///////////////////////////////////////////////////////////////////////////////

f('mhash', Variant,
  array('hash' => Int64,
        'data' => String,
        'key' => array(String, 'null_string')));

f('mhash_get_hash_name', Variant,
  array('hash' => Int64));

f('mhash_count', Int64);

f('mhash_get_block_size', Variant,
  array('hash' => Int64));

f('mhash_keygen_s2k', Variant,
  array('hash' => Int64,
        'password' => String,
        'salt' => String,
        'bytes' => Int64));
