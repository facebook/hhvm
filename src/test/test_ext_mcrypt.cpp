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

#include <test/test_ext_mcrypt.h>
#include <runtime/ext/ext_mcrypt.h>
#include <runtime/ext/ext_string.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtMcrypt::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_mcrypt_module_open);
  RUN_TEST(test_mcrypt_module_close);
  RUN_TEST(test_mcrypt_list_algorithms);
  RUN_TEST(test_mcrypt_list_modes);
  RUN_TEST(test_mcrypt_module_get_algo_block_size);
  RUN_TEST(test_mcrypt_module_get_algo_key_size);
  RUN_TEST(test_mcrypt_module_get_supported_key_sizes);
  RUN_TEST(test_mcrypt_module_is_block_algorithm_mode);
  RUN_TEST(test_mcrypt_module_is_block_algorithm);
  RUN_TEST(test_mcrypt_module_is_block_mode);
  RUN_TEST(test_mcrypt_module_self_test);
  RUN_TEST(test_mcrypt_create_iv);
  RUN_TEST(test_mcrypt_encrypt);
  RUN_TEST(test_mcrypt_decrypt);
  RUN_TEST(test_mcrypt_cbc);
  RUN_TEST(test_mcrypt_cfb);
  RUN_TEST(test_mcrypt_ecb);
  RUN_TEST(test_mcrypt_ofb);
  RUN_TEST(test_mcrypt_get_block_size);
  RUN_TEST(test_mcrypt_get_cipher_name);
  RUN_TEST(test_mcrypt_get_iv_size);
  RUN_TEST(test_mcrypt_get_key_size);
  RUN_TEST(test_mcrypt_enc_get_algorithms_name);
  RUN_TEST(test_mcrypt_enc_get_block_size);
  RUN_TEST(test_mcrypt_enc_get_iv_size);
  RUN_TEST(test_mcrypt_enc_get_key_size);
  RUN_TEST(test_mcrypt_enc_get_modes_name);
  RUN_TEST(test_mcrypt_enc_get_supported_key_sizes);
  RUN_TEST(test_mcrypt_enc_is_block_algorithm_mode);
  RUN_TEST(test_mcrypt_enc_is_block_algorithm);
  RUN_TEST(test_mcrypt_enc_is_block_mode);
  RUN_TEST(test_mcrypt_enc_self_test);
  RUN_TEST(test_mcrypt_generic);
  RUN_TEST(test_mcrypt_generic_init);
  RUN_TEST(test_mdecrypt_generic);
  RUN_TEST(test_mcrypt_generic_deinit);
  RUN_TEST(test_mcrypt_generic_end);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtMcrypt::test_mcrypt_module_open() {
  Variant td = f_mcrypt_module_open("rijndael-256", "", "ofb", "");
  Variant iv = f_mcrypt_create_iv(f_mcrypt_enc_get_iv_size(td),
                                  k_MCRYPT_DEV_RANDOM);
  Variant ks = f_mcrypt_enc_get_key_size(td);
  Variant key = f_substr(f_md5("very secret key"), 0, ks);
  f_mcrypt_generic_init(td, key, iv);
  Variant encrypted = f_mcrypt_generic(td, "This is very important data");
  VERIFY(!same(encrypted, "This is very important data"));
  f_mcrypt_generic_deinit(td);
  f_mcrypt_generic_init(td, key, iv);
  Variant decrypted = f_mdecrypt_generic(td, encrypted);
  f_mcrypt_generic_end(td);
  f_mcrypt_module_close(td);

  VS(decrypted, "This is very important data");
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_module_close() {
  // tested in test_mcrypt_module_open()
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_list_algorithms() {
  VERIFY(array_value_exists(f_mcrypt_list_algorithms(), "blowfish"));
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_list_modes() {
  VERIFY(array_value_exists(f_mcrypt_list_modes(), "cbc"));
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_module_get_algo_block_size() {
  VS(f_mcrypt_module_get_algo_block_size("blowfish"), 8);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_module_get_algo_key_size() {
  VS(f_mcrypt_module_get_algo_key_size("blowfish"), 56);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_module_get_supported_key_sizes() {
  VS(f_mcrypt_module_get_supported_key_sizes("blowfish"), Array::Create());
  VS(f_mcrypt_module_get_supported_key_sizes("twofish"),
     CREATE_VECTOR3(16, 24, 32));
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_module_is_block_algorithm_mode() {
  VS(f_mcrypt_module_is_block_algorithm_mode("cbc"), true);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_module_is_block_algorithm() {
  VS(f_mcrypt_module_is_block_algorithm("blowfish"), true);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_module_is_block_mode() {
  VS(f_mcrypt_module_is_block_mode("cbc"), true);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_module_self_test() {
  VS(f_mcrypt_module_self_test(k_MCRYPT_RIJNDAEL_128), true);
  VS(f_mcrypt_module_self_test("bogus"), false);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_create_iv() {
  // tested in test_mcrypt_module_open()
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_encrypt() {
  String text = "boggles the inivisble monkey will rule the world";
  String key = "very secret key";
  Variant iv_size = f_mcrypt_get_iv_size(k_MCRYPT_XTEA, k_MCRYPT_MODE_ECB);
  Variant iv = f_mcrypt_create_iv(iv_size, k_MCRYPT_RAND);
  Variant enc = f_mcrypt_encrypt(k_MCRYPT_XTEA, key, text, k_MCRYPT_MODE_ECB,
                                 iv);
  VS(f_bin2hex(enc), "f522c62002fa16129c8576bcddc6dd0f7ea81991103ba42962d94c8bfff3ee660d53b187d7e989540abf5a729c2f7baf");
  Variant crypttext = f_mcrypt_decrypt(k_MCRYPT_XTEA, key, enc,
                                       k_MCRYPT_MODE_ECB, iv);
  VS(crypttext, text);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_decrypt() {
  // tested in test_mcrypt_encrypt()
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_cbc() {
  String key = "123456789012345678901234567890123456789012345678901234567890";
  String CC = "4007000000027";
  Variant encrypted =
    f_mcrypt_cbc(k_MCRYPT_RIJNDAEL_128, f_substr(key,0,32),
                 CC, k_MCRYPT_ENCRYPT, f_substr(key,32,16));
  Variant decrypted =
    f_mcrypt_cbc(k_MCRYPT_RIJNDAEL_128, f_substr(key,0,32),
                 encrypted, k_MCRYPT_DECRYPT, f_substr(key,32,16));
  VERIFY(!same(encrypted, decrypted));
  VS(decrypted.toString().data(), CC);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_cfb() {
  String key = "123456789012345678901234567890123456789012345678901234567890";
  String CC = "4007000000027";
  Variant encrypted =
    f_mcrypt_cfb(k_MCRYPT_RIJNDAEL_128, f_substr(key,0,32),
                 CC, k_MCRYPT_ENCRYPT, f_substr(key,32,16));
  Variant decrypted =
    f_mcrypt_cfb(k_MCRYPT_RIJNDAEL_128, f_substr(key,0,32),
                 encrypted, k_MCRYPT_DECRYPT, f_substr(key,32,16));
  VERIFY(!same(encrypted, decrypted));
  VS(decrypted, CC);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_ecb() {
  String key = "123456789012345678901234567890123456789012345678901234567890";
  String CC = "4007000000027";
  Variant encrypted =
    f_mcrypt_ecb(k_MCRYPT_RIJNDAEL_128, f_substr(key,0,32),
                 CC, k_MCRYPT_ENCRYPT, f_substr(key,32,16));
  Variant decrypted =
    f_mcrypt_ecb(k_MCRYPT_RIJNDAEL_128, f_substr(key,0,32),
                 encrypted, k_MCRYPT_DECRYPT, f_substr(key,32,16));
  VERIFY(!same(encrypted, decrypted));
  VS(decrypted.toString().data(), CC);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_ofb() {
  String key = "123456789012345678901234567890123456789012345678901234567890";
  String CC = "4007000000027";
  Variant encrypted =
    f_mcrypt_ofb(k_MCRYPT_RIJNDAEL_128, f_substr(key,0,32),
                 CC, k_MCRYPT_ENCRYPT, f_substr(key,32,16));
  Variant decrypted =
    f_mcrypt_ofb(k_MCRYPT_RIJNDAEL_128, f_substr(key,0,32),
                 encrypted, k_MCRYPT_DECRYPT, f_substr(key,32,16));
  VERIFY(!same(encrypted, decrypted));
  VS(decrypted, CC);
  return Count(true);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_get_block_size() {
  VS(f_mcrypt_get_block_size("tripledes", "ecb"), 8);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_get_cipher_name() {
  VS(f_mcrypt_get_cipher_name(k_MCRYPT_TRIPLEDES), "3DES");
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_get_iv_size() {
  VS(f_mcrypt_get_iv_size(k_MCRYPT_CAST_256, k_MCRYPT_MODE_CFB), 16);
  VS(f_mcrypt_get_iv_size("des", "ecb"), 8);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_get_key_size() {
  VS(f_mcrypt_get_key_size("tripledes", "ecb"), 24);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_enc_get_algorithms_name() {
  Variant td = f_mcrypt_module_open("cast-256", "", "cfb", "");
  VS(f_mcrypt_enc_get_algorithms_name(td), "CAST-256");
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_enc_get_block_size() {
  Variant td = f_mcrypt_module_open("tripledes", "", "ecb", "");
  VS(f_mcrypt_enc_get_block_size(td), 8);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_enc_get_iv_size() {
  Variant td = f_mcrypt_module_open("cast-256", "", "cfb", "");
  VS(f_mcrypt_enc_get_iv_size(td), 16);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_enc_get_key_size() {
  Variant td = f_mcrypt_module_open("tripledes", "", "ecb", "");
  VS(f_mcrypt_enc_get_key_size(td), 24);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_enc_get_modes_name() {
  Variant td = f_mcrypt_module_open("cast-256", "", "cfb", "");
  VS(f_mcrypt_enc_get_modes_name(td), "CFB");
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_enc_get_supported_key_sizes() {
  Variant td = f_mcrypt_module_open("rijndael-256", "", "ecb", "");
  VS(f_mcrypt_enc_get_supported_key_sizes(td),
     CREATE_VECTOR3(16, 24, 32));
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_enc_is_block_algorithm_mode() {
  Variant td = f_mcrypt_module_open("tripledes", "", "ecb", "");
  VS(f_mcrypt_enc_is_block_algorithm_mode(td), true);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_enc_is_block_algorithm() {
  Variant td = f_mcrypt_module_open("tripledes", "", "ecb", "");
  VS(f_mcrypt_enc_is_block_algorithm(td), true);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_enc_is_block_mode() {
  Variant td = f_mcrypt_module_open("tripledes", "", "ecb", "");
  VS(f_mcrypt_enc_is_block_mode(td), true);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_enc_self_test() {
  Variant td = f_mcrypt_module_open("tripledes", "", "ecb", "");
  VS(f_mcrypt_enc_self_test(td), 0);
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_generic() {
  // tested in test_mcrypt_module_open()
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_generic_init() {
  // tested in test_mcrypt_module_open()
  return Count(true);
}

bool TestExtMcrypt::test_mdecrypt_generic() {
  // tested in test_mcrypt_module_open()
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_generic_deinit() {
  // tested in test_mcrypt_module_open()
  return Count(true);
}

bool TestExtMcrypt::test_mcrypt_generic_end() {
  // tested in test_mcrypt_module_open()
  return Count(true);
}
