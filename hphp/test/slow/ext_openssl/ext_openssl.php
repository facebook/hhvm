<?hh

function VS($x, $y) :mixed{
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) :mixed{ VS($x != false, true); }

//////////////////////////////////////////////////////////////////////

function test_openssl_csr_export_to_file() :mixed{
  $ignore = null;
  $csr = openssl_csr_new(null, inout $ignore);
  VERIFY($csr != null);

  $tmp = tempnam(sys_get_temp_dir(), 'vmopenssltest');
  unlink($tmp);
  VS(file_get_contents($tmp), false);
  openssl_csr_export_to_file($csr, $tmp);
  VERIFY(strlen(file_get_contents($tmp)) > 400);
  unlink($tmp);
}

function test_openssl_csr_get_public_key() :mixed{
  $ignore = null;
  $csr = openssl_csr_new(null, inout $ignore);
  VERIFY($csr != null);
  $publickey = openssl_csr_get_public_key($csr);
  VERIFY($publickey != false);
  VERIFY($publickey != null);
}

function test_openssl_csr_get_subject() :mixed{
  $ignore = null;
  $csr = openssl_csr_new(null, inout $ignore);
  VERIFY($csr != null);
  $subject = openssl_csr_get_subject($csr);
  VERIFY(is_darray($subject));
  VERIFY($subject['O'] == "Internet Widgits Pty Ltd" ||
         $subject['O'] == "Default Company Ltd");
}

function test_openssl_csr_sign() :mixed{
  $dn = vec[
           "countryName",
           "stateOrProvinceName",
           "localityName",
           "organizationName",
           "organizationalUnitName",
           "commonName",
           "emailAddress"
  ];

  $privkeypass = "1234";
  $numberofdays = 365;

  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new(darray($dn), inout $privkey);
  VERIFY($csr != null);
  $scert = openssl_csr_sign($csr, null, $privkey, $numberofdays);
  $publickey = null;
  openssl_x509_export($scert, inout $publickey);
  $privatekey = null;
  openssl_pkey_export($privkey, inout $privatekey, $privkeypass);
  $csrStr = null;
  openssl_csr_export($csr, inout $csrStr);

  VERIFY(strlen($privatekey) > 500);
  VERIFY(strlen($publickey) > 800);
  VERIFY(strlen($csrStr) > 500);
}

function test_openssl_error_string() :mixed{
  $ret = openssl_error_string();
}

function test_openssl_free_key() :mixed{
  $ignore = null;
  $csr = openssl_csr_new(null, inout $ignore);
  VERIFY($csr != null);
  $publickey = openssl_csr_get_public_key($csr);
  VERIFY($publickey != false);
  VERIFY($publickey != null);
  openssl_free_key($publickey);
}

function test_openssl_pkcs12_export_to_file() :mixed{
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new(null, inout $privkey);
  VERIFY($csr != null);
  $scert = openssl_csr_sign($csr, null, $privkey, 365);

  $tmp = tempnam(sys_get_temp_dir(), 'vmopenssltest');
  unlink($tmp);
  VS(file_get_contents($tmp), false);
  openssl_pkcs12_export_to_file($scert, $tmp, $privkey, "1234");
  VERIFY(strlen(file_get_contents($tmp)) > 400);
  unlink($tmp);
}

function test_openssl_pkcs12_read() :mixed{
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new(null, inout $privkey);
  VERIFY($csr != null);
  $scert = openssl_csr_sign($csr, null, $privkey, 365);

  $pkcs12 = null;
  openssl_pkcs12_export($scert, inout $pkcs12, $privkey, "1234");

  $certs = null;
  VERIFY(openssl_pkcs12_read($pkcs12, inout $certs, "1234"));
  VERIFY(is_darray($certs));
  VERIFY(strlen($certs['cert']) > 500);
  VERIFY(strlen($certs['pkey']) > 500);
}

