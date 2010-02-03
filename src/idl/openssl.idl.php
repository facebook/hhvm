<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('openssl_csr_export_to_file', Boolean,
  array('csr' => Variant,
        'outfilename' => String,
        'notext' => array(Boolean, 'true')));

f('openssl_csr_export', Boolean,
  array('csr' => Variant,
        'out' => String | Reference,
        'notext' => array(Boolean, 'true')));

f('openssl_csr_get_public_key', Variant,
  array('csr' => Variant));

f('openssl_csr_get_subject', Variant,
  array('csr' => Variant,
        'use_shortnames' => array(Boolean, 'true')));

f('openssl_csr_new', Variant,
  array('dn' => StringMap,
        'privkey' => Object | Reference,
        'configargs' => array(Variant, 'null_variant'),
        'extraattribs' => array(Variant, 'null_variant')));

f('openssl_csr_sign', Variant,
  array('csr' => Variant,
        'cacert' => Variant,
        'priv_key' => Variant,
        'days' => Int32,
        'configargs' => array(Variant, 'null_variant'),
        'serial' => array(Int32, '0')));

f('openssl_error_string', Variant);

f('openssl_open', Boolean,
  array('sealed_data' => String,
        'open_data' => String | Reference,
        'env_key' => String,
        'priv_key_id' => Variant));

f('openssl_pkcs12_export_to_file', Boolean,
  array('x509' => Variant,
        'filename' => String,
        'priv_key' => Variant,
        'pass' => String,
        'args' => array(Variant, 'null_variant')));

f('openssl_pkcs12_export', Boolean,
  array('x509' => Variant,
        'out' => String | Reference,
        'priv_key' => Variant,
        'pass' => String,
        'args' => array(Variant, 'null_variant')));

f('openssl_pkcs12_read', Boolean,
  array('pkcs12' => String,
        'certs' => StringVec | Reference,
        'pass' => String));

f('openssl_pkcs7_decrypt', Boolean,
  array('infilename' => String,
        'outfilename' => String,
        'recipcert' => Variant,
        'recipkey' => array(Variant, 'null_variant')));

f('openssl_pkcs7_encrypt', Boolean,
  array('infilename' => String,
        'outfilename' => String,
        'recipcerts' => Variant,
        'headers' => StringVec,
        'flags' => array(Int32, '0'),
        'cipherid' => array(Int32, 'k_OPENSSL_CIPHER_RC2_40')));

f('openssl_pkcs7_sign', Boolean,
  array('infilename' => String,
        'outfilename' => String,
        'signcert' => Variant,
        'privkey' => Variant,
        'headers' => Variant,
        'flags' => array(Int32, 'k_PKCS7_DETACHED'),
        'extracerts' => array(String, 'null_string')));

f('openssl_pkcs7_verify', Variant,
  array('filename' => String,
        'flags' => Int32,
        'outfilename' => array(String, 'null_string'),
        'cainfo' => array(StringVec, 'null_array'),
        'extracerts' => array(String, 'null_string'),
        'content' => array(String, 'null_string')));

f('openssl_pkey_export_to_file', Boolean,
  array('key' => Variant,
        'outfilename' => String,
        'passphrase' => array(String, 'null_string'),
        'configargs' => array(Variant, 'null_variant')));

f('openssl_pkey_export', Boolean,
  array('key' => Variant,
        'out' => String | Reference,
        'passphrase' => array(String, 'null_string'),
        'configargs' => array(Variant, 'null_variant')));

f('openssl_pkey_free', NULL,
  array('key' => Object));

f('openssl_free_key', NULL,
  array('key' => Object));

f('openssl_pkey_get_details', VariantMap,
  array('key' => Object));

f('openssl_pkey_get_private', Variant,
  array('key' => Variant,
        'passphrase' => array(String, 'null_string')));

f('openssl_get_privatekey', Variant,
  array('key' => Variant,
        'passphrase' => array(String, 'null_string')));

f('openssl_pkey_get_public', Variant,
  array('certificate' => Variant));

f('openssl_get_publickey', Variant,
  array('certificate' => Variant));

f('openssl_pkey_new', Object,
  array('configargs' => array(Variant, 'null_variant')));

f('openssl_private_decrypt', Boolean,
  array('data' => String,
        'decrypted' => String | Reference,
        'key' => Variant,
        'padding' => array(Int32, 'k_OPENSSL_PKCS1_PADDING')));

f('openssl_private_encrypt', Boolean,
  array('data' => String,
        'crypted' => String | Reference,
        'key' => Variant,
        'padding' => array(Int32, 'k_OPENSSL_PKCS1_PADDING')));

f('openssl_public_decrypt', Boolean,
  array('data' => String,
        'decrypted' => String | Reference,
        'key' => Variant,
        'padding' => array(Int32, 'k_OPENSSL_PKCS1_PADDING')));

f('openssl_public_encrypt', Boolean,
  array('data' => String,
        'crypted' => String | Reference,
        'key' => Variant,
        'padding' => array(Int32, 'k_OPENSSL_PKCS1_PADDING')));

f('openssl_seal', Variant,
  array('data' => String,
        'sealed_data' => String | Reference,
        'env_keys' => StringVec | Reference,
        'pub_key_ids' => StringVec));

f('openssl_sign', Boolean,
  array('data' => String,
        'signature' => String | Reference,
        'priv_key_id' => Variant,
        'signature_alg' => array(Int32, 'k_OPENSSL_ALGO_SHA1')));

f('openssl_verify', Variant,
  array('data' => String,
        'signature' => String,
        'pub_key_id' => Variant,
        'signature_alg' => array(Int32, 'k_OPENSSL_ALGO_SHA1')));

f('openssl_x509_check_private_key', Boolean,
  array('cert' => Variant,
        'key' => Variant));

f('openssl_x509_checkpurpose', Int32,
  array('x509cert' => Variant,
        'purpose' => Int32,
        'cainfo' => array(StringVec, 'null_array'),
        'untrustedfile' => array(String, 'null_string')));

f('openssl_x509_export_to_file', Boolean,
  array('x509' => Variant,
        'outfilename' => String,
        'notext' => array(Boolean, 'true')));

f('openssl_x509_export', Boolean,
  array('x509' => Variant,
        'output' => String | Reference,
        'notext' => array(Boolean, 'true')));

f('openssl_x509_free', NULL,
  array('x509cert' => Object));

f('openssl_x509_parse', Variant,
  array('x509cert' => Variant,
        'shortnames' => array(Boolean, 'true')));

f('openssl_x509_read', Variant,
  array('x509certdata' => Variant));
