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

#include <test/test_ext_hash.h>
#include <runtime/ext/ext_hash.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtHash::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_hash);
  RUN_TEST(test_hash_algos);
  RUN_TEST(test_hash_init);
  RUN_TEST(test_hash_file);
  RUN_TEST(test_hash_final);
  RUN_TEST(test_hash_hmac_file);
  RUN_TEST(test_hash_hmac);
  RUN_TEST(test_hash_update_file);
  RUN_TEST(test_hash_update_stream);
  RUN_TEST(test_hash_update);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtHash::test_hash() {
  static const char *expected[] = {
    "1072638ba8fc6f2ff06c251b62f426fd",
    "a1e0224d596927a56c8bae416b2ef23e",
    "5c6ffbdd40d9556b73a21e63c3e0e904",
    "c0854fb9fb03c41cce3802cb0d220529e6eef94e",
    "68b1282b91de2c054c36629cb8dd447f12f096d3e3c587978dc2248444633483",
    "b7273c05ad141ccb6696b3659e57137c453b6d64690fa7d5cf96368df4a7138703a8c6ead31727b487b3628746510391",
    "0a8c150176c2ba391d7f1670ef4955cd99d3c3ec8cf06198cec30d436f2ac0c9b64229b5a54bdbd5563160503ce992a74be528761da9d0c48b7c74627302eb25",
    "077a3fd0a18a01cb23c3e1bede846f99",
    "ec457d0a974c48d5685a7efa03d137dc8bbde7e3",
    "50c51844b845b31323765ae334349dd6a94db3e5b9624540bcbfa940f6857c3f",
    "67b248ca0b750028e76f09f5f9b9b746f04c228ce659f75393c83ee46b82dea011f15f465a7e4f71",
    "802dc377bf6dc4f905b90cf2f1ddb39d4958526c3772bce41c03488701630eeede851f5ddc195714ea9e35311a513e31c3b616ffce5756bd963e0fdc092b2f87",
    "9370512795923aaeeb76fe3d8ea7433e",
    "9370512795923aaeeb76fe3d8ea7433e0697d65d",
    "9370512795923aaeeb76fe3d8ea7433e0697d65d1d6b3975",
    "623163ed5f7b8e35ca1c2c1beb083289",
    "623163ed5f7b8e35ca1c2c1beb0832891ab00cc6",
    "623163ed5f7b8e35ca1c2c1beb0832891ab00cc615ef6e6b",
    "1d4ca34cc860789a2a63fab87cd6a2c3ae5ecb1df8bd3cce605ffa2de1fbd73b",
    "c10eb0cb71a04377a0452a4aa64853996f73cac95f6ae434df8083d473fac944",
    "5e10f17b",
    "413a86af",
    "4246a382",
    "aa517f0b69b75146a39384ec92a02877",
    "d9f46869ef2bf66602a0725c079d35a9f3ac6dd0",
    "04934beab28037aae8fd658389626368530562b96c9aba89",
    "41c959f1e44e931b0473c54c49080d74d49a96ea56e766af408a85fd",
    "6c7c17d5784428d4d62b7f652223d64a30c78aa5f2c2c71ce780ec3f0d0ec28d",
    "31147399766a068d0417390a4b9fa0c8",
    "ad9aa7904f202cdfb1faf7d385d10e3de575f74c",
    "1568ff7e99ca98fbeb9a2d4c0318dcc290d0eb10d064d0f6",
    "94656ade22076dd122714b4168a2ed8228b554f70eca5728c5038579",
    "404e1584994409ee38e0829099521168ba7c3aa1b0e82d1a72ec387fd1317ecf",
    "bc0db0f23eabcd9d0c4d0a2e498b8c47",
    "51341aa150d38695628491580d8d6dc9850201f7",
    "da27cf4bdb2decd4731c69a017534535eecd6d9b9015fc41",
    "f8a5d442ef5b82e062599fe38ddbf46999b29f0a15caa8bebabbff68",
    "16e1688c75cf09338fce299455ec0f6f783ca1cbb2006203ae6ae98b23f9294a"
  };

  String data = "The quick brown fox jumped over the lazy dog.";
  int i = 0;
  VS(f_hash("md2",        data), expected[i++]);
  VS(f_hash("md4",        data), expected[i++]);
  VS(f_hash("md5",        data), expected[i++]);
  VS(f_hash("sha1",       data), expected[i++]);
  VS(f_hash("sha256",     data), expected[i++]);
  VS(f_hash("sha384",     data), expected[i++]);
  VS(f_hash("sha512",     data), expected[i++]);
  VS(f_hash("ripemd128",  data), expected[i++]);
  VS(f_hash("ripemd160",  data), expected[i++]);
  VS(f_hash("ripemd256",  data), expected[i++]);
  VS(f_hash("ripemd320",  data), expected[i++]);
  VS(f_hash("whirlpool",  data), expected[i++]);
  VS(f_hash("tiger128,3", data), expected[i++]);
  VS(f_hash("tiger160,3", data), expected[i++]);
  VS(f_hash("tiger192,3", data), expected[i++]);
  VS(f_hash("tiger128,4", data), expected[i++]);
  VS(f_hash("tiger160,4", data), expected[i++]);
  VS(f_hash("tiger192,4", data), expected[i++]);
  VS(f_hash("snefru",     data), expected[i++]);
  VS(f_hash("gost",       data), expected[i++]);
  VS(f_hash("adler32",    data), expected[i++]);
  VS(f_hash("crc32",      data), expected[i++]);
  VS(f_hash("crc32b",     data), expected[i++]);
  VS(f_hash("haval128,3", data), expected[i++]);
  VS(f_hash("haval160,3", data), expected[i++]);
  VS(f_hash("haval192,3", data), expected[i++]);
  VS(f_hash("haval224,3", data), expected[i++]);
  VS(f_hash("haval256,3", data), expected[i++]);
  VS(f_hash("haval128,4", data), expected[i++]);
  VS(f_hash("haval160,4", data), expected[i++]);
  VS(f_hash("haval192,4", data), expected[i++]);
  VS(f_hash("haval224,4", data), expected[i++]);
  VS(f_hash("haval256,4", data), expected[i++]);
  VS(f_hash("haval128,5", data), expected[i++]);
  VS(f_hash("haval160,5", data), expected[i++]);
  VS(f_hash("haval192,5", data), expected[i++]);
  VS(f_hash("haval224,5", data), expected[i++]);
  VS(f_hash("haval256,5", data), expected[i++]);

  return Count(true);
}

