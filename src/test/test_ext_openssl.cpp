/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <test/test_ext_openssl.h>
#include <runtime/ext/ext_openssl.h>
#include <runtime/ext/ext_file.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtOpenssl::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_openssl_csr_export_to_file);
  RUN_TEST(test_openssl_csr_export);
  RUN_TEST(test_openssl_csr_get_public_key);
  RUN_TEST(test_openssl_csr_get_subject);
  RUN_TEST(test_openssl_csr_new);
  RUN_TEST(test_openssl_csr_sign);
  RUN_TEST(test_openssl_error_string);
  RUN_TEST(test_openssl_free_key);
  RUN_TEST(test_openssl_open);
  RUN_TEST(test_openssl_pkcs12_export_to_file);
  RUN_TEST(test_openssl_pkcs12_export);
  RUN_TEST(test_openssl_pkcs12_read);
  RUN_TEST(test_openssl_pkcs7_decrypt);
  //RUN_TEST(test_openssl_pkcs7_encrypt);
  RUN_TEST(test_openssl_pkcs7_sign);
  RUN_TEST(test_openssl_pkcs7_verify);
  RUN_TEST(test_openssl_pkey_export_to_file);
  RUN_TEST(test_openssl_pkey_export);
  RUN_TEST(test_openssl_pkey_free);
  RUN_TEST(test_openssl_pkey_get_details);
  RUN_TEST(test_openssl_pkey_get_private);
  RUN_TEST(test_openssl_get_privatekey);
  RUN_TEST(test_openssl_pkey_get_public);
  RUN_TEST(test_openssl_get_publickey);
  RUN_TEST(test_openssl_pkey_new);
  RUN_TEST(test_openssl_private_decrypt);
  RUN_TEST(test_openssl_private_encrypt);
  RUN_TEST(test_openssl_public_decrypt);
  RUN_TEST(test_openssl_public_encrypt);
  RUN_TEST(test_openssl_seal);
  RUN_TEST(test_openssl_sign);
  RUN_TEST(test_openssl_verify);
  RUN_TEST(test_openssl_x509_check_private_key);
  RUN_TEST(test_openssl_x509_checkpurpose);
  RUN_TEST(test_openssl_x509_export_to_file);
  RUN_TEST(test_openssl_x509_export);
  RUN_TEST(test_openssl_x509_free);
  RUN_TEST(test_openssl_x509_parse);
  RUN_TEST(test_openssl_x509_read);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtOpenssl::test_openssl_csr_export_to_file() {
  Variant csr = f_openssl_csr_new(null, null);
  VERIFY(!csr.isNull());

  const char *tmp = "test/test_csr.tmp";
  f_unlink(tmp);
  VS(f_file_get_contents(tmp), false);
  f_openssl_csr_export_to_file(csr, tmp);
  VERIFY(f_file_get_contents(tmp).toString().size() > 400);
  f_unlink(tmp);

  return Count(true);
}

bool TestExtOpenssl::test_openssl_csr_export() {
  // tested in test_openssl_csr_sign()
  return Count(true);
}

bool TestExtOpenssl::test_openssl_csr_get_public_key() {
  Variant csr = f_openssl_csr_new(null, null);
  VERIFY(!csr.isNull());
  Variant publickey = f_openssl_csr_get_public_key(csr);
  VERIFY(!same(publickey, false));
  VERIFY(!publickey.isNull());
  return Count(true);
}

bool TestExtOpenssl::test_openssl_csr_get_subject() {
  Variant csr = f_openssl_csr_new(null, null);
  VERIFY(!csr.isNull());
  VS(f_openssl_csr_get_subject(csr)["O"], "My Company Ltd");
  return Count(true);
}

bool TestExtOpenssl::test_openssl_csr_new() {
  // tested in test_openssl_csr_sign()
  return Count(true);
}

