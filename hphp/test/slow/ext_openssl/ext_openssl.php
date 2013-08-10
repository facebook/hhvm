<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x != false, true); }

//////////////////////////////////////////////////////////////////////

function test_openssl_csr_export_to_file() {
  $csr = openssl_csr_new(null, $ignore);
  VERIFY($csr != null);

  $tmp = tempnam('/tmp', 'vmopenssltest');
  unlink($tmp);
  VS(file_get_contents($tmp), false);
  openssl_csr_export_to_file($csr, $tmp);
  VERIFY(strlen(file_get_contents($tmp)) > 400);
  unlink($tmp);
}

function test_openssl_csr_get_public_key() {
  $csr = openssl_csr_new(null, $ignore);
  VERIFY($csr != null);
  $publickey = openssl_csr_get_public_key($csr);
  VERIFY($publickey != false);
  VERIFY($publickey != null);
}

function test_openssl_csr_get_subject() {
  $csr = openssl_csr_new(null, $ignore);
  VERIFY($csr != null);
  VERIFY(openssl_csr_get_subject($csr)['O'] == "Internet Widgits Pty Ltd");
}

function test_openssl_csr_sign() {
  $dn = array(
           "countryName",
           "stateOrProvinceName",
           "localityName",
           "organizationName",
           "organizationalUnitName",
           "commonName",
           "emailAddress"
  );

  $privkeypass = "1234";
  $numberofdays = 365;

  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new($dn, $privkey);
  VERIFY($csr != null);
  $scert = openssl_csr_sign($csr, null, $privkey, $numberofdays);
  openssl_x509_export($scert, $publickey);
  openssl_pkey_export($privkey, $privatekey, $privkeypass);
  openssl_csr_export($csr, $csrStr);

  VERIFY(strlen($privatekey) > 500);
  VERIFY(strlen($publickey) > 800);
  VERIFY(strlen($csrStr) > 500);
}

function test_openssl_error_string() {
  $ret = openssl_error_string();
}

function test_openssl_free_key() {
  $csr = openssl_csr_new(null, $ignore);
  VERIFY($csr != null);
  $publickey = openssl_csr_get_public_key($csr);
  VERIFY($publickey != false);
  VERIFY($publickey != null);
  openssl_free_key($publickey);
}

function test_openssl_pkcs12_export_to_file() {
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new(null, $privkey);
  VERIFY($csr != null);
  $scert = openssl_csr_sign($csr, null, $privkey, 365);

  $tmp = tempnam('/tmp', 'vmopenssltest');
  unlink($tmp);
  VS(file_get_contents($tmp), false);
  openssl_pkcs12_export_to_file($scert, $tmp, $privkey, "1234");
  VERIFY(strlen(file_get_contents($tmp)) > 400);
  unlink($tmp);
}

function test_openssl_pkcs12_read() {
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new(null, $privkey);
  VERIFY($csr != null);
  $scert = openssl_csr_sign($csr, null, $privkey, 365);

  openssl_pkcs12_export($scert, $pkcs12, $privkey, "1234");

  VERIFY(openssl_pkcs12_read($pkcs12, $certs, "1234"));
  VERIFY(strlen($certs['cert']) > 500);
  VERIFY(strlen($certs['pkey']) > 500);
}

function test_openssl_pkcs7_sign() {
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new(null, $privkey);
  VERIFY($csr != null);
  $scert = openssl_csr_sign($csr, null, $privkey, 365);
  $pubkey = openssl_csr_get_public_key($csr);
  VERIFY($pubkey != null);

  $data = "some secret data";
  $infile = tempnam('/tmp', 'invmtestopenssl');
  $outfile = tempnam('/tmp', 'outvmtestopenssl');
  unlink($infile);
  unlink($outfile);
  file_put_contents($infile, $data);

  VERIFY(openssl_pkcs7_sign
         ($infile, $outfile, $scert, $privkey,
          array("To" => "t@facebook.com", "From" => "hzhao@facebook.com")));

  $tmp = tempnam('/tmp', 'x509vmtestopenssl');
  unlink($tmp);
  VS(file_get_contents($tmp), false);
  VERIFY(openssl_x509_export_to_file($scert, $tmp));

  VS(openssl_pkcs7_verify($outfile, 0, $infile, (array)$tmp), true);
  unlink($infile);
  unlink($outfile);
  unlink($tmp);
}

