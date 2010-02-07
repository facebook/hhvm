
#include <test/test_ext_fbintern.h>
#include <cpp/ext/ext_fbintern.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtFbintern::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_fb_set_opcode);
  RUN_TEST(test_fb_reset_opcode);
  RUN_TEST(test_fb_config_coredump);
  RUN_TEST(test_fb_debug_rlog);
  RUN_TEST(test_fb_backtrace);
  RUN_TEST(test_fb_render_wrapped);
  RUN_TEST(test_fb_add_included_file);
  RUN_TEST(test_fb_request_timers);
  RUN_TEST(test_fb_get_ape_version);
  RUN_TEST(test_fb_get_derived_classes);
  RUN_TEST(test_hotprofiler_enable);
  RUN_TEST(test_hotprofiler_disable);
  RUN_TEST(test_phprof_enable);
  RUN_TEST(test_phprof_disable);
  RUN_TEST(test_fql_set_static_data_10);
  RUN_TEST(test_fql_static_data_set_10);
  RUN_TEST(test_fql_parse_10);
  RUN_TEST(test_fql_multiparse_10);
  RUN_TEST(test_xhp_preprocess_code);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtFbintern::test_fb_set_opcode() {
  try {
    f_fb_set_opcode(0, "");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtFbintern::test_fb_reset_opcode() {
  try {
    f_fb_reset_opcode(0);
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtFbintern::test_fb_config_coredump() {
  try {
    f_fb_config_coredump(true, 0);
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtFbintern::test_fb_debug_rlog() {
  // too trivial or obvious to test here
  return Count(true);
}

bool TestExtFbintern::test_fb_backtrace() {
  try {
    f_fb_backtrace();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtFbintern::test_fb_render_wrapped() {
  String text = "abcdefghijk lmnopqrstuvwxyz abcdefghijklmnopqrstuvw xyz "
    "abcdefghijklmnopqrstu vwxyz abcdefghijklmnopqrstuvwxyz"
    "a bcdefghijklmnopqrstuvwx y "
    "abcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxy";
  VS(f_fb_render_wrapped(text),
     "<span>abcdefghijk lmnopqrstuvwxyz abcdefghijklmnopqrstuvw xyz "
     "abcdefghijklmnopqrstu vwxyz abcdefghijklmnopqrstuvwxyz</span><wbr></wbr>"
     "<span class=\"word_break\"></span><span>a bcdefghijklmnopqrstuvwx y "
     "abcdefghijklmnopqrstuvwxya</span><wbr></wbr><span class=\"word_break\">"
     "</span>bcdefghijklmnopqrstuvwxy");
  VS(f_fb_render_wrapped(text, 5),
     "<span>abcde</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>fghij</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>k lmnop</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>qrstu</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>vwxyz</span><wbr></wbr><span "
     "class=\"word_break\"></span><span> abcde</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>fghij</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>klmno</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>pqrst</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>uvw xyz abcde</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>fghij</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>klmno</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>pqrst</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>u vwxyz</span><wbr></wbr><span "
     "class=\"word_break\"></span><span> abcde</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>fghij</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>klmno</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>pqrst</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>uvwxy</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>za bcdef</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>ghijk</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>lmnop</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>qrstu</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>vwx y abcde</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>fghij</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>klmno</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>pqrst</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>uvwxy</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>abcde</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>fghij</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>klmno</span><wbr></wbr><span "
     "class=\"word_break\"></span><span>pqrst</span><wbr></wbr><span "
     "class=\"word_break\"></span>uvwxy");
  VS(f_fb_render_wrapped(text, 19, "head"),
     "headabcdefghijk lmnopqrstuvwxyz "
     "abcdefghijklmnopqrs</span><wbr></wbr><span "
     "class=\"word_break\"></span>headtuvw xyz "
     "abcdefghijklmnopqrs</span><wbr></wbr><span "
     "class=\"word_break\"></span>headtu vwxyz "
     "abcdefghijklmnopqrs</span><wbr></wbr><span "
     "class=\"word_break\"></span>headtuvwxyza "
     "bcdefghijklmnopqrst</span><wbr></wbr><span "
     "class=\"word_break\"></span>headuvwx y "
     "abcdefghijklmnopqrs</span><wbr></wbr><span "
     "class=\"word_break\"></span>headtuvwxyabcdefghijklm</span><wbr></wbr><span "
     "class=\"word_break\"></span>nopqrstuvwxy");
  VS(f_fb_render_wrapped(text, 30, "head", "tail"),
     "headabcdefghijk lmnopqrstuvwxyz abcdefghijklmnopqrstuvw xyz "
     "abcdefghijklmnopqrstu vwxyz abcdefghijklmnopqrstuvwxyza "
     "bcdefghijklmnopqrstuvwx y "
     "abcdefghijklmnopqrstuvwxyabcdetailfghijklmnopqrstuvwxy");
  return Count(true);
}

bool TestExtFbintern::test_fb_add_included_file() {
  try {
    f_fb_add_included_file("");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtFbintern::test_fb_request_timers() {
  VS(f_fb_request_timers(), false);
  return Count(true);
}

bool TestExtFbintern::test_fb_get_ape_version() {
  VS(f_fb_get_ape_version(), "");
  return Count(true);
}

bool TestExtFbintern::test_fb_get_derived_classes() {
  Array ret = f_fb_get_derived_classes("test_file", "test");
  VS(ret.size(), 1);
  VS(ret[0], "test");
  return Count(true);
}

static int test_level_2() {
  ProfilerInjection pi(ThreadInfo::s_threadInfo.get(), __func__);
  int sum = 0;
  for (int i = 0; i < 100000; i++) sum += i;
  return sum;
}
static void test_level_1() {
  ProfilerInjection pi(ThreadInfo::s_threadInfo.get(), __func__);
  test_level_2();
  test_level_2();
}
bool TestExtFbintern::test_hotprofiler_enable() {
  f_hotprofiler_enable(2);
  test_level_1();
#ifdef HOTPROFILER
  Array res = f_hotprofiler_disable().toArray();
  VS(res.size(), 3);
  VS(res["test_level_1==>test_level_2"]["ct"], 2);
#endif
  return Count(true);
}

bool TestExtFbintern::test_hotprofiler_disable() {
  // tested in test_hotprofiler_enable
  return Count(true);
}

bool TestExtFbintern::test_phprof_enable() {
  f_phprof_enable();
  test_level_1();
#ifdef HOTPROFILER
  Array res = f_phprof_disable().toArray();
  VS(res.size(), 3);
  VS(res["test_level_1==>test_level_2"]["ct"], 2);
#endif
  return Count(true);
}

bool TestExtFbintern::test_phprof_disable() {
  // tested in test_phprof_disable
  return Count(true);
}

bool TestExtFbintern::test_fql_set_static_data_10() {
  // tested with PHP unit tests
  return Count(true);
}

bool TestExtFbintern::test_fql_static_data_set_10() {
  // tested with PHP unit tests
  return Count(true);
}

bool TestExtFbintern::test_fql_parse_10() {
  // tested with PHP unit tests
  return Count(true);
}

bool TestExtFbintern::test_fql_multiparse_10() {
  // tested with PHP unit tests
  return Count(true);
}

bool TestExtFbintern::test_xhp_preprocess_code() {
  return Count(true);
}