bool TestExtOpenssl::test_openssl_csr_sign() {
  Array dn(ArrayInit(7, false).
           set(0, "countryName", "XX").
           set(1, "stateOrProvinceName", "State").
           set(2, "localityName", "SomewhereCity").
           set(3, "organizationName", "MySelf").
           set(4, "organizationalUnitName", "Whatever").
           set(5, "commonName", "mySelf").
           set(6, "emailAddress", "user@domain.com").
           create());
  String privkeypass = "1234";
  int numberofdays = 365;

  Variant privkey = f_openssl_pkey_new();
  VERIFY(!privkey.isNull());
  Variant csr = f_openssl_csr_new(dn, privkey);
  VERIFY(!csr.isNull());
  Variant scert = f_openssl_csr_sign(csr, null, privkey, numberofdays);
  Variant publickey, privatekey, csrStr;
  f_openssl_x509_export(scert, ref(publickey));
  f_openssl_pkey_export(privkey, ref(privatekey), privkeypass);
  f_openssl_csr_export(csr, ref(csrStr));

  //f_var_dump(privatekey); f_var_dump(publickey); f_var_dump(csrStr);
  VERIFY(privatekey.toString().size() > 500);
  VERIFY(publickey.toString().size() > 800);
  VERIFY(csrStr.toString().size() > 500);

  return Count(true);
}

bool TestExtOpenssl::test_openssl_error_string() {
  Variant ret = f_openssl_error_string();
  return Count(true);
}

bool TestExtOpenssl::test_openssl_free_key() {
  Variant csr = f_openssl_csr_new(null, null);
  VERIFY(!csr.isNull());
  Variant publickey = f_openssl_csr_get_public_key(csr);
  VERIFY(!same(publickey, false));
  VERIFY(!publickey.isNull());
  f_openssl_free_key(publickey);
  return Count(true);
}

bool TestExtOpenssl::test_openssl_open() {
  // tested in test_openssl_seal()
  return Count(true);
}

bool TestExtOpenssl::test_openssl_pkcs12_export_to_file() {
  Variant privkey = f_openssl_pkey_new();
  VERIFY(!privkey.isNull());
  Variant csr = f_openssl_csr_new(null, privkey);
  VERIFY(!csr.isNull());
  Variant scert = f_openssl_csr_sign(csr, null, privkey, 365);

  const char *tmp = "test/test_pkcs12.tmp";
  f_unlink(tmp);
  VS(f_file_get_contents(tmp), false);
  f_openssl_pkcs12_export_to_file(scert, tmp, privkey, "1234");
  VERIFY(f_file_get_contents(tmp).toString().size() > 400);
  f_unlink(tmp);
  return Count(true);
}

bool TestExtOpenssl::test_openssl_pkcs12_export() {
  // tested in test_openssl_pkcs12_read()
  return Count(true);
}

bool TestExtOpenssl::test_openssl_pkcs12_read() {
  Variant privkey = f_openssl_pkey_new();
  VERIFY(!privkey.isNull());
  Variant csr = f_openssl_csr_new(null, privkey);
  VERIFY(!csr.isNull());
  Variant scert = f_openssl_csr_sign(csr, null, privkey, 365);

  Variant pkcs12;
  f_openssl_pkcs12_export(scert, ref(pkcs12), privkey, "1234");

  Variant certs;
  VERIFY(f_openssl_pkcs12_read(pkcs12, ref(certs), "1234"));
  VERIFY(certs["cert"].toString().size() > 500);
  VERIFY(certs["pkey"].toString().size() > 500);

  return Count(true);
}

bool TestExtOpenssl::test_openssl_pkcs7_decrypt() {
  // tested in test_openssl_pkcs7_encrypt()
  return Count(true);
}

