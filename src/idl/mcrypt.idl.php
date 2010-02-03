<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('mcrypt_module_open', Variant,
  array('algorithm' => String,
        'algorithm_directory' => String,
        'mode' => String,
        'mode_directory' => String));

f('mcrypt_module_close', Boolean,
  array('td' => Resource));

f('mcrypt_list_algorithms', StringVec,
  array('lib_dir' => array(String, 'null_string')));

f('mcrypt_list_modes', StringVec,
  array('lib_dir' => array(String, 'null_string')));

f('mcrypt_module_get_algo_block_size', Int32,
  array('algorithm' => String,
        'lib_dir' => array(String, 'null_string')));

f('mcrypt_module_get_algo_key_size', Int32,
  array('algorithm' => String,
        'lib_dir' => array(String, 'null_string')));

f('mcrypt_module_get_supported_key_sizes', Int64Vec,
  array('algorithm' => String,
        'lib_dir' => array(String, 'null_string')));

f('mcrypt_module_is_block_algorithm_mode', Boolean,
  array('mode' => String,
        'lib_dir' => array(String, 'null_string')));

f('mcrypt_module_is_block_algorithm', Boolean,
  array('algorithm' => String,
        'lib_dir' => array(String, 'null_string')));

f('mcrypt_module_is_block_mode', Boolean,
  array('mode' => String,
        'lib_dir' => array(String, 'null_string')));

f('mcrypt_module_self_test', Boolean,
  array('algorithm' => String,
        'lib_dir' => array(String, 'null_string')));

f('mcrypt_create_iv', Variant,
  array('size' => Int32,
        'source' => array(Int32, '0')));

f('mcrypt_encrypt', Variant,
  array('cipher' => String,
        'key' => String,
        'data' => String,
        'mode' => String,
        'iv' => array(String, 'null_string')));

f('mcrypt_decrypt', Variant,
  array('cipher' => String,
        'key' => String,
        'data' => String,
        'mode' => String,
        'iv' => array(String, 'null_string')));

f('mcrypt_cbc', Variant,
  array('cipher' => String,
        'key' => String,
        'data' => String,
        'mode' => Int32,
        'iv' => array(String, 'null_string')));

f('mcrypt_cfb', Variant,
  array('cipher' => String,
        'key' => String,
        'data' => String,
        'mode' => Int32,
        'iv' => array(String, 'null_string')));

f('mcrypt_ecb', Variant,
  array('cipher' => String,
        'key' => String,
        'data' => String,
        'mode' => Int32,
        'iv' => array(String, 'null_string')));

f('mcrypt_ofb', Variant,
  array('cipher' => String,
        'key' => String,
        'data' => String,
        'mode' => Int32,
        'iv' => array(String, 'null_string')));

f('mcrypt_get_block_size', Variant,
  array('cipher' => String,
        'module' => array(String, 'null_string')));

f('mcrypt_get_cipher_name', Variant,
  array('cipher' => String));

f('mcrypt_get_iv_size', Variant,
  array('cipher' => String,
        'mode' => String));

f('mcrypt_get_key_size', Int32,
  array('cipher' => String,
        'module' => String));

f('mcrypt_enc_get_algorithms_name',     String,    array('td' => Resource));
f('mcrypt_enc_get_block_size',          Int32,     array('td' => Resource));
f('mcrypt_enc_get_iv_size',             Int32,     array('td' => Resource));
f('mcrypt_enc_get_key_size',            Int32,     array('td' => Resource));
f('mcrypt_enc_get_modes_name',          String,    array('td' => Resource));
f('mcrypt_enc_get_supported_key_sizes', Int64Vec,  array('td' => Resource));
f('mcrypt_enc_is_block_algorithm_mode', Boolean,   array('td' => Resource));
f('mcrypt_enc_is_block_algorithm',      Boolean,   array('td' => Resource));
f('mcrypt_enc_is_block_mode',           Boolean,   array('td' => Resource));
f('mcrypt_enc_self_test',               Int32,     array('td' => Resource));

f('mcrypt_generic', Variant,
  array('td' => Resource,
        'data' => String));

f('mcrypt_generic_init', Int32,
  array('td' => Resource,
        'key' => String,
        'iv' => String));

f('mdecrypt_generic', Variant,
  array('td' => Resource,
        'data' => String));

f('mcrypt_generic_deinit', Boolean,
  array('td' => Resource));

f('mcrypt_generic_end', Boolean,
  array('td' => Resource));