function test_openssl_pkey_export_to_file() {
  $tmp = tempnam('/tmp', 'vmopenssltest');
  unlink($tmp);
  VS(file_get_contents($tmp), false);

  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  openssl_pkey_export_to_file($privkey, $tmp, "1234");

  VERIFY(strlen(file_get_contents($tmp)) > 400);
  unlink($tmp);
}

function test_openssl_pkey_export() {
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  openssl_pkey_export($privkey, $out, "1234");
  VERIFY(strlen($out) > 500);
}

function test_openssl_pkey_free() {
  $fkey = file_get_contents(__DIR__."/test_public.pem");
  $k = openssl_pkey_get_public($fkey);
  VERIFY($k != false);
  VERIFY($k != null);
  openssl_pkey_free($k);
}

function test_openssl_pkey_get_details() {
  {
    $fkey = file_get_contents(__DIR__."/test_public.pem");
    $k = openssl_pkey_get_public($fkey);
    VERIFY($k !== false);
    VERIFY($k != null);
    VS(openssl_pkey_get_details($k)['bits'], 1024);
  }
  {
    $fkey = file_get_contents(__DIR__."/test_private.pem");
    $k = openssl_pkey_get_private($fkey);
    VERIFY($k !== false);
    VERIFY($k != null);
    VS(openssl_pkey_get_details($k)['bits'], 512);
  }
}

function test_openssl_private_encrypt() {
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new(null, $privkey);
  VERIFY($csr != null);
  $pubkey = openssl_csr_get_public_key($csr);
  VERIFY($pubkey != null);

  $data = "some secret data";
  VERIFY(openssl_private_encrypt($data, $out, $privkey));
  VERIFY(openssl_public_decrypt($out, $out2, $pubkey));
  VS($out2, $data);
}

function test_openssl_public_encrypt() {
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new(null, $privkey);
  VERIFY($csr != null);
  $pubkey = openssl_csr_get_public_key($csr);
  VERIFY($pubkey != null);

  $data = "some secret data";
  VERIFY(openssl_public_encrypt($data, $out, $pubkey));
  VERIFY(openssl_private_decrypt($out, $out2, $privkey));
  VS($out2, $data);
}

function test_openssl_seal() {
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new(null, $privkey);
  VERIFY($csr != null);
  $pubkey = openssl_csr_get_public_key($csr);
  VERIFY($pubkey != null);

  $data = "some secret messages";
  VERIFY(openssl_seal($data, $sealed, $ekeys, array($pubkey)));
  VERIFY(strlen($sealed) > 0);
  VS(count($ekeys), 1);
  VERIFY(strlen($ekeys[0]) > 0);

  VERIFY(openssl_open($sealed, $open_data, $ekeys[0], $privkey));
  VS($open_data, $data);

  VERIFY(openssl_open($sealed, $open_data, $ekeys[0], $privkey, 'RC4'));
  VS($open_data, $data);

  VERIFY(openssl_seal($data, $sealed, $ekeys, array($pubkey), 'AES-256-ECB'));
  VERIFY(strlen($sealed) > 0);
  VS(count($ekeys), 1);
  VERIFY(strlen($ekeys[0]) > 0);

  VERIFY(openssl_open($sealed, $open_data, $ekeys[0], $privkey, 'AES-256-ECB'));
  VS($open_data, $data);
}

function test_openssl_sign() {
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new(null, $privkey);
  VERIFY($csr != null);
  $pubkey = openssl_csr_get_public_key($csr);
  VERIFY($pubkey != null);

  $data = "some secret messages";
  VERIFY(openssl_sign($data, $signature, $privkey));
  VS(openssl_verify($data, $signature, $pubkey), 1);

}