function test_openssl_pkcs7_sign() :mixed{
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new(null, inout $privkey);
  VERIFY($csr != null);
  $scert = openssl_csr_sign($csr, null, $privkey, 365);
  $pubkey = openssl_csr_get_public_key($csr);
  VERIFY($pubkey != null);

  $data = "some secret data";
  $infile = tempnam(sys_get_temp_dir(), 'invmtestopenssl');
  $outfile = tempnam(sys_get_temp_dir(), 'outvmtestopenssl');
  unlink($infile);
  unlink($outfile);
  file_put_contents($infile, $data);

  VERIFY(openssl_pkcs7_sign
         ($infile, $outfile, $scert, $privkey,
          dict["To" => "t@facebook.com", "From" => "hzhao@facebook.com"]));

  $tmp = tempnam(sys_get_temp_dir(), 'x509vmtestopenssl');
  unlink($tmp);
  VS(file_get_contents($tmp), false);
  VERIFY(openssl_x509_export_to_file($scert, $tmp));

  VS(openssl_pkcs7_verify($outfile, 0, $infile, vec[$tmp]), true);
  unlink($infile);
  unlink($outfile);
  unlink($tmp);
}

function test_openssl_pkey_export_to_file() :mixed{
  $tmp = tempnam(sys_get_temp_dir(), 'vmopenssltest');
  unlink($tmp);
  VS(file_get_contents($tmp), false);

  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  openssl_pkey_export_to_file($privkey, $tmp, "1234");

  VERIFY(strlen(file_get_contents($tmp)) > 400);
  unlink($tmp);
}

function test_openssl_pkey_export() :mixed{
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $out = null;
  openssl_pkey_export($privkey, inout $out, "1234");
  VERIFY(strlen($out) > 500);
}

function test_openssl_pkey_free() :mixed{
  $fkey = file_get_contents(__DIR__."/test_public.pem");
  $k = openssl_pkey_get_public($fkey);
  VERIFY($k != false);
  VERIFY($k != null);
  openssl_pkey_free($k);
}