bool TestExtOpenssl::test_openssl_pkcs7_encrypt() {
  Variant privkey = f_openssl_pkey_new();
  VERIFY(!privkey.isNull());
  Variant csr = f_openssl_csr_new(null, privkey);
  VERIFY(!csr.isNull());
  Variant scert = f_openssl_csr_sign(csr, null, privkey, 365);
  Variant pubkey = f_openssl_csr_get_public_key(csr);
  VERIFY(!pubkey.isNull());

  String data = "some secret data";
  const char *infile = "test/test_pkcs7.in";
  const char *outfile = "test/test_pkcs7.out";
  f_unlink(infile);
  f_unlink(outfile);
  f_file_put_contents(infile, data);

  VERIFY(f_openssl_pkcs7_encrypt
         (infile, outfile, scert,
          CREATE_MAP2("To", "t@facebook.com","From", "hzhao@facebook.com")));

  f_unlink(infile);
  VERIFY(f_openssl_pkcs7_decrypt(outfile, infile, scert, privkey));
  Variant decrypted = f_file_get_contents(infile);
  f_var_dump(decrypted);
  f_unlink(infile);
  f_unlink(outfile);
  /*
   * PHP didn't work either:

    $privkey = openssl_pkey_new();
    $csr = openssl_csr_new(array(), $privkey);
    $scert = openssl_csr_sign($csr, null, $privkey, 365);

    $data = "some secret data";
    $infile = "test_pkcs7.in";
    $outfile = "test_pkcs7.out";
    file_put_contents($infile, $data);

    openssl_pkcs7_encrypt($infile, $outfile, $scert,
                          array("To" => "t@facebook.com",
                                "From" => "hzhao@facebook.com"));

    var_dump(openssl_pkcs7_decrypt($outfile, $infile, $scert, $privkey));
    $decrypted = file_get_contents($infile);var_dump($decrypted);

   */
  return Count(true);
}

bool TestExtOpenssl::test_openssl_pkcs7_sign() {
  Variant privkey = f_openssl_pkey_new();
  VERIFY(!privkey.isNull());
  Variant csr = f_openssl_csr_new(null, privkey);
  VERIFY(!csr.isNull());
  Variant scert = f_openssl_csr_sign(csr, null, privkey, 365);
  Variant pubkey = f_openssl_csr_get_public_key(csr);
  VERIFY(!pubkey.isNull());

  String data = "some secret data";
  const char *infile = "test/test_pkcs7.in";
  const char *outfile = "test/test_pkcs7.out";
  f_unlink(infile);
  f_unlink(outfile);
  f_file_put_contents(infile, data);

  VERIFY(f_openssl_pkcs7_sign
         (infile, outfile, scert, privkey,
          CREATE_MAP2("To", "t@facebook.com","From", "hzhao@facebook.com")));

  const char *tmp = "test/test_x509.tmp";
  f_unlink(tmp);
  VS(f_file_get_contents(tmp), false);
  VERIFY(f_openssl_x509_export_to_file(scert, tmp));

  VS(f_openssl_pkcs7_verify(outfile, 0, infile, Variant(tmp).toArray()), true);
  f_unlink(infile);
  f_unlink(outfile);
  f_unlink(tmp);
  return Count(true);
}

bool TestExtOpenssl::test_openssl_pkcs7_verify() {
  // tested in test_openssl_pkcs7_sign()
  return Count(true);
}

bool TestExtOpenssl::test_openssl_pkey_export_to_file() {
  const char *tmp = "test/test_pkey.tmp";
  f_unlink(tmp);
  VS(f_file_get_contents(tmp), false);

  Variant privkey = f_openssl_pkey_new();
  VERIFY(!privkey.isNull());
  f_openssl_pkey_export_to_file(privkey, tmp, "1234");

  VERIFY(f_file_get_contents(tmp).toString().size() > 400);
  f_unlink(tmp);
  return Count(true);
}

bool TestExtOpenssl::test_openssl_pkey_export() {
  Variant privkey = f_openssl_pkey_new();
  VERIFY(!privkey.isNull());
  Variant out;
  f_openssl_pkey_export(privkey, ref(out), "1234");
  VERIFY(out.toString().size() > 500);
  return Count(true);
}