function test_openssl_x509_check_private_key() {
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new(null, $privkey);
  VERIFY($csr != null);
  $scert = openssl_csr_sign($csr, null, $privkey, 365);
  VERIFY(openssl_x509_check_private_key($scert, $privkey));
}

function test_openssl_x509_checkpurpose() {
  $fcert = file_get_contents(__DIR__."/test_x509.crt");
  $cert = openssl_x509_read($fcert);
  VS(openssl_x509_checkpurpose($cert, X509_PURPOSE_SSL_CLIENT), 0);
  VS(openssl_x509_checkpurpose($cert, X509_PURPOSE_SSL_SERVER), 0);
}

function test_openssl_x509_export_to_file() {
  $fcert = file_get_contents(__DIR__."/test_x509.crt");
  $cert = openssl_x509_read($fcert);

  $tmp = tempnam('/tmp', 'x509vmopenssltest');
  unlink($tmp);
  VS(file_get_contents($tmp), false);
  VERIFY(openssl_x509_export_to_file($cert, $tmp));

  $fcert2 = file_get_contents($tmp);
  $cert2 = openssl_x509_read($fcert2);
  $info = openssl_x509_parse($cert2);
  VS($info['subject']['O'], "RSA Data Security, Inc.");

  unlink($tmp);
}

function test_openssl_x509_export() {
  $fcert = file_get_contents(__DIR__."/test_x509.crt");
  $cert = openssl_x509_read($fcert);
  VERIFY(openssl_x509_export($cert, $out));
  $cert2 = openssl_x509_read($out);
  $info = openssl_x509_parse($cert2);
  VS($info['subject']['O'], "RSA Data Security, Inc.");
}

function test_openssl_x509_free() {
  $fcert = file_get_contents(__DIR__."/test_x509.crt");
  $cert = openssl_x509_read($fcert);
  VERIFY($cert != null);
  openssl_x509_free($cert);
}

function test_openssl_x509_parse() {
  $fcert = file_get_contents(__DIR__."/test_x509.crt");
  $cert = openssl_x509_read($fcert);
  $info = openssl_x509_parse($cert);
  VS($info['subject']['O'], "RSA Data Security, Inc.");
}

function test_openssl_x509_read() {
  $fcert = file_get_contents(__DIR__."/test_x509.crt");
  $cert = openssl_x509_read($fcert);
  VERIFY($cert != null);
}

function test_openssl_encrypt() {
  $test = "OpenSSL is good for encrypting things";
  $secret = "supersecretthing";
  $cipher = "AES-256-CBC";
  $iv_len = openssl_cipher_iv_length($cipher);
  $iv = openssl_random_pseudo_bytes($iv_len);

  $data = openssl_encrypt($test, $cipher, $secret, 0, $iv);
  VS($test, openssl_decrypt($data, $cipher, $secret, 0, $iv));

  $data = openssl_encrypt($test, $cipher, $secret, OPENSSL_RAW_DATA, $iv);
  VS($test, openssl_decrypt($data, $cipher, $secret, OPENSSL_RAW_DATA, $iv));
}

function test_openssl_digest() {
  $test = "OpenSSL is also good for hashing things";

  VS(md5($test), openssl_digest($test, "md5"));
}

//////////////////////////////////////////////////////////////////////

test_openssl_csr_export_to_file();
test_openssl_csr_get_public_key();
test_openssl_csr_get_subject();
test_openssl_csr_sign();
test_openssl_error_string();
test_openssl_free_key();
test_openssl_pkcs12_export_to_file();
test_openssl_pkcs12_read();
test_openssl_pkcs7_sign();
test_openssl_pkey_export_to_file();
test_openssl_pkey_export();
test_openssl_pkey_free();
test_openssl_pkey_get_details();
test_openssl_private_encrypt();
test_openssl_public_encrypt();
test_openssl_seal();
test_openssl_sign();
test_openssl_x509_check_private_key();
test_openssl_x509_checkpurpose();
test_openssl_x509_export_to_file();
test_openssl_x509_export();
test_openssl_x509_free();
test_openssl_x509_parse();
test_openssl_x509_read();
test_openssl_encrypt();
test_openssl_digest();
