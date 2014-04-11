<?php

$test_values = array(
  OPENSSL_ALGO_SHA1,
  OPENSSL_ALGO_MD5,
  OPENSSL_ALGO_MD4,
);

function run_test($value) {
  $data = 'DATA TO SIGN';
  $signature = 'SIGNATURE';

  $key = openssl_pkey_new();
  $private_key = openssl_get_privatekey($key);
  $public_key = openssl_pkey_get_details($key)['key'];

  try {
    var_dump(openssl_sign($data, $signature, $private_key, $value));
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    var_dump(openssl_verify($data, $signature, $public_key, $value));
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  openssl_free_key($key);
}

$test_values = array_merge($test_values, openssl_get_md_methods());
foreach ($test_values as $test_value) {
  run_test($test_value);
}
