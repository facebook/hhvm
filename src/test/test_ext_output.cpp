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

#include <test/test_ext_output.h>
#include <runtime/ext/ext_output.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtOutput::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_ob_start);
  RUN_TEST(test_ob_clean);
  RUN_TEST(test_ob_flush);
  RUN_TEST(test_ob_end_clean);
  RUN_TEST(test_ob_end_flush);
  RUN_TEST(test_flush);
  RUN_TEST(test_ob_get_clean);
  RUN_TEST(test_ob_get_contents);
  RUN_TEST(test_ob_get_flush);
  RUN_TEST(test_ob_get_length);
  RUN_TEST(test_ob_get_level);
  RUN_TEST(test_ob_get_status);
  RUN_TEST(test_ob_gzhandler);
  RUN_TEST(test_ob_implicit_flush);
  RUN_TEST(test_ob_list_handlers);
  RUN_TEST(test_output_add_rewrite_var);
  RUN_TEST(test_output_reset_rewrite_vars);
  RUN_TEST(test_hphp_log);
  RUN_TEST(test_hphp_stats);
  RUN_TEST(test_hphp_get_stats);
  RUN_TEST(test_hphp_output_global_state);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtOutput::test_ob_start() {
  f_ob_start();
  f_ob_start("strtolower");
  g_context->out() << "TEst";
  f_ob_end_flush();
  VS(f_ob_get_clean(), "test");
  return Count(true);
}

bool TestExtOutput::test_ob_clean() {
  f_ob_start();
  g_context->out() << "test";
  f_ob_clean();
  VS(f_ob_get_clean(), "");
  return Count(true);
}

bool TestExtOutput::test_ob_flush() {
  f_ob_start();
  f_ob_start("strtolower");
  g_context->out() << "TEst";
  f_ob_flush();
  VS(f_ob_get_clean(), "");
  VS(f_ob_get_clean(), "test");
  return Count(true);
}

bool TestExtOutput::test_ob_end_clean() {
  f_ob_start();
  f_ob_start("strtolower");
  g_context->out() << "TEst";
  f_ob_end_clean();
  VS(f_ob_get_clean(), "");
  return Count(true);
}

bool TestExtOutput::test_ob_end_flush() {
  f_ob_start();
  f_ob_start("strtolower");
  g_context->out() << "TEst";
  f_ob_end_flush();
  VS(f_ob_get_clean(), "test");
  return Count(true);
}

bool TestExtOutput::test_flush() {
  f_ob_start();
  f_ob_start("strtolower");
  g_context->out() << ""; // we can't really verify what's written to stdout
  f_flush();
  f_ob_end_clean();
  f_ob_end_clean();
  return Count(true);
}

bool TestExtOutput::test_ob_get_clean() {
  f_ob_start();
  f_ob_start();
  g_context->out() << "test";
  VS(f_ob_get_clean(), "test");
  VS(f_ob_get_clean(), "");
  VS(f_ob_get_clean(), "");
  return Count(true);
}

bool TestExtOutput::test_ob_get_contents() {
  f_ob_start();
  g_context->out() << "test";
  VS(f_ob_get_contents(), "test");
  VS(f_ob_get_contents(), "test"); // verifying content stays
  f_ob_end_clean();
  return Count(true);
}

bool TestExtOutput::test_ob_get_flush() {
  f_ob_start();
  f_ob_start();
  g_context->out() << "test";
  VS(f_ob_get_flush(), "test");
  VS(f_ob_get_flush(), "");
  f_ob_end_clean();
  VS(f_ob_get_flush(), "test");
  f_ob_end_clean();
  return Count(true);
}

bool TestExtOutput::test_ob_get_length() {
  f_ob_start();
  g_context->out() << "test";
  VS(f_ob_get_length(), 4);
  f_ob_end_clean();
  return Count(true);
}

bool TestExtOutput::test_ob_get_level() {
  VS(f_ob_get_level(), 0);
  f_ob_start();
  VS(f_ob_get_level(), 1);
  f_ob_end_clean();
  VS(f_ob_get_level(), 0);
  return Count(true);
}

bool TestExtOutput::test_ob_get_status() {
  try {
    f_ob_get_status();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtOutput::test_ob_gzhandler() {
  try {
    f_ob_gzhandler("value", 0);
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtOutput::test_ob_implicit_flush() {
  f_ob_implicit_flush(true);  // no-op currently
  f_ob_implicit_flush(false); // no-op currently
  return Count(true);
}

bool TestExtOutput::test_ob_list_handlers() {
  f_ob_start();
  f_ob_start("test");
  Array handlers = f_ob_list_handlers();
  f_ob_end_clean();
  f_ob_end_clean();
  VS(handlers, CREATE_VECTOR2(null, "test"));
  return Count(true);
}

bool TestExtOutput::test_output_add_rewrite_var() {
  try {
    f_output_add_rewrite_var("name", "value");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtOutput::test_output_reset_rewrite_vars() {
  try {
    f_output_reset_rewrite_vars();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtOutput::test_hphp_log() {
  return Count(true);
}

bool TestExtOutput::test_hphp_stats() {
  return Count(true);
}

bool TestExtOutput::test_hphp_get_stats() {
  return Count(true);
}

bool TestExtOutput::test_hphp_output_global_state() {
  return Count(true);
}