bool TestExtOpenssl::test_openssl_pkey_free() {
  Variant fkey = f_file_get_contents("test/test_public.pem");
  Variant k = f_openssl_pkey_get_public(fkey);
  VERIFY(!same(k, false));
  VERIFY(!k.isNull());
  f_openssl_pkey_free(k);
  return Count(true);
}

bool TestExtOpenssl::test_openssl_pkey_get_details() {
  {
    Variant fkey = f_file_get_contents("test/test_public.pem");
    Variant k = f_openssl_pkey_get_public(fkey);
    VERIFY(!same(k, false));
    VERIFY(!k.isNull());
    VS(f_openssl_pkey_get_details(k)["bits"], 1024);
  }
  {
    Variant fkey = f_file_get_contents("test/test_private.pem");
    Variant k = f_openssl_pkey_get_private(fkey);
    VERIFY(!same(k, false));
    VERIFY(!k.isNull());
    VS(f_openssl_pkey_get_details(k)["bits"], 512);
  }
  return Count(true);
}

bool TestExtOpenssl::test_openssl_pkey_get_private() {
  // tested in test_openssl_pkey_get_details()
  return Count(true);
}

bool TestExtOpenssl::test_openssl_get_privatekey() {
  // same as openssl_pkey_get_private()
  return Count(true);
}

bool TestExtOpenssl::test_openssl_pkey_get_public() {
  // tested in test_openssl_pkey_get_details()
  return Count(true);
}

bool TestExtOpenssl::test_openssl_get_publickey() {
  // same as openssl_pkey_get_public()
  return Count(true);
}

bool TestExtOpenssl::test_openssl_pkey_new() {
  // tested in test_openssl_csr_sign() and test_openssl_seal()
  return Count(true);
}

bool TestExtOpenssl::test_openssl_private_decrypt() {
  // tested in test_openssl_public_encrypt()
  return Count(true);
}

bool TestExtOpenssl::test_openssl_private_encrypt() {
  Variant privkey = f_openssl_pkey_new();
  VERIFY(!privkey.isNull());
  Variant csr = f_openssl_csr_new(null, privkey);
  VERIFY(!csr.isNull());
  Variant pubkey = f_openssl_csr_get_public_key(csr);
  VERIFY(!pubkey.isNull());

  String data = "some secret data";
  Variant out;
  VERIFY(f_openssl_private_encrypt(data, ref(out), privkey));
  Variant out2;
  VERIFY(f_openssl_public_decrypt(out, ref(out2), pubkey));
  VS(out2, data);
  return Count(true);
}

bool TestExtOpenssl::test_openssl_public_decrypt() {
  // tested in test_openssl_private_encrypt()
  return Count(true);
}

bool TestExtOpenssl::test_openssl_public_encrypt() {
  Variant privkey = f_openssl_pkey_new();
  VERIFY(!privkey.isNull());
  Variant csr = f_openssl_csr_new(null, privkey);
  VERIFY(!csr.isNull());
  Variant pubkey = f_openssl_csr_get_public_key(csr);
  VERIFY(!pubkey.isNull());

  String data = "some secret data";
  Variant out;
  VERIFY(f_openssl_public_encrypt(data, ref(out), pubkey));
  Variant out2;
  VERIFY(f_openssl_private_decrypt(out, ref(out2), privkey));
  VS(out2, data);
  return Count(true);
}

bool TestExtOpenssl::test_openssl_seal() {
  Variant privkey = f_openssl_pkey_new();
  VERIFY(!privkey.isNull());
  Variant csr = f_openssl_csr_new(null, privkey);
  VERIFY(!csr.isNull());
  Variant pubkey = f_openssl_csr_get_public_key(csr);
  VERIFY(!pubkey.isNull());

  String data = "some secret messages";
  Variant sealed;
  Variant ekeys;
  VERIFY(f_openssl_seal(data, ref(sealed), ref(ekeys),
                        CREATE_VECTOR1(pubkey)));
  VERIFY(!sealed.toString().empty());
  VS(ekeys.toArray().size(), 1);
  VERIFY(!ekeys[0].toString().empty());

  Variant open_data;
  VERIFY(f_openssl_open(sealed, ref(open_data), ekeys[0], privkey));
  VS(open_data, data);

  return Count(true);
}