function test_openssl_pkey_get_details() :mixed{
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

function test_openssl_private_encrypt() :mixed{
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new(null, inout $privkey);
  VERIFY($csr != null);
  $pubkey = openssl_csr_get_public_key($csr);
  VERIFY($pubkey != null);

  $data = "some secret data";
  $out = null;
  $out2 = null;
  VERIFY(openssl_private_encrypt($data, inout $out, $privkey));
  VERIFY(openssl_public_decrypt($out, inout $out2, $pubkey));
  VS($out2, $data);
}

function test_openssl_public_encrypt() :mixed{
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new(null, inout $privkey);
  VERIFY($csr != null);
  $pubkey = openssl_csr_get_public_key($csr);
  VERIFY($pubkey != null);

  $data = "some secret data";
  $out = null;
  $out2 = null;
  VERIFY(openssl_public_encrypt($data, inout $out, $pubkey));
  VERIFY(openssl_private_decrypt($out, inout $out2, $privkey));
  VS($out2, $data);
}

function test_openssl_seal() :mixed{
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new(null, inout $privkey);
  VERIFY($csr != null);
  $pubkey = openssl_csr_get_public_key($csr);
  VERIFY($pubkey != null);

  $data = "some secret messages";
  $sealed = null;
  $ekeys = null;
  $iv = null;
  VERIFY(openssl_seal($data, inout $sealed, inout $ekeys, vec[$pubkey],
                      '', inout $iv));
  VERIFY(strlen($sealed) > 0);
  VS(count($ekeys), 1);
  VERIFY(strlen($ekeys[0]) > 0);

  $open_data = null;
  VERIFY(openssl_open($sealed, inout $open_data, $ekeys[0], $privkey));
  VS($open_data, $data);

  VERIFY(openssl_open($sealed, inout $open_data, $ekeys[0], $privkey, 'RC4'));
  VS($open_data, $data);

  VERIFY(openssl_seal($data, inout $sealed, inout $ekeys, vec[$pubkey],
                      'AES-256-ECB', inout $iv));
  VERIFY(strlen($sealed) > 0);
  VS(count($ekeys), 1);
  VERIFY(strlen($ekeys[0]) > 0);

  VERIFY(openssl_open($sealed, inout $open_data, $ekeys[0], $privkey,
                      'AES-256-ECB'));
  VS($open_data, $data);
}

function test_openssl_sign() :mixed{
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new(null, inout $privkey);
  VERIFY($csr != null);
  $pubkey = openssl_csr_get_public_key($csr);
  VERIFY($pubkey != null);

  $data = "some secret messages";
  $signature = null;
  VERIFY(openssl_sign($data, inout $signature, $privkey));
  VS(openssl_verify($data, $signature, $pubkey), 1);

}

function test_openssl_x509_check_private_key() :mixed{
  $privkey = openssl_pkey_new();
  VERIFY($privkey != null);
  $csr = openssl_csr_new(null, inout $privkey);
  VERIFY($csr != null);
  $scert = openssl_csr_sign($csr, null, $privkey, 365);
  VERIFY(openssl_x509_check_private_key($scert, $privkey));
}

function test_openssl_x509_checkpurpose() :mixed{
  $fcert = file_get_contents(__DIR__."/test_x509.crt");
  $cert = openssl_x509_read($fcert);
  VS(openssl_x509_checkpurpose($cert, X509_PURPOSE_SSL_CLIENT, vec[]), 0);
  VS(openssl_x509_checkpurpose($cert, X509_PURPOSE_SSL_SERVER, vec[]), 0);
}

function test_openssl_x509_export_to_file() :mixed{
  $fcert = file_get_contents(__DIR__."/test_x509.crt");
  $cert = openssl_x509_read($fcert);

  $tmp = tempnam(sys_get_temp_dir(), 'x509vmopenssltest');
  unlink($tmp);
  VS(file_get_contents($tmp), false);
  VERIFY(openssl_x509_export_to_file($cert, $tmp));

  $fcert2 = file_get_contents($tmp);
  $cert2 = openssl_x509_read($fcert2);
  $info = openssl_x509_parse($cert2);
  VS($info['subject']['O'], "RSA Data Security, Inc.");

  unlink($tmp);
}

function test_openssl_x509_export() :mixed{
  $fcert = file_get_contents(__DIR__."/test_x509.crt");
  $cert = openssl_x509_read($fcert);
  $out = null;
  VERIFY(openssl_x509_export($cert, inout $out));
  $cert2 = openssl_x509_read($out);
  $info = openssl_x509_parse($cert2);
  VS($info['subject']['O'], "RSA Data Security, Inc.");
}

function test_openssl_x509_free() :mixed{
  $fcert = file_get_contents(__DIR__."/test_x509.crt");
  $cert = openssl_x509_read($fcert);
  VERIFY($cert != null);
  openssl_x509_free($cert);
}

function test_openssl_x509_parse() :mixed{
  $fcert = file_get_contents(__DIR__."/test_x509.crt");
  $cert = openssl_x509_read($fcert);
  $info = openssl_x509_parse($cert);
  VS($info['subject']['O'], "RSA Data Security, Inc.");
}

function test_openssl_x509_read() :mixed{
  $fcert = file_get_contents(__DIR__."/test_x509.crt");
  $cert = openssl_x509_read($fcert);
  VERIFY($cert != null);
}

function test_openssl_encrypt() :mixed{
  $test = "OpenSSL is good for encrypting things";
  $secret = "supersecretthing";
  $cipher = "AES-256-CBC";
  $iv_len = openssl_cipher_iv_length($cipher);
  $crypto_strong = false;
  $iv = openssl_random_pseudo_bytes($iv_len, inout $crypto_strong);

  $data = openssl_encrypt($test, $cipher, $secret, 0, $iv);
  VS($test, openssl_decrypt($data, $cipher, $secret, 0, $iv));

  $data = openssl_encrypt($test, $cipher, $secret, OPENSSL_RAW_DATA, $iv);
  VS($test, openssl_decrypt($data, $cipher, $secret, OPENSSL_RAW_DATA, $iv));
}

function test_openssl_digest() :mixed{
  $test = "OpenSSL is also good for hashing things";

  VS(md5($test), openssl_digest($test, "md5"));
}

function test_openssl_encrypt_long() :mixed{
  $pt = 'aa';
  $method = 'aes-128-cbc';
  $iv = str_repeat('x', 16);
  $ct1 = openssl_encrypt($pt, $method, str_repeat('a', 19), 0, $iv);
  $ct2 = openssl_encrypt($pt, $method, str_repeat('a', 16), 0, $iv);
  var_dump($ct1 === $ct2);
}

function test_openssl_decrypt_long() :mixed{
  $pt = 'aa';
  $method = 'aes-128-cbc';
  $iv = str_repeat('x', 16);
  $ct = openssl_encrypt($pt, $method, str_repeat('a', 16), 0, $iv);
  $pt1 = openssl_decrypt($ct, $method, str_repeat('a', 16), 0, $iv);
  $pt2 = openssl_decrypt($ct, $method, str_repeat('a', 19), 0, $iv);
  var_dump($pt1 === $pt2);
  var_dump($pt1 === $pt);
}


//////////////////////////////////////////////////////////////////////

<<__EntryPoint>>
function main_ext_openssl() :mixed{
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
test_openssl_encrypt_long();
test_openssl_decrypt_long();
}
