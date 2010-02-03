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

#include <test/test_ext_mailparse.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtMailparse::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_mail);
  RUN_TEST(test_ezmlm_hash);
  RUN_TEST(test_mailparse_msg_create);
  RUN_TEST(test_mailparse_msg_free);
  RUN_TEST(test_mailparse_msg_parse_file);
  RUN_TEST(test_mailparse_msg_parse);
  RUN_TEST(test_mailparse_msg_extract_part_file);
  RUN_TEST(test_mailparse_msg_extract_whole_part_file);
  RUN_TEST(test_mailparse_msg_extract_part);
  RUN_TEST(test_mailparse_msg_get_part_data);
  RUN_TEST(test_mailparse_msg_get_part);
  RUN_TEST(test_mailparse_msg_get_structure);
  RUN_TEST(test_mailparse_rfc822_parse_addresses);
  RUN_TEST(test_mailparse_stream_encode);
  RUN_TEST(test_mailparse_uudecode_all);
  RUN_TEST(test_mailparse_determine_best_xfer_encoding);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtMailparse::test_mail() {
  //VCB("<?php ");
  return true;
}

bool TestExtMailparse::test_ezmlm_hash() {
  //VCB("<?php ");
  return true;
}

bool TestExtMailparse::test_mailparse_msg_create() {
  //VCB("<?php ");
  return true;
}

bool TestExtMailparse::test_mailparse_msg_free() {
  //VCB("<?php ");
  return true;
}

bool TestExtMailparse::test_mailparse_msg_parse_file() {
  //VCB("<?php ");
  return true;
}

bool TestExtMailparse::test_mailparse_msg_parse() {
  //VCB("<?php ");
  return true;
}

bool TestExtMailparse::test_mailparse_msg_extract_part_file() {
  //VCB("<?php ");
  return true;
}

bool TestExtMailparse::test_mailparse_msg_extract_whole_part_file() {
  //VCB("<?php ");
  return true;
}

bool TestExtMailparse::test_mailparse_msg_extract_part() {
  //VCB("<?php ");
  return true;
}

bool TestExtMailparse::test_mailparse_msg_get_part_data() {
  //VCB("<?php ");
  return true;
}

bool TestExtMailparse::test_mailparse_msg_get_part() {
  //VCB("<?php ");
  return true;
}

bool TestExtMailparse::test_mailparse_msg_get_structure() {
  //VCB("<?php ");
  return true;
}

bool TestExtMailparse::test_mailparse_rfc822_parse_addresses() {
  //VCB("<?php ");
  return true;
}

bool TestExtMailparse::test_mailparse_stream_encode() {
  //VCB("<?php ");
  return true;
}

bool TestExtMailparse::test_mailparse_uudecode_all() {
  //VCB("<?php ");
  return true;
}

bool TestExtMailparse::test_mailparse_determine_best_xfer_encoding() {
  //VCB("<?php ");
  return true;
}
