/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/util/coro.h"
#include "hphp/util/extern-worker.h"

#include <folly/FileUtil.h>
#include <folly/init/Init.h>
#include <folly/executors/GlobalExecutor.h>

#include <filesystem>

#include <boost/filesystem.hpp>

#include <gtest/gtest.h>

namespace HPHP {

using namespace extern_worker;
namespace coro = folly::coro;

namespace fs = std::filesystem;

namespace {

struct C1 {
  C1() : m_int{0} {}
  C1(int i, std::string s) : m_int{i}, m_str{s} {}
  int m_int;
  std::string m_str;

  template <typename SerDe> void serde(SerDe& sd) {
    sd(m_int)
      (m_str);
  }
};

struct C2 {
  C2() : m_int{0} {}
  C2(int i, std::string s) : m_str{s}, m_int{i} {}

  C2(const C2&) = delete;
  C2(C2&&) = default;
  C2& operator=(const C2&) = delete;
  C2& operator=(C2&&) = default;

  std::string m_str;
  int m_int;

  template <typename SerDe> void serde(SerDe& sd) {
    sd(m_str)
      (m_int);
  }
};

}

TEST(ExternWorker, Blobs) {
  Options options;
  options.setUseSubprocess(Options::UseSubprocess::Always);

  Client client{folly::getGlobalCPUExecutor(), options};

  EXPECT_TRUE(client.usingSubprocess());
  EXPECT_EQ(client.implName(), "subprocess");

  Ref<int> i123 = coro::blockingWait(client.store(123));
  Ref<int> i456 = coro::blockingWait(client.store(456));
  Ref<int> i789 = coro::blockingWait(client.store(789));
  std::tuple<Ref<int>, Ref<int>> t1 = coro::blockingWait(client.store(314, 737));

  Ref<std::string> sABC = coro::blockingWait(client.store(std::string{"abc"}));
  Ref<std::string> sDEF = coro::blockingWait(client.store(std::string{"def"}));
  std::tuple<Ref<std::string>, Ref<std::string>> t2 =
    coro::blockingWait(client.store(std::string{"hello"}, std::string{"good-bye"}));

  Ref<C1> c1_1 = coro::blockingWait(client.store(C1{1, "a"}));
  Ref<C1> c1_2 = coro::blockingWait(client.store(C1{2, "zzzzz"}));

  Ref<C2> c2_1 = coro::blockingWait(client.store(C2{500, "qwerty"}));
  Ref<C2> c2_2 = coro::blockingWait(client.store(C2{101, ""}));

  std::tuple<Ref<std::string>, Ref<int>> t3 =
    coro::blockingWait(client.store(std::string{"118"}, 118));
  std::tuple<Ref<int>, Ref<std::string>> t4 =
    coro::blockingWait(client.store(476, std::string{"476"}));

  std::tuple<Ref<int>, Ref<std::string>, Ref<C1>, Ref<C2>> t5 =
    coro::blockingWait(client.store(845, std::string{"str1"}, C1{591, "str2"}, C2{229, "str3"}));

  std::vector<int> i_list1;
  std::vector<std::string> s_list1;
  std::vector<C1> c1_list1;
  std::vector<C2> c2_list1;
  for (int i = 0; i < 10; ++i) {
    i_list1.emplace_back(500+i);
    s_list1.emplace_back(folly::sformat("list-str-{}", i));
    c1_list1.emplace_back(700+i, folly::sformat("c1-str-{}", i));
    c2_list1.emplace_back(1900+i, folly::sformat("c2-str-{}", i));
  }

  std::tuple<
    std::vector<Ref<int>>,
    std::vector<Ref<std::string>>,
    std::vector<Ref<C1>>,
    std::vector<Ref<C2>>
  > t6 =
  coro::blockingWait(coro::collectAll(
    client.storeMulti(std::move(i_list1)),
    client.storeMulti(std::move(s_list1)),
    client.storeMulti(std::move(c1_list1)),
    client.storeMulti(std::move(c2_list1))
  ));
  EXPECT_EQ(std::get<0>(t6).size(), 10);
  EXPECT_EQ(std::get<1>(t6).size(), 10);
  EXPECT_EQ(std::get<2>(t6).size(), 10);
  EXPECT_EQ(std::get<3>(t6).size(), 10);

  std::vector<std::tuple<std::string, C2, int, C1>> t_list1;
  for (int i = 0; i < 12; ++i) {
    t_list1.emplace_back(
      folly::sformat("t_list1-str1-{}", i),
      C2{863321+i, folly::sformat("t_list1-str2-{}", i)},
      991134+i,
      C1{11345+i, folly::sformat("t_list1-str3-{}", i)}
    );
  }

  std::vector<std::tuple<Ref<std::string>, Ref<C2>, Ref<int>, Ref<C1>>> t_list2 =
    coro::blockingWait(client.storeMultiTuple(std::move(t_list1)));
  EXPECT_EQ(t_list2.size(), 12);

  Ref<std::tuple<C1, C2, std::string, int>> t7 =
    coro::blockingWait(client.store(std::make_tuple(C1{375, "t100"}, C2{376, "t101"}, std::string{"t102"}, 573)));

  std::vector<int> i_list2{{2000, 2001, 2002}};
  Ref<std::vector<int>> i_list3 = coro::blockingWait(client.store(std::move(i_list2)));

  std::vector<std::tuple<std::string, int>> s_list2{
    { std::make_tuple("x200", 3001), std::make_tuple("x201", 3002) }
  };

  std::vector<Ref<std::tuple<std::string, int>>> t_list4 =
    coro::blockingWait(client.storeMulti(std::move(s_list2)));
  EXPECT_EQ(t_list4.size(), 2);

  std::tuple<int, int, int> t8 = coro::blockingWait(coro::collectAll(
    client.load(i789),
    client.load(i123),
    client.load(i456)
  ));
  EXPECT_EQ(std::get<0>(t8), 789);
  EXPECT_EQ(std::get<1>(t8), 123);
  EXPECT_EQ(std::get<2>(t8), 456);

  EXPECT_EQ(coro::blockingWait(client.load(sDEF)), "def");
  EXPECT_EQ(coro::blockingWait(client.load(sABC)), "abc");

  EXPECT_EQ(coro::blockingWait(client.load(std::get<0>(t3))), "118");
  EXPECT_EQ(coro::blockingWait(client.load(std::get<1>(t3))), 118);
  EXPECT_EQ(coro::blockingWait(client.load(std::get<0>(t4))), 476);
  EXPECT_EQ(coro::blockingWait(client.load(std::get<1>(t4))), "476");

  C1 c1_3 = coro::blockingWait(client.load(c1_1));
  EXPECT_EQ(c1_3.m_str, "a");
  EXPECT_EQ(c1_3.m_int, 1);

  C2 c2_3 = coro::blockingWait(client.load(c2_2));
  EXPECT_EQ(c2_3.m_str, "");
  EXPECT_EQ(c2_3.m_int, 101);

  std::tuple<C1, int, C2, std::string> t9 =
    coro::blockingWait(client.load(c1_2, std::get<0>(t1), c2_1, std::get<1>(t2)));
  EXPECT_EQ(std::get<0>(t9).m_int, 2);
  EXPECT_EQ(std::get<0>(t9).m_str, "zzzzz");
  EXPECT_EQ(std::get<1>(t9), 314);
  EXPECT_EQ(std::get<2>(t9).m_int, 500);
  EXPECT_EQ(std::get<2>(t9).m_str, "qwerty");
  EXPECT_EQ(std::get<3>(t9), "good-bye");

  std::vector<int> i_list5 = coro::blockingWait(client.load(std::get<0>(t6)));
  std::vector<std::string> s_list5 = coro::blockingWait(client.load(std::get<1>(t6)));
  std::vector<C1> c1_list5 = coro::blockingWait(client.load(std::get<2>(t6)));
  std::vector<C2> c2_list5 = coro::blockingWait(client.load(std::get<3>(t6)));
  ASSERT_EQ(i_list5.size(), 10);
  ASSERT_EQ(s_list5.size(), 10);
  ASSERT_EQ(c1_list5.size(), 10);
  ASSERT_EQ(c2_list5.size(), 10);

  for (size_t i = 0; i < i_list5.size(); ++i) {
    EXPECT_EQ(i_list5[i], 500+i);
    EXPECT_EQ(s_list5[i], folly::sformat("list-str-{}", i));
    EXPECT_EQ(c1_list5[i].m_int, 700+i);
    EXPECT_EQ(c1_list5[i].m_str, folly::sformat("c1-str-{}", i));
    EXPECT_EQ(c2_list5[i].m_int, 1900+i);
    EXPECT_EQ(c2_list5[i].m_str, folly::sformat("c2-str-{}", i));
  }

  std::vector<std::tuple<std::string, C2, int, C1>> t_list5 =
    coro::blockingWait(client.load(t_list2));
  ASSERT_EQ(t_list5.size(), 12);
  for (size_t i = 0; i < 12; ++i){
    EXPECT_EQ(std::get<0>(t_list5[i]), folly::sformat("t_list1-str1-{}", i));
    EXPECT_EQ(std::get<1>(t_list5[i]).m_int, 863321+i);
    EXPECT_EQ(std::get<1>(t_list5[i]).m_str, folly::sformat("t_list1-str2-{}", i));
    EXPECT_EQ(std::get<2>(t_list5[i]), 991134+i);
    EXPECT_EQ(std::get<3>(t_list5[i]).m_int, 11345+i);
    EXPECT_EQ(std::get<3>(t_list5[i]).m_str, folly::sformat("t_list1-str3-{}", i));
  }

  std::tuple<C1, C2, std::string, int> t10 =
    coro::blockingWait(client.load(t7));
  EXPECT_EQ(std::get<0>(t10).m_int, 375);
  EXPECT_EQ(std::get<0>(t10).m_str, "t100");
  EXPECT_EQ(std::get<1>(t10).m_int, 376);
  EXPECT_EQ(std::get<1>(t10).m_str, "t101");
  EXPECT_EQ(std::get<2>(t10), "t102");
  EXPECT_EQ(std::get<3>(t10), 573);

  std::vector<std::tuple<std::string, int>> t_list6 =
    coro::blockingWait(client.load(t_list4));
  ASSERT_EQ(t_list6.size(), 2);
  EXPECT_EQ(std::get<0>(t_list6[0]), "x200");
  EXPECT_EQ(std::get<1>(t_list6[0]), 3001);
  EXPECT_EQ(std::get<0>(t_list6[1]), "x201");
  EXPECT_EQ(std::get<1>(t_list6[1]), 3002);

  std::vector<int> i_list6 = coro::blockingWait(client.load(i_list3));
  ASSERT_EQ(i_list6.size(), 3);
  EXPECT_EQ(i_list6[0], 2000);
  EXPECT_EQ(i_list6[1], 2001);
  EXPECT_EQ(i_list6[2], 2002);
}

TEST(ExternWorker, Files) {
  auto const tmpDir = fs::temp_directory_path();
  auto const base = tmpDir / "hphp-extern-worker-test";
  auto const root =
    base / boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%").native();

  fs::create_directory(base, tmpDir);
  fs::create_directory(root, base);

  auto const makeString = [] (fs::path p) {
    return folly::sformat("This file is called: \"{}\"", p.native());
  };
  auto const makeFile = [&] {
    auto const p =
      root / boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%").native();
    folly::writeFile(makeString(p), p.c_str());
    return p;
  };

  Options options;
  options.setUseSubprocess(Options::UseSubprocess::Always);

  Client client{folly::getGlobalCPUExecutor(), options};

  EXPECT_TRUE(client.usingSubprocess());
  EXPECT_EQ(client.implName(), "subprocess");

  auto const p1 = makeFile();
  auto const p2 = makeFile();
  auto const p3 = makeFile();
  std::vector<fs::path> ps;
  for (size_t i = 0; i < 5; ++i) ps.emplace_back(makeFile());

  Ref<std::string> r1 = coro::blockingWait(client.storeFile(p1));
  EXPECT_EQ(client.getStats().files.load(), 1);
  EXPECT_EQ(client.getStats().filesUploaded.load(), 0);

  std::tuple<Ref<std::string>, Ref<std::string>> t1 = coro::blockingWait(coro::collectAll(
    client.storeFile(p2),
    client.storeFile(p3)
  ));
  EXPECT_EQ(client.getStats().files.load(), 3);
  EXPECT_EQ(client.getStats().filesUploaded.load(), 0);
  Ref<std::string> r2 = std::get<0>(t1);
  Ref<std::string> r3 = std::get<1>(t1);

  std::vector<Ref<std::string>> refs = coro::blockingWait(client.storeFile(ps));
  EXPECT_EQ(refs.size(), ps.size());
  EXPECT_EQ(client.getStats().files.load(), 3 + ps.size());
  EXPECT_EQ(client.getStats().filesUploaded.load(), 0);

  std::string s1 = coro::blockingWait(client.load(r1));
  EXPECT_EQ(s1, makeString(p1));

  std::tuple<std::string, std::string> t2 = coro::blockingWait(client.load(r2, r3));
  EXPECT_EQ(std::get<0>(t2), makeString(p2));
  EXPECT_EQ(std::get<1>(t2), makeString(p3));

  std::vector<std::string> strs = coro::blockingWait(client.load(refs));
  ASSERT_EQ(strs.size(), ps.size());

  for (size_t i = 0; i < strs.size(); ++i) {
    EXPECT_EQ(strs[i], makeString(ps[i]));
  }

  fs::remove_all(root);
}

namespace {

struct Test1 {
  static std::string name() { return "test-job1"; }
  static void fini() {}
  static void init() {}
  static int run() { return 123; }
};
struct Test2 {
  static std::string name() { return "test-job2"; }
  static void fini() {}
  static void init() {}
  static std::string run() { return "abc"; }
};
struct Test3 {
  static std::string name() { return "test-job3"; }
  static void fini() {}
  static void init(int i) { s_add = i; }
  static int run(int x) { return s_add + x; }
  static int s_add;
};
struct Test4 {
  static std::string name() { return "test-job4"; }
  static void fini() {}
  static void init(const std::string& s) { s_add = s; }
  static std::string run(const std::string& x) { return s_add + x; }
  static std::string s_add;
};
struct Test5 {
  static std::string name() { return "test-job5"; }
  static void fini() {}
  static void init(const std::string& s, int i) {
    s_add = folly::sformat("{}-{}", s, i);
  }
  static std::string run(int i, const std::string& s) {
    return folly::sformat("{}-{}-{}", s_add, i, s);
  }
  static std::string s_add;
};
struct Test6 {
  static std::string name() { return "test-job6"; }
  static void fini() {}
  static void init(const C1& c1, const C2& c2) {
    s_add = folly::sformat("{}/{}/{}/{}",
                           c1.m_int, c1.m_str,
                           c2.m_int, c2.m_str);
  }
  static std::string run(const C2& c2, const C1& c1) {
    return folly::sformat("{}/{}/{}/{}/{}",
                          s_add,
                          c2.m_int, c2.m_str,
                          c1.m_int, c1.m_str);
  }
  static std::string s_add;
};
struct Test7 {
  static std::string name() { return "test-job7"; }
  static void fini() {}
  static void init() {}
  static Opt<std::string> run(Opt<int> x) {
    if (!x.val.has_value()) return Opt<std::string>{};
    return Opt<std::string>{std::to_string(*x.val)};
  }
};
struct Test8 {
  static std::string name() { return "test-job8"; }
  static void fini() {}
  static void init() {}
  static Variadic<std::string> run(Variadic<int> v) {
    Variadic<std::string> out;
    for (auto x : v.vals) {
      out.vals.emplace_back(std::to_string(x));
    }
    return out;
  }
};
struct Test9 {
  static std::string name() { return "test-job9"; }
  static void fini() {}
  static void init() {}
  static Multi<std::string, int> run(int x) {
    return Multi<std::string, int>{std::make_tuple(std::to_string(x), x)};
  }
};
struct Test10 {
  static std::string name() { return "test-job10"; }
  static void fini() {}
  static void init() {}
  static Multi<int, std::string, Variadic<std::string>, Opt<C1>, C2>
  run(int x,
      std::string s,
      Variadic<std::string> v,
      Opt<C1> o,
      C2 c) {
    return Multi<int, std::string, Variadic<std::string>, Opt<C1>, C2>{
      std::make_tuple(x, s, v, o, std::move(c))
    };
  }
};
struct Test11 {
  static std::string name() { return "test-job11"; }
  static void fini() {}
  static void init() {}
  static Optional<int> run(const Optional<int>& x) { return x; }
};
struct Test12 {
  static std::string name() { return "test-job12"; }
  static void fini() {}
  static void init() {}
  static std::vector<int> run(const std::vector<int>& x) { return x; }
};
struct Test13 {
  static std::string name() { return "test-job13"; }
  static void fini() {}
  static void init() {}
  static std::tuple<int, std::string>
  run(const std::tuple<int, std::string>& x) { return x; }
};

struct Test14 {
  static std::string name() { return "test-job14"; }
  static std::vector<int> fini() { return s_fini; }
  static void init() {}
  static int run() {
    auto const i = s_fini.size();
    s_fini.emplace_back(i + 100);
    return i + 200;
  }