bool TestExtOpenssl::test_openssl_sign() {
  Variant privkey = f_openssl_pkey_new();
  VERIFY(!privkey.isNull());
  Variant csr = f_openssl_csr_new(null, privkey);
  VERIFY(!csr.isNull());
  Variant pubkey = f_openssl_csr_get_public_key(csr);
  VERIFY(!pubkey.isNull());

  String data = "some secret messages";
  Variant signature;
  VERIFY(f_openssl_sign(data, ref(signature), privkey));
  VS(f_openssl_verify(data, signature, pubkey), 1);

  return Count(true);
}

bool TestExtOpenssl::test_openssl_verify() {
  // tested in test_openssl_sign()
  return Count(true);
}

bool TestExtOpenssl::test_openssl_x509_check_private_key() {
  Variant privkey = f_openssl_pkey_new();
  VERIFY(!privkey.isNull());
  Variant csr = f_openssl_csr_new(null, privkey);
  VERIFY(!csr.isNull());
  Variant scert = f_openssl_csr_sign(csr, null, privkey, 365);
  VERIFY(f_openssl_x509_check_private_key(scert, privkey));
  return Count(true);
}

bool TestExtOpenssl::test_openssl_x509_checkpurpose() {
  Variant fcert = f_file_get_contents("test/test_x509.crt");
  Variant cert = f_openssl_x509_read(fcert);
  VS(f_openssl_x509_checkpurpose(cert, k_X509_PURPOSE_SSL_CLIENT), 0);
  VS(f_openssl_x509_checkpurpose(cert, k_X509_PURPOSE_SSL_SERVER), 0);
  return Count(true);
}

bool TestExtOpenssl::test_openssl_x509_export_to_file() {
  Variant fcert = f_file_get_contents("test/test_x509.crt");
  Variant cert = f_openssl_x509_read(fcert);

  const char *tmp = "test/test_x509.tmp";
  f_unlink(tmp);
  VS(f_file_get_contents(tmp), false);
  VERIFY(f_openssl_x509_export_to_file(cert, tmp));

  Variant fcert2 = f_file_get_contents(tmp);
  Variant cert2 = f_openssl_x509_read(fcert2);
  Variant info = f_openssl_x509_parse(cert2);
  VS(info["subject"]["O"], "RSA Data Security, Inc.");

  f_unlink(tmp);
  return Count(true);
}

bool TestExtOpenssl::test_openssl_x509_export() {
  Variant fcert = f_file_get_contents("test/test_x509.crt");
  Variant cert = f_openssl_x509_read(fcert);
  Variant out;
  VERIFY(f_openssl_x509_export(cert, ref(out)));
  Variant cert2 = f_openssl_x509_read(out);
  Variant info = f_openssl_x509_parse(cert2);
  VS(info["subject"]["O"], "RSA Data Security, Inc.");
  return Count(true);
}

bool TestExtOpenssl::test_openssl_x509_free() {
  Variant fcert = f_file_get_contents("test/test_x509.crt");
  Variant cert = f_openssl_x509_read(fcert);
  VERIFY(!cert.toObject().isNull());
  f_openssl_x509_free(cert);
  return Count(true);
}

bool TestExtOpenssl::test_openssl_x509_parse() {
  Variant fcert = f_file_get_contents("test/test_x509.crt");
  Variant cert = f_openssl_x509_read(fcert);
  Variant info = f_openssl_x509_parse(cert);
  VS(info["subject"]["O"], "RSA Data Security, Inc.");
  return Count(true);
}

bool TestExtOpenssl::test_openssl_x509_read() {
  Variant fcert = f_file_get_contents("test/test_x509.crt");
  Variant cert = f_openssl_x509_read(fcert);
  VERIFY(!cert.toObject().isNull());
  return Count(true);
}