bool TestExtHash::test_hash_algos() {
  VERIFY(!f_hash_algos().empty());
  return Count(true);
}

bool TestExtHash::test_hash_init() {
  Object ctx = f_hash_init("md5");
  f_hash_update(ctx, "The quick brown fox ");
  f_hash_update(ctx, "jumped over the lazy dog.");
  VS(f_hash_final(ctx), "5c6ffbdd40d9556b73a21e63c3e0e904");
  return Count(true);
}

bool TestExtHash::test_hash_file() {
  VS(f_hash_file("md5", "test/test_hash_file.txt"),
     "5c6ffbdd40d9556b73a21e63c3e0e904");
  return Count(true);
}

bool TestExtHash::test_hash_final() {
  return test_hash_init();
}

bool TestExtHash::test_hash_hmac_file() {
  VS(f_hash_hmac_file("md5", "test/test_hash_file.txt", "secret"),
     "7eb2b5c37443418fc77c136dd20e859c");
  return Count(true);
}

bool TestExtHash::test_hash_hmac() {
  VS(f_hash_hmac("ripemd160", "The quick brown fox jumped over the lazy dog.",
                 "secret"),
     "b8e7ae12510bdfb1812e463a7f086122cf37e4f7");
  return Count(true);
}

bool TestExtHash::test_hash_update_file() {
  // this is the same as hash_update, except reading from a file
  return Count(true);
}

bool TestExtHash::test_hash_update_stream() {
  // this is the same as hash_update, except reading from a stream
  return Count(true);
}

bool TestExtHash::test_hash_update() {
  return test_hash_init();
}