  static std::vector<int> s_fini;
};

int Test3::s_add{0};
std::string Test4::s_add;
std::string Test5::s_add;
std::string Test6::s_add;
std::vector<int> Test14::s_fini;

Job<Test1> s_test1;
Job<Test2> s_test2;
Job<Test3> s_test3;
Job<Test4> s_test4;
Job<Test5> s_test5;
Job<Test6> s_test6;
Job<Test7> s_test7;
Job<Test8> s_test8;
Job<Test9> s_test9;
Job<Test10> s_test10;
Job<Test11> s_test11;
Job<Test12> s_test12;
Job<Test13> s_test13;
Job<Test14> s_test14;

}

TEST(ExternWorker, Exec) {
  Options options;
  options.setUseSubprocess(Options::UseSubprocess::Always);

  Client client{folly::getGlobalCPUExecutor(), options};

  EXPECT_TRUE(client.usingSubprocess());
  EXPECT_EQ(client.implName(), "subprocess");

  auto const testJob1_2 = [&] (size_t size) {
    std::vector<std::tuple<>> inputs;
    for (size_t i = 0; i < size; ++i) {
      inputs.emplace_back(std::tuple<>{});
    }

    std::tuple<std::vector<Ref<int>>, std::vector<Ref<std::string>>> outputsT =
      coro::blockingWait(coro::collectAll(
        client.exec(s_test1, {}, inputs),
        client.exec(s_test2, {}, inputs)
      ));

    ASSERT_EQ(std::get<0>(outputsT).size(), size);
    ASSERT_EQ(std::get<1>(outputsT).size(), size);

    std::vector<int> outputs1 =
      coro::blockingWait(client.load(std::get<0>(outputsT)));
    std::vector<std::string> outputs2 =
      coro::blockingWait(client.load(std::get<1>(outputsT)));
    ASSERT_EQ(outputs1.size(), size);
    ASSERT_EQ(outputs2.size(), size);

    for (size_t i = 0; i < size; ++i) {
      EXPECT_EQ(outputs1[i], 123);
      EXPECT_EQ(outputs2[i], "abc");
    }
  };
  testJob1_2(0);
  testJob1_2(1);
  testJob1_2(2);
  testJob1_2(5);

  auto const testJob3_4 = [&] (size_t size, int add, std::string prefix) {
    std::vector<std::tuple<Ref<int>>> inputs1;
    std::vector<std::tuple<Ref<std::string>>> inputs2;
    for (size_t i = 0; i < size; ++i) {
      inputs1.emplace_back(std::make_tuple(coro::blockingWait(client.store(int(i+100)))));
      inputs2.emplace_back(std::make_tuple(coro::blockingWait(client.store(folly::sformat("str-{}", i+100)))));
    }

    Ref<int> addRef = coro::blockingWait(client.store(add));
    Ref<std::string> prefixRef = coro::blockingWait(client.store(prefix));

    std::vector<Ref<int>> refs1 =
      coro::blockingWait(client.exec(s_test3, std::make_tuple(addRef), inputs1));
    std::vector<Ref<std::string>> refs2 =
      coro::blockingWait(client.exec(s_test4, std::make_tuple(prefixRef), inputs2));
    ASSERT_EQ(refs1.size(), size);
    ASSERT_EQ(refs2.size(), size);

    std::vector<int> outputs1 = coro::blockingWait(client.load(refs1));
    std::vector<std::string> outputs2 = coro::blockingWait(client.load(refs2));
    ASSERT_EQ(outputs1.size(), size);
    ASSERT_EQ(outputs2.size(), size);

    for (size_t i = 0; i < size; ++i) {
      EXPECT_EQ(outputs1[i], i+100+add);
      EXPECT_EQ(outputs2[i], folly::sformat("{}str-{}", prefix, i+100));
    }
  };
  testJob3_4(0, 1, "prefix1");
  testJob3_4(1, 50, "prefix2");
  testJob3_4(2, 1111, "prefix3");
  testJob3_4(7, 1234, "prefix4");

  auto const testJob5 = [&] (size_t size, int init1, std::string init2) {
    std::tuple<Ref<std::string>, Ref<int>> inits =
      coro::blockingWait(client.store(init2, init1));

    std::vector<std::tuple<Ref<int>, Ref<std::string>>> inputs;
    for (size_t i = 0; i < size; ++i) {
      inputs.emplace_back(
        coro::blockingWait(client.store(int(i + 891), folly::sformat("hello-{}", i)))
      );
    }

    std::vector<Ref<std::string>> refs =
      coro::blockingWait(client.exec(s_test5, inits, inputs));
    ASSERT_EQ(refs.size(), size);

    std::vector<std::string> outputs = coro::blockingWait(client.load(refs));
    ASSERT_EQ(outputs.size(), size);

    for (size_t i = 0; i < size; ++i) {
      EXPECT_EQ(outputs[i], folly::sformat("{}-{}-{}-hello-{}", init2, init1, i+891, i));
    }
  };
  testJob5(0, 1, "bye1");
  testJob5(1, 27, "bye2");
  testJob5(3, 91, "bye3");

  auto const testJob6 = [&] (size_t size, C1 c1, C2 c2) {
    C2 c2_copy{c2.m_int, c2.m_str};

    std::tuple<Ref<C1>, Ref<C2>> inits =
      coro::blockingWait(client.store(c1, std::move(c2)));

    std::vector<std::tuple<Ref<C2>, Ref<C1>>> inputs;
    for (size_t i = 0; i < size; ++i) {
      C1 c1_2{int(i*2+1), std::to_string(i*2+1)};
      C2 c2_2{int(i*2), std::to_string(i*2)};
      inputs.emplace_back(
        coro::blockingWait(client.store(std::move(c2_2), c1_2))
      );
    }

    std::vector<Ref<std::string>> refs =
      coro::blockingWait(client.exec(s_test6, inits, inputs));
    ASSERT_EQ(refs.size(), size);

    std::vector<std::string> outputs = coro::blockingWait(client.load(refs));
    ASSERT_EQ(outputs.size(), size);

    for (size_t i = 0; i < size; ++i) {
      C1 c1_2{int(i*2+1), std::to_string(i*2+1)};
      C2 c2_2{int(i*2), std::to_string(i*2)};
      EXPECT_EQ(
        outputs[i],
        folly::sformat("{}/{}/{}/{}/{}/{}/{}/{}",
                       c1.m_int, c1.m_str,
                       c2_copy.m_int, c2_copy.m_str,
                       c2_2.m_int, c2_2.m_str,
                       c1_2.m_int, c1_2.m_str)
      );
    }
  };
  testJob6(0, C1{1, "a"}, C2{7, "b"});
  testJob6(1, C1{3, "c"}, C2{9, "d"});
  testJob6(2, C1{5, "e"}, C2{11, "f"});

  auto const testJob7 = [&] (size_t size) {
    std::vector<std::tuple<Optional<Ref<int>>>> inputs;
    for (size_t i = 0; i < size; ++i) {
      if ((i % 2) == 0) {
        inputs.emplace_back(std::nullopt);
      } else {
        inputs.emplace_back(std::make_tuple(coro::blockingWait(client.store(int(i)))));
      }
    }

    std::vector<Optional<Ref<std::string>>> refs =
      coro::blockingWait(client.exec(s_test7, {}, inputs));
    ASSERT_EQ(refs.size(), size);

    std::vector<Ref<std::string>> filtered;
    for (size_t i = 0; i < size; ++i) {
      if ((i % 2) == 0) {
        EXPECT_FALSE(refs[i].has_value());
      } else {
        ASSERT_TRUE(refs[i].has_value());
        filtered.emplace_back(*refs[i]);
      }
    }
    EXPECT_EQ(filtered.size(), size / 2);

    std::vector<std::string> outputs = coro::blockingWait(client.load(filtered));
    ASSERT_EQ(outputs.size(), size / 2);
    for (size_t i = 0; i < size; ++i) {
      if ((i % 2) == 0) continue;
      EXPECT_EQ(outputs[i / 2], std::to_string(i));
    }
  };
  testJob7(0);
  testJob7(1);
  testJob7(2);
  testJob7(8);

  auto const testJob8 = [&] (size_t size) {
    std::vector<Ref<int>> inputs;
    for (size_t i = 0; i < size; ++i) {
      inputs.emplace_back(coro::blockingWait(client.store(int(i))));
    }

    std::vector<std::vector<Ref<std::string>>> refs =
      coro::blockingWait(client.exec(s_test8, {}, {std::make_tuple(inputs)}));
    ASSERT_EQ(refs.size(), 1);
    ASSERT_EQ(refs[0].size(), size);

    std::vector<std::string> outputs = coro::blockingWait(client.load(refs[0]));
    ASSERT_EQ(outputs.size(), size);

    for (size_t i = 0; i < size; ++i) {
      EXPECT_EQ(outputs[i], std::to_string(i));
    }
  };
  testJob8(0);
  testJob8(1);
  testJob8(2);
  testJob8(7);

  auto const testJob9 = [&] (size_t size) {
    std::vector<std::tuple<Ref<int>>> inputs;
    for (size_t i = 0; i < size; ++i) {
      inputs.emplace_back(
        std::make_tuple(coro::blockingWait(client.store(int(i+123))))
      );
    }

    std::vector<std::tuple<Ref<std::string>, Ref<int>>> refs =
      coro::blockingWait(client.exec(s_test9, {}, inputs));
    ASSERT_EQ(refs.size(), size);

    std::vector<std::tuple<std::string, int>> outputs =
      coro::blockingWait(client.load(refs));
    ASSERT_EQ(outputs.size(), size);

    for (size_t i = 0; i < size; ++i) {
      EXPECT_EQ(std::get<0>(outputs[i]), std::to_string(i+123));
      EXPECT_EQ(std::get<1>(outputs[i]), i+123);
    }
  };
  testJob9(0);
  testJob9(1);
  testJob9(2);
  testJob9(3);
  testJob9(5);

  auto const testJob10 = [&] (size_t size, size_t numStrs) {
    std::vector<
      std::tuple<
        Ref<int>,
        Ref<std::string>,
        std::vector<Ref<std::string>>,
        Optional<Ref<C1>>,
        Ref<C2>
      >
    > inputs;

    for (size_t i = 0; i < size; ++i) {
      std::vector<std::string> strs;
      for (size_t j = 0; j < numStrs; ++j) {
        strs.emplace_back(folly::sformat("some-str-{}", j));
      }
      std::vector<Ref<std::string>> strRefs =
        coro::blockingWait(client.storeMulti(strs));

      Optional<Ref<C1>> opt;
      if ((i % 2) == 0) {
        opt.emplace(
          coro::blockingWait(client.store(C1{int(i+200), std::to_string(i+200)}))
        );
      }

      std::tuple<Ref<int>, Ref<std::string>, Ref<C2>> stores =
        coro::blockingWait(
          client.store(
            int(i),
            folly::sformat("another-str-{}", i),
            C2{int(i+300), std::to_string(i+300)}
          )
        );

      inputs.emplace_back(
        std::get<0>(stores),
        std::get<1>(stores),
        strRefs,
        opt,
        std::get<2>(stores)
      );
    }

    std::vector<
      std::tuple<
        Ref<int>,
        Ref<std::string>,
        std::vector<Ref<std::string>>,
        Optional<Ref<C1>>,
        Ref<C2>
      >
    > refs = coro::blockingWait(client.exec(s_test10, {}, inputs));

    ASSERT_EQ(refs.size(), size);
    for (size_t i = 0; i < size; ++i) {
      auto const& r = refs[i];

      ASSERT_EQ(std::get<2>(r).size(), numStrs);
      std::vector<std::string> strs = coro::blockingWait(client.load(std::get<2>(r)));
      ASSERT_EQ(strs.size(), numStrs);
      for (size_t j = 0; j < numStrs; ++j) {
        EXPECT_EQ(strs[j], folly::sformat("some-str-{}", j));
      }

      if ((i % 2) == 0) {
        ASSERT_TRUE(std::get<3>(r).has_value());
        C1 c = coro::blockingWait(client.load(*std::get<3>(r)));
        EXPECT_EQ(c.m_int, i+200);
        EXPECT_EQ(c.m_str, std::to_string(i+200));
      } else {
        ASSERT_FALSE(std::get<3>(r).has_value());
      }

      std::tuple<int, std::string, C2> t =
        coro::blockingWait(
          client.load(
            std::get<0>(r),
            std::get<1>(r),
            std::get<4>(r)
          )
        );
      EXPECT_EQ(std::get<0>(t), i);
      EXPECT_EQ(std::get<1>(t), folly::sformat("another-str-{}", i));
      EXPECT_EQ(std::get<2>(t).m_int, i+300);
      EXPECT_EQ(std::get<2>(t).m_str, std::to_string(i+300));
    }
  };
  testJob10(0, 1);
  testJob10(1, 0);
  testJob10(1, 1);
  testJob10(1, 10);
  testJob10(3, 5);
  testJob10(3, 0);
  testJob10(5, 7);
  testJob10(5, 1);

  auto const testJob11 = [&] {
    std::vector<std::tuple<Ref<Optional<int>>>> inputs;
    inputs.emplace_back(
      std::make_tuple(coro::blockingWait(client.store(Optional<int>{})))
    );
    inputs.emplace_back(
      std::make_tuple(coro::blockingWait(client.store(Optional<int>{11223344})))
    );

    std::vector<Ref<Optional<int>>> refs =
      coro::blockingWait(client.exec(s_test11, {}, inputs));
    ASSERT_EQ(refs.size(), 2);

    std::vector<Optional<int>> outputs = coro::blockingWait(client.load(refs));
    ASSERT_EQ(outputs.size(), 2);

    ASSERT_FALSE(outputs[0].has_value());
    ASSERT_TRUE(outputs[1].has_value());
    EXPECT_EQ(*outputs[1], 11223344);
  };
  testJob11();

  auto const testJob12 = [&] {
    std::vector<std::tuple<Ref<std::vector<int>>>> inputs;
    inputs.emplace_back(
      std::make_tuple(coro::blockingWait(client.store(std::vector<int>{})))
    );
    inputs.emplace_back(
      std::make_tuple(coro::blockingWait(client.store(std::vector<int>{99})))
    );
    inputs.emplace_back(
      std::make_tuple(coro::blockingWait(client.store(std::vector<int>{97, 32})))
    );

    std::vector<Ref<std::vector<int>>> refs =
      coro::blockingWait(client.exec(s_test12, {}, inputs));
    ASSERT_EQ(refs.size(), 3);

    std::vector<std::vector<int>> outputs = coro::blockingWait(client.load(refs));
    ASSERT_EQ(outputs.size(), 3);

    ASSERT_EQ(outputs[0].size(), 0);
    ASSERT_EQ(outputs[1].size(), 1);
    ASSERT_EQ(outputs[2].size(), 2);

    EXPECT_EQ(outputs[1][0], 99);
    EXPECT_EQ(outputs[2][0], 97);
    EXPECT_EQ(outputs[2][1], 32);
  };
  testJob12();

  auto const testJob13 = [&] {
    Ref<std::tuple<int, std::string>> inputs =
      coro::blockingWait(client.store(std::make_tuple(582, std::string{"tuple"})));

    std::vector<Ref<std::tuple<int, std::string>>> refs =
      coro::blockingWait(client.exec(s_test13, {}, {std::make_tuple(inputs)}));
    ASSERT_EQ(refs.size(), 1);

    std::tuple<int, std::string> output = coro::blockingWait(client.load(refs[0]));
    EXPECT_EQ(std::get<0>(output), 582);
    EXPECT_EQ(std::get<1>(output), "tuple");
  };
  testJob13();
}

TEST(ExternWorker, Fini) {
  Options options;
  options.setUseSubprocess(Options::UseSubprocess::Always);

  Client client{folly::getGlobalCPUExecutor(), options};

  EXPECT_TRUE(client.usingSubprocess());
  EXPECT_EQ(client.implName(), "subprocess");

  auto const testJob14Empty = [&] {
    std::tuple<std::vector<Ref<int>>, Ref<std::vector<int>>> refs =
      coro::blockingWait(client.exec(s_test14, {}, {}));
    ASSERT_TRUE(std::get<0>(refs).empty());

    std::vector<int> fini = coro::blockingWait(client.load(std::get<1>(refs)));
    ASSERT_TRUE(fini.empty());
  };
  testJob14Empty();

  auto const testJob14NotEmpty = [&] {
    std::vector<std::tuple<>> inputs;
    inputs.resize(3);

    std::tuple<std::vector<Ref<int>>, Ref<std::vector<int>>> refs =
      coro::blockingWait(client.exec(s_test14, {}, inputs));
    std::vector<Ref<int>> outputRefs = std::get<0>(refs);
    ASSERT_EQ(outputRefs.size(), 3);

    std::vector<int> outputs = coro::blockingWait(client.load(outputRefs));
    ASSERT_EQ(outputs.size(), 3);

    EXPECT_EQ(outputs[0], 200);
    EXPECT_EQ(outputs[1], 201);
    EXPECT_EQ(outputs[2], 202);

    std::vector<int> fini = coro::blockingWait(client.load(std::get<1>(refs)));
    ASSERT_EQ(fini.size(), 3);
    EXPECT_EQ(fini[0], 100);
    EXPECT_EQ(fini[1], 101);
    EXPECT_EQ(fini[2], 102);
  };
  testJob14NotEmpty();
}

TEST(ExternWorker, RefCache) {
  Options options;
  options.setUseSubprocess(Options::UseSubprocess::Always);

  Client client{folly::getGlobalCPUExecutor(), options};

  EXPECT_TRUE(client.usingSubprocess());
  EXPECT_EQ(client.implName(), "subprocess");

  RefCache<int, std::string> cache{client};

  std::vector<coro::Task<Ref<std::string>>> tasks;
  tasks.emplace_back(cache.get(1, "1", folly::getGlobalCPUExecutor()));
  tasks.emplace_back(cache.get(2, "2", folly::getGlobalCPUExecutor()));
  tasks.emplace_back(cache.get(3, "3", folly::getGlobalCPUExecutor()));
  tasks.emplace_back(cache.get(2, "2", folly::getGlobalCPUExecutor()));
  tasks.emplace_back(cache.get(1, "1", folly::getGlobalCPUExecutor()));

  std::vector<Ref<std::string>> refs =
    coro::blockingWait(coro::collectAllRange(std::move(tasks)));
  ASSERT_EQ(refs.size(), 5);

  EXPECT_EQ(refs[0].id(), refs[4].id());
  EXPECT_EQ(refs[1].id(), refs[3].id());

  std::vector<std::string> strs = coro::blockingWait(client.load(refs));
  ASSERT_EQ(strs.size(), 5);
  EXPECT_EQ(strs[0], "1");
  EXPECT_EQ(strs[1], "2");
  EXPECT_EQ(strs[2], "3");
  EXPECT_EQ(strs[3], "2");
  EXPECT_EQ(strs[4], "1");
}

TEST(ExternWorker, Fallback) {
  Options options;
  options.setUseSubprocess(Options::UseSubprocess::Always);

  Client client{folly::getGlobalCPUExecutor(), options};
  client.forceFallback();

  auto str1 = coro::blockingWait(client.store(std::string{"str1"}));
  auto [str2, int1] = coro::blockingWait(client.store(std::string{"str2"}, 100));

  auto strs = coro::blockingWait(
    client.storeMulti(std::vector<std::string>{"str3", "str4", "str5"})
  );
  ASSERT_EQ(strs.size(), 3);
  auto str3 = std::move(strs[0]);
  auto str4 = std::move(strs[1]);
  auto str5 = std::move(strs[2]);

  auto tuples = coro::blockingWait(
    client.storeMultiTuple(
      std::vector<std::tuple<std::string, int>>{
        std::make_tuple(std::string{"str6"}, 200),
        std::make_tuple(std::string{"str7"}, 300),
        std::make_tuple(std::string{"str8"}, 400)
      }
    )
  );
  ASSERT_EQ(tuples.size(), 3);
  auto [str6, int2] = std::move(tuples[0]);
  auto [str7, int3] = std::move(tuples[1]);
  auto [str8, int4] = std::move(tuples[2]);

  auto const tmpDir = fs::temp_directory_path();
  auto const base = tmpDir / "hphp-extern-worker-test";
  auto const root =
    base / boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%").native();

  fs::create_directory(base, tmpDir);
  fs::create_directory(root, base);

  auto const makeString = [] (fs::path p) {
    return folly::sformat("This file is called: \"{}\"", p.native());
  };
  auto const makeFile = [&] {
    auto const p =
      root / boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%").native();
    folly::writeFile(makeString(p), p.c_str());
    return p;
  };

  auto const file1 = makeFile();
  auto const file2 = makeFile();
  auto const file3 = makeFile();

  auto str9 = coro::blockingWait(client.storeFile(file1));
  auto strs2 = coro::blockingWait(
    client.storeFile(std::vector<fs::path>{file2, file3})
  );
  ASSERT_EQ(strs2.size(), 2);
  auto str10 = std::move(strs2[0]);
  auto str11 = std::move(strs2[1]);

  client.unforceFallback();

  auto [str12, str13, str14] = coro::blockingWait(
    client.store(
      std::string{"str12"},
      std::string{"str13"},
      std::string{"str14"}
    )
  );
  auto [int5, int6, int7, int8, int9, int10] = coro::blockingWait(
    client.store(500, 600, 700, 800, 900, 1000)
  );

  auto const file4 = makeFile();
  auto const file5 = makeFile();

  auto strs3 = coro::blockingWait(
    client.storeFile(std::vector<fs::path>{file4, file5})
  );
  ASSERT_EQ(strs3.size(), 2);
  auto str15 = std::move(strs3[0]);
  auto str16 = std::move(strs3[1]);

  auto str17 = coro::blockingWait(client.store(std::string{"str17"}));

  EXPECT_TRUE(str1.fromFallback());
  EXPECT_TRUE(str2.fromFallback());
  EXPECT_TRUE(str3.fromFallback());
  EXPECT_TRUE(str4.fromFallback());
  EXPECT_TRUE(str5.fromFallback());
  EXPECT_TRUE(str6.fromFallback());
  EXPECT_TRUE(str7.fromFallback());
  EXPECT_TRUE(str8.fromFallback());
  EXPECT_TRUE(str9.fromFallback());
  EXPECT_TRUE(str10.fromFallback());
  EXPECT_TRUE(str11.fromFallback());

  EXPECT_TRUE(int1.fromFallback());
  EXPECT_TRUE(int2.fromFallback());
  EXPECT_TRUE(int3.fromFallback());
  EXPECT_TRUE(int4.fromFallback());

  EXPECT_FALSE(str12.fromFallback());
  EXPECT_FALSE(str13.fromFallback());
  EXPECT_FALSE(str14.fromFallback());
  EXPECT_FALSE(str15.fromFallback());
  EXPECT_FALSE(str16.fromFallback());
  EXPECT_FALSE(str17.fromFallback());

  EXPECT_FALSE(int5.fromFallback());
  EXPECT_FALSE(int6.fromFallback());
  EXPECT_FALSE(int7.fromFallback());
  EXPECT_FALSE(int8.fromFallback());

  EXPECT_EQ(coro::blockingWait(client.load(str1)), "str1");
  EXPECT_EQ(coro::blockingWait(client.load(str12)), "str12");

  auto [l1, l2, l3] = coro::blockingWait(client.load(str2, str3, str9));
  EXPECT_EQ(l1, "str2");
  EXPECT_EQ(l2, "str3");
  EXPECT_EQ(l3, makeString(file1));

  auto [l4, l5, l6] = coro::blockingWait(client.load(str1, str12, str3));
  EXPECT_EQ(l4, "str1");
  EXPECT_EQ(l5, "str12");
  EXPECT_EQ(l6, "str3");

  auto [l7, l8, l9] = coro::blockingWait(client.load(str13, str3, str14));
  EXPECT_EQ(l7, "str13");
  EXPECT_EQ(l8, "str3");
  EXPECT_EQ(l9, "str14");

  auto [l10, l11, l12] = coro::blockingWait(client.load(int5, str14, int2));
  EXPECT_EQ(l10, 500);
  EXPECT_EQ(l11, "str14");
  EXPECT_EQ(l12, 200);

  auto [l13, l14,l15] = coro::blockingWait(client.load(str10, int2, str16));
  EXPECT_EQ(l13, makeString(file2));
  EXPECT_EQ(l14, 200);
  EXPECT_EQ(l15, makeString(file5));

  auto strs4 = coro::blockingWait(
    client.load(std::vector<Ref<std::string>>{str2, str3, str1})
  );
  ASSERT_EQ(strs4.size(), 3);
  EXPECT_EQ(strs4[0], "str2");
  EXPECT_EQ(strs4[1], "str3");
  EXPECT_EQ(strs4[2], "str1");

  auto strs5 = coro::blockingWait(
    client.load(std::vector<Ref<std::string>>{str4, str13, str2})
  );
  ASSERT_EQ(strs5.size(), 3);
  EXPECT_EQ(strs5[0], "str4");
  EXPECT_EQ(strs5[1], "str13");
  EXPECT_EQ(strs5[2], "str2");

  auto strs6 = coro::blockingWait(
    client.load(std::vector<Ref<std::string>>{str12, str4, str15})
  );
  ASSERT_EQ(strs6.size(), 3);
  EXPECT_EQ(strs6[0], "str12");
  EXPECT_EQ(strs6[1], "str4");
  EXPECT_EQ(strs6[2], makeString(file4));

  auto strs7 = coro::blockingWait(
    client.load(std::vector<Ref<std::string>>{str3, str6, str14})
  );
  ASSERT_EQ(strs7.size(), 3);
  EXPECT_EQ(strs7[0], "str3");
  EXPECT_EQ(strs7[1], "str6");
  EXPECT_EQ(strs7[2], "str14");

  auto strs8 = coro::blockingWait(
    client.load(std::vector<Ref<std::string>>{str12, str15, str1})
  );
  ASSERT_EQ(strs8.size(), 3);
  EXPECT_EQ(strs8[0], "str12");
  EXPECT_EQ(strs8[1], makeString(file4));
  EXPECT_EQ(strs8[2], "str1");

  auto v1 = coro::blockingWait(
    client.load(
      std::vector<std::tuple<Ref<std::string>, Ref<int>>>{
        std::make_tuple(str1, int1),
        std::make_tuple(str2, int2),
        std::make_tuple(str3, int3)
      }
    )
  );
  ASSERT_EQ(v1.size(), 3);
  EXPECT_EQ(std::get<0>(v1[0]), "str1");
  EXPECT_EQ(std::get<0>(v1[1]), "str2");
  EXPECT_EQ(std::get<0>(v1[2]), "str3");
  EXPECT_EQ(std::get<1>(v1[0]), 100);
  EXPECT_EQ(std::get<1>(v1[1]), 200);
  EXPECT_EQ(std::get<1>(v1[2]), 300);

  auto v2 = coro::blockingWait(
    client.load(
      std::vector<std::tuple<Ref<std::string>, Ref<int>>>{
        std::make_tuple(str13, int3),
        std::make_tuple(str2, int6),
        std::make_tuple(str14, int1)
      }
    )
  );
  ASSERT_EQ(v2.size(), 3);
  EXPECT_EQ(std::get<0>(v2[0]), "str13");
  EXPECT_EQ(std::get<0>(v2[1]), "str2");
  EXPECT_EQ(std::get<0>(v2[2]), "str14");
  EXPECT_EQ(std::get<1>(v2[0]), 300);
  EXPECT_EQ(std::get<1>(v2[1]), 600);
  EXPECT_EQ(std::get<1>(v2[2]), 100);

  auto v3 = coro::blockingWait(
    client.load(
      std::vector<std::tuple<Ref<std::string>, Ref<int>>>{
        std::make_tuple(str1, int5),
        std::make_tuple(str2, int6),
        std::make_tuple(str3, int7)
      }
    )
  );
  ASSERT_EQ(v3.size(), 3);
  EXPECT_EQ(std::get<0>(v3[0]), "str1");
  EXPECT_EQ(std::get<0>(v3[1]), "str2");
  EXPECT_EQ(std::get<0>(v3[2]), "str3");
  EXPECT_EQ(std::get<1>(v3[0]), 500);
  EXPECT_EQ(std::get<1>(v3[1]), 600);
  EXPECT_EQ(std::get<1>(v3[2]), 700);

  auto o1 = coro::blockingWait(
    client.exec(
      s_test5,
      std::make_tuple(str1, int1),
      std::vector<std::tuple<Ref<int>, Ref<std::string>>>{
        std::make_tuple(int2, str2),
        std::make_tuple(int3, str3),
        std::make_tuple(int4, str4)
      }
    )
  );
  ASSERT_EQ(o1.size(), 3);
  EXPECT_TRUE(o1[0].fromFallback());
  EXPECT_TRUE(o1[1].fromFallback());
  EXPECT_TRUE(o1[2].fromFallback());

  EXPECT_EQ(coro::blockingWait(client.load(o1[0])), "str1-100-200-str2");
  EXPECT_EQ(coro::blockingWait(client.load(o1[1])), "str1-100-300-str3");
  EXPECT_EQ(coro::blockingWait(client.load(o1[2])), "str1-100-400-str4");

  auto o2 = coro::blockingWait(
    client.exec(
      s_test5,
      std::make_tuple(str17, int8),
      std::vector<std::tuple<Ref<int>, Ref<std::string>>>{
        std::make_tuple(int5, str12),
        std::make_tuple(int6, str13),
        std::make_tuple(int7, str14)
      }
    )
  );
  ASSERT_EQ(o2.size(), 3);
  EXPECT_FALSE(o2[0].fromFallback());
  EXPECT_FALSE(o2[1].fromFallback());
  EXPECT_FALSE(o2[2].fromFallback());

  EXPECT_EQ(coro::blockingWait(client.load(o2[0])), "str17-800-500-str12");
  EXPECT_EQ(coro::blockingWait(client.load(o2[1])), "str17-800-600-str13");
  EXPECT_EQ(coro::blockingWait(client.load(o2[2])), "str17-800-700-str14");

  auto o3 = coro::blockingWait(
    client.exec(
      s_test5,
      std::make_tuple(str17, int1),
      std::vector<std::tuple<Ref<int>, Ref<std::string>>>{
        std::make_tuple(int5, str12),
        std::make_tuple(int6, str13),
        std::make_tuple(int7, str14)
      }
    )
  );
  ASSERT_EQ(o3.size(), 3);
  EXPECT_TRUE(o3[0].fromFallback());
  EXPECT_TRUE(o3[1].fromFallback());
  EXPECT_TRUE(o3[2].fromFallback());

  EXPECT_EQ(coro::blockingWait(client.load(o3[0])), "str17-100-500-str12");
  EXPECT_EQ(coro::blockingWait(client.load(o3[1])), "str17-100-600-str13");
  EXPECT_EQ(coro::blockingWait(client.load(o3[2])), "str17-100-700-str14");

  auto o4 = coro::blockingWait(
    client.exec(
      s_test5,
      std::make_tuple(str17, int8),
      std::vector<std::tuple<Ref<int>, Ref<std::string>>>{
        std::make_tuple(int5, str12),
        std::make_tuple(int6, str3),
        std::make_tuple(int7, str14)
      }
    )
  );
  ASSERT_EQ(o4.size(), 3);
  EXPECT_TRUE(o4[0].fromFallback());
  EXPECT_TRUE(o4[1].fromFallback());
  EXPECT_TRUE(o4[2].fromFallback());

  EXPECT_EQ(coro::blockingWait(client.load(o4[0])), "str17-800-500-str12");
  EXPECT_EQ(coro::blockingWait(client.load(o4[1])), "str17-800-600-str3");
  EXPECT_EQ(coro::blockingWait(client.load(o4[2])), "str17-800-700-str14");

  auto o5 = coro::blockingWait(
    client.exec(
      s_test7, std::make_tuple(),
      std::vector<std::tuple<Optional<Ref<int>>>>{
        std::make_tuple(int7),
        std::make_tuple(int8),
        std::make_tuple(int2)
      }
    )
  );
  ASSERT_EQ(o5.size(), 3);
  ASSERT_TRUE(o5[0].has_value());
  ASSERT_TRUE(o5[1].has_value());
  ASSERT_TRUE(o5[2].has_value());

  EXPECT_TRUE(o5[0]->fromFallback());
  EXPECT_TRUE(o5[1]->fromFallback());
  EXPECT_TRUE(o5[2]->fromFallback());

  EXPECT_EQ(coro::blockingWait(client.load(*o5[0])), "700");
  EXPECT_EQ(coro::blockingWait(client.load(*o5[1])), "800");
  EXPECT_EQ(coro::blockingWait(client.load(*o5[2])), "200");

  auto o6 = coro::blockingWait(
    client.exec(
      s_test7, std::make_tuple(),
      std::vector<std::tuple<Optional<Ref<int>>>>{
        std::make_tuple(int7),
        std::make_tuple(int8),
        std::make_tuple(int6)
      }
    )
  );
  ASSERT_EQ(o6.size(), 3);
  ASSERT_TRUE(o6[0].has_value());
  ASSERT_TRUE(o6[1].has_value());
  ASSERT_TRUE(o6[2].has_value());

  EXPECT_FALSE(o6[0]->fromFallback());
  EXPECT_FALSE(o6[1]->fromFallback());
  EXPECT_FALSE(o6[2]->fromFallback());

  EXPECT_EQ(coro::blockingWait(client.load(*o6[0])), "700");
  EXPECT_EQ(coro::blockingWait(client.load(*o6[1])), "800");
  EXPECT_EQ(coro::blockingWait(client.load(*o6[2])), "600");

  auto o7 = coro::blockingWait(
    client.exec(
      s_test8, std::make_tuple(),
      std::vector<std::tuple<std::vector<Ref<int>>>>{
        std::make_tuple(std::vector<Ref<int>>{ int5, int6 }),
        std::make_tuple(std::vector<Ref<int>>{ int7, int8 }),
        std::make_tuple(std::vector<Ref<int>>{ int9, int10 })
      }
    )
  );
  ASSERT_EQ(o7.size(), 3);
  ASSERT_EQ(o7[0].size(), 2);
  ASSERT_EQ(o7[1].size(), 2);
  ASSERT_EQ(o7[2].size(), 2);

  EXPECT_FALSE(o7[0][0].fromFallback());
  EXPECT_FALSE(o7[0][1].fromFallback());
  EXPECT_FALSE(o7[1][0].fromFallback());
  EXPECT_FALSE(o7[1][1].fromFallback());
  EXPECT_FALSE(o7[2][0].fromFallback());
  EXPECT_FALSE(o7[2][1].fromFallback());

  EXPECT_EQ(coro::blockingWait(client.load(o7[0][0])), "500");
  EXPECT_EQ(coro::blockingWait(client.load(o7[0][1])), "600");
  EXPECT_EQ(coro::blockingWait(client.load(o7[1][0])), "700");
  EXPECT_EQ(coro::blockingWait(client.load(o7[1][1])), "800");
  EXPECT_EQ(coro::blockingWait(client.load(o7[2][0])), "900");
  EXPECT_EQ(coro::blockingWait(client.load(o7[2][1])), "1000");

  auto o8 = coro::blockingWait(
    client.exec(
      s_test8, std::make_tuple(),
      std::vector<std::tuple<std::vector<Ref<int>>>>{
        std::make_tuple(std::vector<Ref<int>>{ int5, int6 }),
        std::make_tuple(std::vector<Ref<int>>{ int7, int8 }),
        std::make_tuple(std::vector<Ref<int>>{ int1, int10 })
      }
    )
  );
  ASSERT_EQ(o8.size(), 3);
  ASSERT_EQ(o8[0].size(), 2);
  ASSERT_EQ(o8[1].size(), 2);
  ASSERT_EQ(o8[2].size(), 2);

  EXPECT_TRUE(o8[0][0].fromFallback());
  EXPECT_TRUE(o8[0][1].fromFallback());
  EXPECT_TRUE(o8[1][0].fromFallback());
  EXPECT_TRUE(o8[1][1].fromFallback());
  EXPECT_TRUE(o8[2][0].fromFallback());
  EXPECT_TRUE(o8[2][1].fromFallback());

  EXPECT_EQ(coro::blockingWait(client.load(o8[0][0])), "500");
  EXPECT_EQ(coro::blockingWait(client.load(o8[0][1])), "600");
  EXPECT_EQ(coro::blockingWait(client.load(o8[1][0])), "700");
  EXPECT_EQ(coro::blockingWait(client.load(o8[1][1])), "800");
  EXPECT_EQ(coro::blockingWait(client.load(o8[2][0])), "100");
  EXPECT_EQ(coro::blockingWait(client.load(o8[2][1])), "1000");

  client.forceFallback();

  auto o9 = coro::blockingWait(
    client.exec(
      s_test5,
      std::make_tuple(str17, int7),
      std::vector<std::tuple<Ref<int>, Ref<std::string>>>{
        std::make_tuple(int8, str12),
        std::make_tuple(int9, str13),
        std::make_tuple(int10, str14)
      }
    )
  );
  ASSERT_EQ(o9.size(), 3);
  EXPECT_TRUE(o9[0].fromFallback());
  EXPECT_TRUE(o9[1].fromFallback());
  EXPECT_TRUE(o9[2].fromFallback());

  EXPECT_EQ(coro::blockingWait(client.load(o9[0])), "str17-700-800-str12");
  EXPECT_EQ(coro::blockingWait(client.load(o9[1])), "str17-700-900-str13");
  EXPECT_EQ(coro::blockingWait(client.load(o9[2])), "str17-700-1000-str14");
}

}

int main(int argc, char** argv) {
  if (argc > 1 && !strcmp(argv[1], HPHP::extern_worker::s_option)) {
    return HPHP::extern_worker::main(argc, argv);
  }
  ::testing::InitGoogleTest(&argc, argv);
  folly::init(&argc, &argv);
  return RUN_ALL_TESTS();
}
