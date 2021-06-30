/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>

#include <folly/ScopeGuard.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/executors/ManualExecutor.h>
#include <folly/experimental/TestUtil.h>
#include <folly/portability/GTest.h>
#include <folly/portability/GMock.h>

#include "hphp/runtime/ext/facts/exception.h"
#include "hphp/runtime/ext/facts/file-facts.h"
#include "hphp/runtime/ext/facts/symbol-map.h"

using ::testing::AllOf;
using ::testing::Contains;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::IsEmpty;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAre;

namespace HPHP {
namespace Facts {

template<typename S, SymKind k>
void PrintTo(const Symbol<S, k>& symbol, ::std::ostream* os) {
  *os << symbol.slice();
}

template<typename S, SymKind k>
bool operator==(const Symbol<S, k>& symbol, const char* str) {
  return symbol.slice() == str;
}

template<typename S, SymKind k, typename T, typename U>
void PrintTo(const std::tuple<Symbol<S, k>, Path<S>, T, U>& typeInfo, ::std::ostream* os) {
  *os << std::get<0>(typeInfo).slice();
}

template<typename S, SymKind k, typename T, typename U>
bool operator==(const std::tuple<Symbol<S, k>, Path<S>, T, U>& typeInfo, const char* str) {
  return std::get<0>(typeInfo) == str;
}

namespace {

constexpr int kTypeFlagAbstractBit = 1;
constexpr int kTypeFlagFinalBit = 2;

auto constexpr kSHA = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

/**
 * RAII wrapper which ensures we finish draining the given
 * ManualExecutor before ending each test.
 */
struct SymbolMapWrapper {
  SymbolMapWrapper(
      std::unique_ptr<SymbolMap<std::string>> m,
      std::shared_ptr<folly::ManualExecutor> exec)
      : m_exec{exec}, m_map{std::move(m)} {
    if (m_exec) {
      m_map->m_exec = m_exec;
    }
  }

  ~SymbolMapWrapper() {
    if (m_exec) {
      m_exec->drain();
    }
  }

  SymbolMapWrapper(const SymbolMapWrapper&) = delete;
  SymbolMapWrapper& operator=(const SymbolMapWrapper&) = delete;
  SymbolMapWrapper(SymbolMapWrapper&& old) noexcept
      : m_exec{std::move(old.m_exec)}, m_map{std::move(old.m_map)} {
  }
  SymbolMapWrapper& operator=(SymbolMapWrapper&& old) noexcept = delete;

  std::shared_ptr<folly::ManualExecutor> m_exec;
  std::unique_ptr<SymbolMap<std::string>> m_map;
};

/**
 * If a map has a manual executor assigned to it, use this to
 * explicitly tell the executor to run.
 */
void waitForDB(
    SymbolMap<std::string>& m, std::shared_ptr<folly::ManualExecutor>& exec) {
  ASSERT_EQ(m.m_exec.get(), static_cast<folly::Executor*>(exec.get()));
  exec->drain();
  m.waitForDBUpdate();
}

} // namespace

class SymbolMapTest : public ::testing::Test {
protected:
  void SetUp() override {
    m_tmpdir = std::make_unique<folly::test::TemporaryDirectory>(
        folly::test::TemporaryDirectory{"autoload"});

    if (!m_exec) {
      m_exec = std::make_shared<folly::ManualExecutor>();
    }
    if (!m_exec2) {
      m_exec2 = std::make_shared<folly::ManualExecutor>();
    }
  }

  void TearDown() override {
    m_wrappers.clear();
  }

  SymbolMap<std::string>& make(
      std::string root, std::shared_ptr<folly::ManualExecutor> exec = nullptr) {
    auto dbPath = m_tmpdir->path() /
                  folly::to<std::string>(
                      "autoload_", std::hash<std::string>{}(root), "_db.sql3");
    m_wrappers.push_back(SymbolMapWrapper{
        std::make_unique<SymbolMap<std::string>>(
            std::move(root),
            DBData::readWrite(
                std::move(dbPath), static_cast<::gid_t>(-1), 0644)),
        std::move(exec)});
    return *m_wrappers.back().m_map;
  }

  std::unique_ptr<folly::test::TemporaryDirectory> m_tmpdir;
  std::shared_ptr<folly::ManualExecutor> m_exec;
  std::shared_ptr<folly::ManualExecutor> m_exec2;
  std::vector<SymbolMapWrapper> m_wrappers;
};

TEST_F(SymbolMapTest, addPaths) {
  auto& m = make("/var/www");

  FileFacts ff{
      .m_types =
          {{.m_name = "SomeClass",
            .m_kind = TypeKind::Class,
            .m_baseTypes = {"BaseClass"}},
           {.m_name = "BaseClass", .m_kind = TypeKind::Class},
           {.m_name = "SomeTypeAlias", .m_kind = TypeKind::TypeAlias}},
      .m_functions = {"some_fn"},
      .m_constants = {"SOME_CONSTANT"},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  // Define symbols in path
  folly::fs::path path1 = {"some/path1.php"};
  m.update("", "1:2:3", {path1}, {}, {ff});
  EXPECT_EQ(m.getClock(), "1:2:3");

  // Symbol -> file
  EXPECT_EQ(m.getTypeFile("SomeClass"), path1.native());
  EXPECT_EQ(m.getFunctionFile("some_fn"), path1.native());
  EXPECT_EQ(m.getConstantFile("SOME_CONSTANT"), path1.native());
  EXPECT_EQ(m.getTypeAliasFile("SomeTypeAlias"), path1.native());

  // Types and type aliases are still different
  EXPECT_EQ(m.getTypeAliasFile("SomeClass"), nullptr);
  EXPECT_EQ(m.getTypeFile("SomeTypeAlias"), nullptr);

  // file -> symbols
  EXPECT_THAT(
    m.getFileTypes(path1), UnorderedElementsAre("BaseClass", "SomeClass"));
  EXPECT_EQ(m.getFileFunctions(path1).at(0), "some_fn");
  EXPECT_EQ(m.getFileConstants(path1).at(0), "SOME_CONSTANT");
  EXPECT_EQ(m.getFileTypeAliases(path1).at(0), "SomeTypeAlias");

  // Check for case insensitivity (constants are case-sensitive, nothing else
  // is)
  EXPECT_EQ(m.getTypeFile("someclass"), path1.native());
  EXPECT_EQ(m.getFunctionFile("SOME_FN"), path1.native());
  EXPECT_EQ(m.getConstantFile("Some_Constant"), nullptr);
  EXPECT_EQ(m.getTypeAliasFile("sometypealias"), path1.native());

  // Check for undefined symbols
  EXPECT_EQ(m.getTypeFile("UndefinedClass"), nullptr);
  EXPECT_EQ(m.getFunctionFile("undefined_fn"), nullptr);
  EXPECT_EQ(m.getConstantFile("UNDEFINED_CONSTANT"), nullptr);
  EXPECT_EQ(m.getTypeAliasFile("UndefinedTypeAlias"), nullptr);

  // Check inheritance
  EXPECT_EQ(
      m.getBaseTypes("SomeClass", DeriveKind::Extends).at(0), "BaseClass");
  EXPECT_TRUE(m.getDerivedTypes("SomeClass", DeriveKind::Extends).empty());
  EXPECT_TRUE(m.getBaseTypes("BaseClass", DeriveKind::Extends).empty());
  EXPECT_EQ(
      m.getDerivedTypes("BaseClass", DeriveKind::Extends).at(0), "SomeClass");

  // Define duplicate symbols in path2
  folly::fs::path path2 = {"some/path2.php"};
  ASSERT_NE(path1, path2);

  m.update("1:2:3", "1:2:4", {path2}, {}, {ff});
  EXPECT_EQ(m.getClock(), "1:2:4");
  EXPECT_EQ(m.getTypeFile("SomeClass"), nullptr);
  EXPECT_EQ(m.getTypeFile("BaseClass"), nullptr);
  EXPECT_EQ(m.getFunctionFile("some_fn"), nullptr);
  EXPECT_EQ(m.getConstantFile("SOME_CONSTANT"), nullptr);
  EXPECT_EQ(m.getTypeAliasFile("SomeTypeAlias"), nullptr);
  EXPECT_TRUE(m.getBaseTypes("SomeClass", DeriveKind::Extends).empty());
  EXPECT_TRUE(m.getDerivedTypes("SomeClass", DeriveKind::Extends).empty());
  EXPECT_TRUE(m.getBaseTypes("BaseClass", DeriveKind::Extends).empty());
  EXPECT_TRUE(m.getDerivedTypes("BaseClass", DeriveKind::Extends).empty());

  // Remove path1, leaving the symbols defined in path2
  m.update("1:2:4", "1:2:5", {}, {path1}, {});
  EXPECT_EQ(m.getClock(), "1:2:5");
  EXPECT_EQ(m.getTypeFile("SomeClass"), path2.native());
  EXPECT_EQ(m.getFunctionFile("some_fn"), path2.native());
  EXPECT_EQ(m.getConstantFile("SOME_CONSTANT"), path2.native());
  EXPECT_EQ(m.getTypeAliasFile("SomeTypeAlias"), path2.native());

  EXPECT_THAT(
    m.getBaseTypes("SomeClass", DeriveKind::Extends),
    ElementsAre("BaseClass"));

  EXPECT_TRUE(m.getDerivedTypes("SomeClass", DeriveKind::Extends).empty());
  EXPECT_TRUE(m.getBaseTypes("BaseClass", DeriveKind::Extends).empty());

  EXPECT_THAT(
    m.getDerivedTypes("BaseClass", DeriveKind::Extends),
    ElementsAre("SomeClass"));
}

TEST_F(SymbolMapTest, duplicateSymbols) {
  auto& m = make("/var/www");

  FileFacts ff{
      .m_types =
          {{.m_name = "SomeClass", .m_kind = TypeKind::Class},
           {.m_name = "SomeTypeAlias", .m_kind = TypeKind::TypeAlias}},
      .m_functions = {"some_fn"},
      .m_constants = {"SOME_CONSTANT"},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  folly::fs::path path1 = {"some/path1.php"};
  folly::fs::path path2 = {"some/path2.php"};
  ASSERT_NE(path1, path2);
  m.update("", "1:2:3", {path1, path2}, {}, {ff, ff});

  EXPECT_EQ(m.getClock(), "1:2:3");
  EXPECT_EQ(m.getTypeFile("SomeClass"), nullptr);
  EXPECT_EQ(m.getFunctionFile("some_fn"), nullptr);
  EXPECT_EQ(m.getConstantFile("SOME_CONSTANT"), nullptr);
  EXPECT_EQ(m.getTypeAliasFile("SomeTypeAlias"), nullptr);
}

TEST_F(SymbolMapTest, DBFill) {
  auto& m1 = make("/var/www");
  auto& m2 = make("/var/www");

  FileFacts ff{
      .m_types =
          {{.m_name = "SomeClass",
            .m_kind = TypeKind::Class,
            .m_baseTypes = {"BaseClass"}},
           {.m_name = "BaseClass", .m_kind = TypeKind::Class},
           {.m_name = "SomeTypeAlias", .m_kind = TypeKind::TypeAlias}},
      .m_functions = {"some_fn"},
      .m_constants = {"SOME_CONSTANT"},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  folly::fs::path path = {"some/path.php"};

  m1.update("", "1:2:3", {path}, {}, {ff});

  // symbol -> file
  EXPECT_EQ(m1.getTypeFile("SomeClass"), path.native());
  EXPECT_EQ(m1.getFunctionFile("some_fn"), path.native());
  EXPECT_EQ(m1.getConstantFile("SOME_CONSTANT"), path.native());
  EXPECT_EQ(m1.getTypeAliasFile("SomeTypeAlias"), path.native());

  // file -> symbol
  EXPECT_THAT(
      m1.getFileTypes(path),
      UnorderedElementsAre("BaseClass", "SomeClass"));
  EXPECT_EQ(m1.getFileFunctions(path).at(0), "some_fn");
  EXPECT_EQ(m1.getFileConstants(path).at(0), "SOME_CONSTANT");
  EXPECT_EQ(m1.getFileTypeAliases(path).at(0), "SomeTypeAlias");

  // inheritance
  EXPECT_THAT(
      m1.getBaseTypes("SomeClass", DeriveKind::Extends),
      ElementsAre("BaseClass"));
  EXPECT_TRUE(m1.getDerivedTypes("SomeClass", DeriveKind::Extends).empty());
  EXPECT_TRUE(m1.getBaseTypes("BaseClass", DeriveKind::Extends).empty());
  EXPECT_EQ(
      m1.getDerivedTypes("BaseClass", DeriveKind::Extends).at(0).slice(),
      "SomeClass");

  m1.waitForDBUpdate();

  m2.update("1:2:3", "2:3:4", {}, {}, {});

  // file -> symbol
  EXPECT_THAT(
    m1.getFileTypes(path),
    UnorderedElementsAre("BaseClass", "SomeClass"));
  EXPECT_EQ(m2.getFileFunctions(path).at(0).slice(), "some_fn");
  EXPECT_EQ(m2.getFileConstants(path).at(0).slice(), "SOME_CONSTANT");
  EXPECT_EQ(m2.getFileTypeAliases(path).at(0).slice(), "SomeTypeAlias");

  // symbol -> file
  EXPECT_EQ(m2.getTypeFile("SomeClass"), path.native());
  EXPECT_EQ(m2.getTypeFile("BaseClass"), path.native());
  EXPECT_EQ(m2.getFunctionFile("some_fn"), path.native());
  EXPECT_EQ(m2.getConstantFile("SOME_CONSTANT"), path.native());
  EXPECT_EQ(m2.getTypeAliasFile("SomeTypeAlias"), path.native());

  // inheritance
  EXPECT_THAT(
      m2.getBaseTypes("SomeClass", DeriveKind::Extends),
      ElementsAre("BaseClass"));
  EXPECT_TRUE(m2.getDerivedTypes("SomeClass", DeriveKind::Extends).empty());
  EXPECT_TRUE(m2.getBaseTypes("BaseClass", DeriveKind::Extends).empty());
  EXPECT_THAT(
      m2.getDerivedTypes("BaseClass", DeriveKind::Extends),
      ElementsAre("SomeClass"));
}

TEST_F(SymbolMapTest, ChangeSymbolCase) {
  auto& m = make("/var/www");
  FileFacts ff{
      .m_types = {{.m_name = "HTTPDNSURL", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  folly::fs::path path = {"some/path.php"};
  m.update("", "1:2:3", {path}, {}, {ff});

  EXPECT_EQ(m.getTypeName("httpdnsurl")->slice(), "HTTPDNSURL");
  EXPECT_EQ(m.getFileTypes(path).at(0).slice(), "HTTPDNSURL");
  EXPECT_NE(m.getFileTypes(path).at(0).slice(), "HttpDnsUrl");

  // Classes are case-insensitive, but changing the case of a class
  // should change the canonical representation
  ff.m_types.at(0).m_name = "HttpDnsUrl";
  m.update("1:2:3", "1:2:4", {path}, {}, {ff});
  auto types = m.getFileTypes(path);
  EXPECT_EQ(m.getTypeName("httpdnsurl")->slice(), "HttpDnsUrl");
  EXPECT_EQ(types.at(0).slice(), "HttpDnsUrl");
  EXPECT_NE(types.at(0).slice(), "HTTPDNSURL");

  m.waitForDBUpdate();

  auto& m2 = make("/var/www");
  EXPECT_EQ(m2.dbClock(), "1:2:4");
  m2.update("1:2:4", "1:2:4", {}, {}, {});
  EXPECT_EQ(m2.getClock(), "1:2:4");
  auto dbTypes = m2.getFileTypes(path);
  EXPECT_EQ(m.getTypeName("httpdnsurl")->slice(), "HttpDnsUrl");
  EXPECT_EQ(types.at(0).slice(), "HttpDnsUrl");
}

TEST_F(SymbolMapTest, ChangeBaseClassSymbolCase) {
  auto& m = make("/var/www");
  FileFacts ff1{
      .m_types =
          {{.m_name = "SomeClass",
            .m_kind = TypeKind::Class,
            .m_baseTypes = {"baseclass"}}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  FileFacts ff2{
      .m_types = {{.m_name = "baseclass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"};

  folly::fs::path path1 = {"some/path1.php"};
  folly::fs::path path2 = {"some/path2.php"};
  m.update("", "1:2:3", {path1, path2}, {}, {ff1, ff2});
  EXPECT_THAT(
      m.getBaseTypes("SomeClass", DeriveKind::Extends),
      ElementsAre("baseclass"));
  EXPECT_THAT(
      m.getDerivedTypes("baseclass", DeriveKind::Extends),
      ElementsAre("SomeClass"));

  ff1.m_types.at(0).m_baseTypes.at(0) = "BaseClass";
  ff2.m_types.at(0).m_name = "BaseClass";
  m.update("1:2:3", "1:2:4", {path1, path2}, {}, {ff1, ff2});
  EXPECT_THAT(
      m.getBaseTypes("SomeClass", DeriveKind::Extends),
      ElementsAre("BaseClass"));
  EXPECT_THAT(
      m.getDerivedTypes("BaseClass", DeriveKind::Extends),
      ElementsAre("SomeClass"));
}

/**
 * Types are case insensitive, but asking for the path of a lowercased type
 * can cause us to cache the casing you gave us rather than the correct case.
 * This test ensures we always cache the correctly-cased type.
 */
TEST_F(SymbolMapTest, MaintainCorrectCase) {
  auto& m1 = make("/var/www");
  FileFacts ff{
      .m_types =
          {{.m_name = "CamelCasedType", .m_kind = TypeKind::Class},
           {.m_name = "CamelCasedTypeAlias", .m_kind = TypeKind::TypeAlias}},
      .m_functions = {{"CamelCasedFunction"}},
      .m_constants = {{"CamelCasedConstant"}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  folly::fs::path path = {"some/path.php"};
  m1.update("", "1:2:3", {path}, {}, {ff});
  m1.waitForDBUpdate();

  auto& m2 = make("/var/www");

  // Call getTypeFile before calling getFileTypes in order to prime the cache
  // with the wrong name
  EXPECT_EQ(m2.getTypeFile("camelcasedtype"), path.native());
  EXPECT_EQ(m2.getFileTypes(path).at(0).slice(), "CamelCasedType");

  EXPECT_EQ(m2.getFunctionFile("camelcasedfunction"), path.native());
  EXPECT_EQ(m2.getFileFunctions(path).at(0).slice(), "CamelCasedFunction");

  EXPECT_EQ(m2.getConstantFile("camelcasedconstant"), nullptr);
  EXPECT_EQ(m2.getFileConstants(path).at(0).slice(), "CamelCasedConstant");

  EXPECT_EQ(m2.getTypeAliasFile("camelcasedtypealias"), path.native());
  EXPECT_EQ(m2.getFileTypeAliases(path).at(0).slice(), "CamelCasedTypeAlias");
}

TEST_F(SymbolMapTest, DBUpdateWithDuplicateDeclaration) {
  auto& m1 = make("/var/www");
  auto& m2 = make("/var/www");

  FileFacts ff{
      .m_types = {{.m_name = "SomeClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  folly::fs::path path1 = {"some/path.php"};
  folly::fs::path path2 = {"some/path2.php"};
  ASSERT_NE(path1, path2);

  m1.update("", "1:2:3", {path1, path2}, {}, {ff, ff});
  EXPECT_EQ(m1.getFileTypes(path1).at(0).slice(), "SomeClass");
  EXPECT_EQ(m1.getFileTypes(path2).at(0).slice(), "SomeClass");
  EXPECT_EQ(m1.getTypeFile("SomeClass"), nullptr);
  EXPECT_EQ(m1.getAllPaths().size(), 2);

  m1.waitForDBUpdate();
  m2.update("1:2:3", "1:2:3", {}, {}, {});

  EXPECT_EQ(m2.getFileTypes(path1).at(0).slice(), "SomeClass");
  EXPECT_EQ(m2.getFileTypes(path2).at(0).slice(), "SomeClass");
  EXPECT_EQ(m2.getTypeFile("SomeClass"), nullptr);

  EXPECT_EQ(m2.getFileTypes(path1).at(0).slice(), "SomeClass");
  EXPECT_EQ(m2.getFileTypes(path2).at(0).slice(), "SomeClass");
  EXPECT_EQ(m2.getTypeFile("SomeClass"), nullptr);
  EXPECT_EQ(m2.getAllPaths().size(), 2);
}

TEST_F(SymbolMapTest, getAllSymbols) {
  auto& m = make("/var/www");

  FileFacts ff{
      .m_types =
          {{.m_name = "SomeClass", .m_kind = TypeKind::Class},
           {.m_name = "SomeTypeAlias", .m_kind = TypeKind::TypeAlias}},
      .m_functions = {"SomeFunction"},
      .m_constants = {"SOME_CONSTANT"},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  folly::fs::path p = {"some/path.php"};

  m.update("", "1", {p}, {}, {ff});

  auto typePaths = m.getAllTypes();
  EXPECT_EQ(typePaths.size(), 1);
  EXPECT_EQ(typePaths.at(0).first.slice(), "SomeClass");
  EXPECT_EQ(typePaths.at(0).second.slice(), p.native());

  auto functionPaths = m.getAllFunctions();
  EXPECT_EQ(functionPaths.size(), 1);
  EXPECT_EQ(functionPaths.at(0).first.slice(), "SomeFunction");
  EXPECT_EQ(functionPaths.at(0).second.slice(), p.native());

  auto constantPaths = m.getAllConstants();
  EXPECT_EQ(constantPaths.size(), 1);
  EXPECT_EQ(constantPaths.at(0).first.slice(), "SOME_CONSTANT");
  EXPECT_EQ(constantPaths.at(0).second.slice(), p.native());

  auto typeAliasPaths = m.getAllTypeAliases();
  EXPECT_EQ(typeAliasPaths.size(), 1);
  EXPECT_EQ(typeAliasPaths.at(0).first.slice(), "SomeTypeAlias");
  EXPECT_EQ(typeAliasPaths.at(0).second.slice(), p.native());
}

TEST_F(SymbolMapTest, CopiedFile) {
  auto& m1 = make("/var/www");

  folly::fs::path path1 = {"some/path.php"};
  folly::fs::path path2 = {"some/path2.php"};
  ASSERT_NE(path1, path2);

  FileFacts ff{
      .m_types =
          {{.m_name = "SomeClass", .m_kind = TypeKind::Class},
           {.m_name = "OtherClass", .m_kind = TypeKind::Class},
           {.m_name = "SomeTypeAlias", .m_kind = TypeKind::TypeAlias}},
      .m_functions = {"SomeFunction"},
      .m_constants = {"SomeConstant"},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  m1.update("", "1:2:3", {path1}, {}, {ff});
  m1.waitForDBUpdate();

  auto& m2 = make("/var/www");
  m2.update("1:2:3", "1:2:4", {path2}, {}, {ff});

  EXPECT_EQ(m2.getTypeFile("SomeClass"), nullptr);
  EXPECT_EQ(m2.getTypeFile("OtherClass"), nullptr);
  EXPECT_EQ(m2.getFunctionFile("SomeFunction"), nullptr);
  EXPECT_EQ(m2.getConstantFile("SomeConstant"), nullptr);
  EXPECT_EQ(m2.getTypeAliasFile("SomeTypeAlias"), nullptr);

  EXPECT_THAT(
      m2.getFileTypes(path1), UnorderedElementsAre("SomeClass", "OtherClass"));
  EXPECT_THAT(
      m2.getFileTypes(path2), UnorderedElementsAre("SomeClass", "OtherClass"));
  EXPECT_THAT(m2.getFileFunctions(path1), ElementsAre("SomeFunction"));
  EXPECT_THAT(m2.getFileFunctions(path2), ElementsAre("SomeFunction"));
  EXPECT_THAT(m2.getFileConstants(path1), ElementsAre("SomeConstant"));
  EXPECT_THAT(m2.getFileConstants(path2), ElementsAre("SomeConstant"));
  EXPECT_THAT(m2.getFileTypeAliases(path1), ElementsAre("SomeTypeAlias"));
  EXPECT_THAT(m2.getFileTypeAliases(path2), ElementsAre("SomeTypeAlias"));
}

TEST_F(SymbolMapTest, ConcurrentCallsToGetTypeFile) {
  auto& m1 = make("/var/www");

  folly::fs::path path1 = {"some/path.php"};
  folly::fs::path path2 = {"some/path2.php"};
  ASSERT_NE(path1, path2);

  FileFacts ff{
      .m_types =
          {{.m_name = "SomeClass", .m_kind = TypeKind::Class},
           {.m_name = "OtherClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  m1.update("", "1:2:3", {path1, path2}, {}, {ff, ff});
  m1.waitForDBUpdate();
  for (auto i = 0; i < 100; i++) {
    auto& m2 = make("/var/www");
    m2.update("1:2:3", "1:2:3", {}, {}, {});

    // getTypeFile and getFileTypes load from the DB and cache results
    // in memory. We need to make sure they can be called concurrently.
    std::vector<folly::Future<folly::Unit>> futures;
    SCOPE_EXIT {
      folly::collect(futures).wait();
    };
    for (auto j = 0; j < 2; j++) {
      futures.push_back(folly::via(folly::getCPUExecutor().get(), [&]() {
        EXPECT_EQ(m2.getTypeFile("SomeClass"), nullptr);
      }));
      futures.push_back(folly::via(folly::getCPUExecutor().get(), [&]() {
        EXPECT_EQ(m2.getTypeFile("OtherClass"), nullptr);
      }));
      futures.push_back(folly::via(folly::getCPUExecutor().get(), [&]() {
        EXPECT_THAT(
          m2.getFileTypes(path1),
          UnorderedElementsAre("SomeClass", "OtherClass"));
      }));
      futures.push_back(folly::via(folly::getCPUExecutor().get(), [&]() {
        EXPECT_THAT(
          m2.getFileTypes(path2),
          UnorderedElementsAre("SomeClass", "OtherClass"));
      }));
    }
  }
}

TEST_F(SymbolMapTest, DoesNotFillDeadPathFromDB) {
  auto& m1 = make("/var/www", m_exec);
  auto& m2 = make("/var/www", m_exec);
  auto& m3 = make("/var/www", m_exec);

  folly::fs::path path = {"some/path.php"};
  FileFacts ff{
      .m_types = {{.m_name = "SomeClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  m1.update("", "1:2:3", {path}, {}, {ff});
  waitForDB(m1, m_exec);
  EXPECT_EQ(m1.getAllPaths().begin()->slice(), path.native());

  m2.update("1:2:3", "1:2:4", {}, {path}, {});
  waitForDB(m2, m_exec);
  EXPECT_EQ(m2.getTypeFile("SomeClass"), nullptr);
  EXPECT_TRUE(m2.getAllPaths().empty());

  m3.update("1:2:4", "1:2:4", {}, {}, {});
  EXPECT_EQ(m3.getTypeFile("SomeClass"), nullptr);
  EXPECT_TRUE(m3.getAllPaths().empty());
}

TEST_F(SymbolMapTest, UpdateOnlyIfCorrectSince) {
  auto& m = make("/var/www");
  m.update("", "1:2:3", {}, {}, {});
  EXPECT_EQ(m.getClock(), "1:2:3");

  folly::fs::path path = {"some/path.php"};
  FileFacts ff{
      .m_types = {{.m_name = "SomeClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  // since token is "4:5:6" but needs to be "1:2:3". It's not, so
  // `m.update()` throws and we don't change any data in `m`.
  ASSERT_THROW(m.update("4:5:6", "4:5:7", {path}, {}, {ff}), UpdateExc);
  EXPECT_EQ(m.getTypeFile("SomeClass"), nullptr);
  EXPECT_TRUE(m.getAllPaths().empty());
}

TEST_F(SymbolMapTest, DelayedDBUpdateDoesNotMakeResultsIncorrect) {
  auto& m = make("/var/www");

  folly::fs::path path1 = {"some/path1.php"};
  folly::fs::path path2 = {"some/path2.php"};
  ASSERT_NE(path1, path2);

  FileFacts ff{
      .m_types =
          {{.m_name = "SomeClass",
            .m_kind = TypeKind::Class,
            .m_baseTypes = {"BaseClass"}},
           {.m_name = "BaseClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  m.update("", "1:2:3", {path1, path2}, {}, {ff, ff});
  EXPECT_EQ(m.getClock(), "1:2:3");

  // Duplicate declaration
  EXPECT_EQ(m.getKind("SomeClass"), TypeKind::Unknown);
  EXPECT_EQ(m.getKind("BaseClass"), TypeKind::Unknown);
  EXPECT_EQ(m.getTypeFile("SomeClass"), nullptr);
  EXPECT_EQ(m.getTypeFile("BaseClass"), nullptr);
  EXPECT_TRUE(m.getBaseTypes("SomeClass", DeriveKind::Extends).empty());
  EXPECT_TRUE(m.getDerivedTypes("BaseeClass", DeriveKind::Extends).empty());

  m.waitForDBUpdate();

  auto& m2 = make("/var/www", m_exec);

  m2.update("1:2:3", "1:2:4", {}, {path2}, {});
  // We do not wait for the DB update to complete.
  // m_exec.drive();
  // m2.waitForDBUpdate();
  EXPECT_EQ(m2.getKind("SomeClass"), TypeKind::Class);
  EXPECT_EQ(m2.getKind("BaseClass"), TypeKind::Class);
  EXPECT_EQ(m2.getTypeFile("SomeClass"), path1.native());
  EXPECT_EQ(m2.getTypeFile("BaseClass"), path1.native());
  EXPECT_THAT(
      m2.getBaseTypes("SomeClass", DeriveKind::Extends),
      ElementsAre("BaseClass"));
  EXPECT_THAT(
      m2.getDerivedTypes("BaseClass", DeriveKind::Extends),
      ElementsAre("SomeClass"));
  EXPECT_TRUE(m2.getFileTypes(path2).empty());

  m2.update("1:2:4", "1:2:5", {}, {path1}, {});
  EXPECT_EQ(m2.getKind("SomeClass"), TypeKind::Unknown);
  EXPECT_EQ(m2.getTypeFile("SomeClass"), nullptr);
  EXPECT_EQ(m2.getKind("BaseClass"), TypeKind::Unknown);
  EXPECT_EQ(m2.getTypeFile("BaseClass"), nullptr);
  EXPECT_TRUE(m2.getFileTypes(path1).empty());
  EXPECT_TRUE(m2.getBaseTypes("SomeClass", DeriveKind::Extends).empty());
  EXPECT_TRUE(m2.getDerivedTypes("BaseClass", DeriveKind::Extends).empty());
}

TEST_F(SymbolMapTest, GetKind) {
  auto& m1 = make("/var/www");

  folly::fs::path p = {"some/path1.php"};

  FileFacts ff{
      .m_types =
          {{.m_name = "C1", .m_kind = TypeKind::Class},
           {.m_name = "I1", .m_kind = TypeKind::Interface},
           {.m_name = "T1", .m_kind = TypeKind::Trait},
           {.m_name = "E1", .m_kind = TypeKind::Enum}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  m1.update("", "1:2:3", {p}, {}, {ff});

  EXPECT_EQ(m1.getKind("C1"), TypeKind::Class);
  EXPECT_EQ(m1.getKind("I1"), TypeKind::Interface);
  EXPECT_EQ(m1.getKind("T1"), TypeKind::Trait);
  EXPECT_EQ(m1.getKind("E1"), TypeKind::Enum);
  EXPECT_EQ(m1.getKind("Bogus"), TypeKind::Unknown);

  m1.waitForDBUpdate();

  auto& m2 = make("/var/www");
  m2.update("1:2:3", "1:2:3", {}, {}, {});

  EXPECT_EQ(m2.getKind("C1"), TypeKind::Class);
  EXPECT_EQ(m2.getKind("I1"), TypeKind::Interface);
  EXPECT_EQ(m2.getKind("T1"), TypeKind::Trait);
  EXPECT_EQ(m2.getKind("E1"), TypeKind::Enum);
  EXPECT_EQ(m2.getKind("Bogus"), TypeKind::Unknown);
}

TEST_F(SymbolMapTest, TypeIsAbstractOrFinal) {

  FileFacts ff{
      .m_types =
          {{.m_name = "Abstract",
            .m_kind = TypeKind::Class,
            .m_flags = kTypeFlagAbstractBit},
           {.m_name = "Final",
            .m_kind = TypeKind::Class,
            .m_flags = kTypeFlagFinalBit},
           {.m_name = "AbstractFinal",
            .m_kind = TypeKind::Trait,
            .m_flags = kTypeFlagAbstractBit | kTypeFlagFinalBit}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  folly::fs::path p = {"some/path1.php"};

  auto& m1 = make("/var/www");
  m1.update("", "1:2:3", {p}, {}, {ff});

  EXPECT_TRUE(m1.isTypeAbstract("Abstract"));
  EXPECT_FALSE(m1.isTypeFinal("Abstract"));

  EXPECT_FALSE(m1.isTypeAbstract("Final"));
  EXPECT_TRUE(m1.isTypeFinal("Final"));

  EXPECT_TRUE(m1.isTypeAbstract("AbstractFinal"));
  EXPECT_TRUE(m1.isTypeFinal("AbstractFinal"));

  m1.waitForDBUpdate();
  auto& m2 = make("/var/www");
  m2.update("1:2:3", "1:2:3", {}, {}, {});

  EXPECT_TRUE(m2.isTypeAbstract("Abstract"));
  EXPECT_FALSE(m2.isTypeFinal("Abstract"));

  EXPECT_FALSE(m2.isTypeAbstract("Final"));
  EXPECT_TRUE(m2.isTypeFinal("Final"));

  EXPECT_TRUE(m2.isTypeAbstract("AbstractFinal"));
  EXPECT_TRUE(m2.isTypeFinal("AbstractFinal"));
}

TEST_F(SymbolMapTest, OverwriteKind) {
  auto& m1 = make("/var/www");

  folly::fs::path p1 = {"some/path1.php"};
  folly::fs::path p2 = {"some/path2.php"};
  ASSERT_NE(p1.native(), p2.native());

  FileFacts ff1{
      .m_types = {{.m_name = "Foo", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  FileFacts ff2{
      .m_types = {{.m_name = "Foo", .m_kind = TypeKind::Interface}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab"};

  // Duplicate declaration
  m1.update("", "1:2:3", {p1, p2}, {}, {ff1, ff2});
  EXPECT_EQ(m1.getKind("Foo"), TypeKind::Unknown);

  // Remove the interface definition, keep the class definition
  m1.update("1:2:3", "1:2:4", {}, {p2}, {});
  EXPECT_EQ(m1.getKind("Foo"), TypeKind::Class);

  // Re-add the interface definition, remove the class definition
  m1.update("1:2:4", "1:2:5", {p2}, {p1}, {ff2});
  EXPECT_EQ(m1.getKind("Foo"), TypeKind::Interface);

  // Change the interface definition to a class definition
  m1.update("1:2:5", "1:2:6", {p2}, {}, {ff1});
  EXPECT_EQ(m1.getKind("Foo"), TypeKind::Class);
}

TEST_F(SymbolMapTest, DeriveKinds) {
  auto& m1 = make("/var/www1");

  folly::fs::path path = {"some/path.php"};
  FileFacts ff{
      .m_types =
          {TypeDetails{.m_name = "BaseClass", .m_kind = TypeKind::Class},
           TypeDetails{
               .m_name = "BaseInterface", .m_kind = TypeKind::Interface},
           TypeDetails{
               .m_name = "SomeTrait",
               .m_kind = TypeKind::Trait,
               .m_requireExtends = {"BaseClass"},
               .m_requireImplements = {"BaseInterface"}}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  m1.update("", "1:2:3", {path}, {}, {ff});
  EXPECT_EQ(m1.getClock(), "1:2:3");
  EXPECT_TRUE(m1.getBaseTypes("SomeTrait", DeriveKind::Extends).empty());
  EXPECT_THAT(
    m1.getBaseTypes("SomeTrait", DeriveKind::RequireExtends),
    ElementsAre("BaseClass"));
  EXPECT_THAT(
    m1.getBaseTypes("SomeTrait", DeriveKind::RequireImplements),
    ElementsAre("BaseInterface"));
  EXPECT_THAT(
    m1.getDerivedTypes("BaseClass", DeriveKind::RequireExtends),
    ElementsAre("SomeTrait"));
  EXPECT_THAT(
    m1.getDerivedTypes("BaseInterface", DeriveKind::RequireImplements),
    ElementsAre("SomeTrait"));

  auto& someTraitFacts = ff.m_types[2];
  ASSERT_EQ(someTraitFacts.m_name, "SomeTrait");
  someTraitFacts.m_requireExtends = {};
  someTraitFacts.m_requireImplements = {};
  ff.m_sha1hex[39] = 'b';
  m1.update("1:2:3", "1:2:4", {path}, {}, {ff});
}

TEST_F(SymbolMapTest, MultipleRoots) {
  auto& m1 = make("/var/www1");

  folly::fs::path path = {"some/path.php"};
  FileFacts ff{
      .m_types = {{.m_name = "SomeClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  m1.update("", "1:2:3", {path}, {}, {ff});
  EXPECT_EQ(m1.getClock(), "1:2:3");
  EXPECT_EQ(m1.getFileTypes(path).at(0).slice(), "SomeClass");
  EXPECT_EQ(m1.getTypeFile("SomeClass"), path.native());

  m1.waitForDBUpdate();

  auto& m2 = make("/var/www2");
  EXPECT_EQ(m2.getClock(), "");

  m2.update("", "1:2:3", {path}, {}, {ff});
  EXPECT_EQ(m2.getClock(), "1:2:3");
  EXPECT_EQ(m2.getFileTypes(path).at(0).slice(), "SomeClass");
  EXPECT_EQ(m2.getTypeFile("SomeClass"), path.native());
}

TEST_F(SymbolMapTest, InterleaveDBUpdates) {
  auto& m1 = make("/var/www", m_exec);
  auto& m2 = make("/var/www", m_exec);

  folly::fs::path path = {"some/path.php"};

  FileFacts ff{
      .m_functions = {"some_fn"},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  m1.update("", "1:2:3", {path}, {}, {ff});
  EXPECT_EQ(m1.getClock(), "1:2:3");
  EXPECT_EQ(m1.getFileFunctions(path).at(0).slice(), "some_fn");
  EXPECT_EQ(m1.getFunctionFile("some_fn"), path.native());

  m2.update("", "1:2:3", {path}, {}, {ff});
  EXPECT_EQ(m2.getClock(), "1:2:3");
  EXPECT_EQ(m2.getFileFunctions(path).at(0).slice(), "some_fn");
  EXPECT_EQ(m2.getFunctionFile("some_fn"), path.native());

  waitForDB(m1, m_exec);
  EXPECT_EQ(m1.getFunctionFile("some_fn"), path.native());
  EXPECT_EQ(m2.getFileFunctions(path).at(0).slice(), "some_fn");
  EXPECT_EQ(m2.getFunctionFile("some_fn"), path.native());

  waitForDB(m2, m_exec);
  EXPECT_EQ(m1.getFunctionFile("some_fn"), path.native());
  EXPECT_EQ(m2.getFunctionFile("some_fn"), path.native());
}

TEST_F(SymbolMapTest, RemoveBaseTypeFromDerivedType) {
  auto& m = make("/var/www", m_exec);

  FileFacts ff{
      .m_types =
          {{.m_name = "SomeClass",
            .m_kind = TypeKind::Class,
            .m_baseTypes = {"BaseClass"}},
           {.m_name = "BaseClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  folly::fs::path path = "some/path1.php";
  m.update("", "1:2:3", {path}, {}, {ff});

  EXPECT_EQ(
      m.getBaseTypes("SomeClass", DeriveKind::Extends).at(0).slice(),
      "BaseClass");
  EXPECT_EQ(
      m.getDerivedTypes("BaseClass", DeriveKind::Extends).at(0).slice(),
      "SomeClass");

  ff.m_types.at(0).m_baseTypes = {};
  m.update("1:2:3", "1:2:4", {path}, {}, {ff});

  EXPECT_TRUE(m.getBaseTypes("SomeClass", DeriveKind::Extends).empty());
  EXPECT_TRUE(m.getDerivedTypes("BaseClass", DeriveKind::Extends).empty());
}

TEST_F(SymbolMapTest, DuplicateDefineDerivedType) {
  auto& m = make("/var/www", m_exec);

  FileFacts ff1{
      .m_types =
          {{.m_name = "SomeClass",
            .m_kind = TypeKind::Class,
            .m_baseTypes = {"BaseClass"}}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  FileFacts ff2{
      .m_types = {{.m_name = "BaseClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"};

  folly::fs::path path1 = "some/path1.php";
  folly::fs::path path2 = "some/path2.php";
  folly::fs::path path3 = "some/path3.php";
  ASSERT_NE(path1, path2);
  ASSERT_NE(path1, path3);
  ASSERT_NE(path2, path3);

  // BaseClass has one definition, but SomeClass is duplicate-defined
  m.update("", "1:2:3", {path1, path2, path3}, {}, {ff1, ff1, ff2});

  EXPECT_EQ(m.getTypeFile("SomeClass"), nullptr);
  EXPECT_TRUE(m.getBaseTypes("SomeClass", DeriveKind::Extends).empty());
  EXPECT_TRUE(m.getDerivedTypes("BaseClass", DeriveKind::Extends).empty());

  // Delete path2, SomeClass now has one definition
  m.update("1:2:3", "1:2:4", {}, {path2}, {});

  EXPECT_EQ(m.getTypeFile("SomeClass"), path1.native());

  EXPECT_THAT(
    m.getBaseTypes("SomeClass", DeriveKind::Extends),
    ElementsAre("BaseClass"));

  EXPECT_THAT(
    m.getDerivedTypes("BaseClass", DeriveKind::Extends),
    ElementsAre("SomeClass"));
}

TEST_F(SymbolMapTest, DBUpdatesOutOfOrder) {
  auto& m1 = make("/var/www", m_exec);
  auto& m2 = make("/var/www", m_exec2);

  FileFacts ff{
      .m_types =
          {{.m_name = "SomeClass",
            .m_kind = TypeKind::Class,
            .m_baseTypes = {"BaseClass"}},
           {.m_name = "BaseClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  folly::fs::path path1 = {"some/path1.php"};
  folly::fs::path path2 = {"some/path2.php"};
  ASSERT_NE(path1, path2);

  // The repo looks like this at timestamps "1:2:3" and "1:2:4":
  // "1:2:3": {path1: ff}
  // <ff is moved from path1 to path2>
  // "1:2:4": {path2: ff}

  // m1 learns about the repo at timestamps "1:2:3" and "1:2:4", but
  // m2 only knows about the repo at "1:2:4". m2 then updates the DB
  // before m1 succeeds in updating the DB.

  m1.update("", "1:2:3", {path1}, {}, {ff});
  EXPECT_EQ(m1.getTypeFile("SomeClass"), path1.native());
  EXPECT_EQ(
      m1.getBaseTypes("SomeClass", DeriveKind::Extends).at(0).slice(),
      "BaseClass");
  EXPECT_TRUE(m1.getBaseTypes("BaseClass", DeriveKind::Extends).empty());
  EXPECT_EQ(
      m1.getDerivedTypes("BaseClass", DeriveKind::Extends).at(0).slice(),
      "SomeClass");
  EXPECT_TRUE(m1.getDerivedTypes("SomeClass", DeriveKind::Extends).empty());

  // m1 hasn't updated the DB yet, so m2 can't do an incremental update.
  ASSERT_THROW(m2.update("1:2:3", "1:2:4", {path2}, {path1}, {ff}), UpdateExc);

  // We do a non-incremental update, passing m2 the full repo
  m2.update("", "1:2:4", {path2}, {}, {ff});
  EXPECT_EQ(m2.getTypeFile("SomeClass"), path2.native());
  EXPECT_EQ(
      m2.getBaseTypes("SomeClass", DeriveKind::Extends).at(0).slice(),
      "BaseClass");
  EXPECT_TRUE(m2.getBaseTypes("BaseClass", DeriveKind::Extends).empty());
  EXPECT_EQ(
      m2.getDerivedTypes("BaseClass", DeriveKind::Extends).at(0).slice(),
      "SomeClass");
  EXPECT_TRUE(m2.getDerivedTypes("SomeClass", DeriveKind::Extends).empty());

  // m2 updates the DB first, bringing the DB to 1:2:4.
  waitForDB(m2, m_exec2);

  // m1 updates the DB next. m1's update should fail. m1 only knows
  // about 1:2:3, so it would be updating the DB with stale data.
  waitForDB(m1, m_exec);

  // Create a third map to observe the state of the DB.
  auto& m3 = make("/var/www", m_exec);
  // The DB clock should be 1:2:4, and the DB state should be
  // consistent with the state in m2, not m1.
  m3.update("1:2:4", "1:2:4", {}, {}, {});
  EXPECT_EQ(m3.getClock(), "1:2:4");
  EXPECT_EQ(m3.getTypeFile("SomeClass"), path2.native());
  EXPECT_EQ(
      m3.getBaseTypes("SomeClass", DeriveKind::Extends).at(0).slice(),
      "BaseClass");
  EXPECT_TRUE(m3.getBaseTypes("BaseClass", DeriveKind::Extends).empty());
  EXPECT_EQ(
      m3.getDerivedTypes("BaseClass", DeriveKind::Extends).at(0).slice(),
      "SomeClass");
  EXPECT_TRUE(m3.getDerivedTypes("SomeClass", DeriveKind::Extends).empty());
}

TEST_F(SymbolMapTest, ChangeAndMoveClassAttrs) {
  auto& m = make("/var/www");

  FileFacts ffWithAttr{
      .m_types =
          {{.m_name = "C1",
            .m_kind = TypeKind::Class,
            .m_attributes = {{.m_name = "A1"}}}},
      .m_sha1hex = kSHA};
  folly::fs::path p1{"p1.php"};

  m.update("", "1", {p1}, {}, {ffWithAttr});
  EXPECT_THAT(m.getAttributesOfType("C1"), ElementsAre("A1"));
  EXPECT_THAT(m.getTypesAndTypeAliasesWithAttribute("A1"), ElementsAre("C1"));

  FileFacts ffEmpty{.m_sha1hex = kSHA};
  FileFacts ffNoAttr{
      .m_types = {{.m_name = "C1", .m_kind = TypeKind::Class}},
      .m_sha1hex = kSHA};
  folly::fs::path p2{"p2.php"};

  m.update("1", "2", {p1, p2}, {}, {ffEmpty, ffNoAttr});
  EXPECT_THAT(m.getAttributesOfType("C1"), IsEmpty());
  EXPECT_THAT(m.getTypesAndTypeAliasesWithAttribute("A1"), IsEmpty());
}

TEST_F(SymbolMapTest, RemovePathFromExistingFile) {
  auto& m = make("/var/www");

  FileFacts ff{
      .m_types = {{.m_name = "SomeClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  FileFacts emptyFF{.m_sha1hex = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"};

  folly::fs::path path1 = {"some/path1.php"};

  m.update("", "1:2:3", {path1}, {}, {ff});
  EXPECT_EQ(m.getTypeFile("SomeClass"), path1.native());
  m.update("1:2:3", "1:2:4", {path1}, {}, {emptyFF});
  EXPECT_EQ(m.getTypeFile("SomeClass"), nullptr);
}

TEST_F(SymbolMapTest, MoveAndCopySymbol) {
  auto& m1 = make("/var/www", m_exec);
  auto& m2 = make("/var/www", m_exec);

  FileFacts ff{
      .m_types = {{.m_name = "SomeClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  FileFacts emptyFF{.m_sha1hex = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"};

  folly::fs::path path1 = {"some/path1.php"};
  folly::fs::path path2 = {"some/path2.php"};
  folly::fs::path path3 = {"some/path3.php"};

  // Initialize m2 as dependent on the DB
  m1.update("", "1:2:3", {path1}, {}, {ff});
  waitForDB(m1, m_exec);
  m2.update("1:2:3", "1:2:3", {}, {}, {});
  ASSERT_EQ(m2.getTypeFile("SomeClass"), path1.native());

  // Delete SomeClass from path1, simultaneously duplicate-define it in path2
  // and path3
  m2.update("1:2:3", "1:2:4", {path1, path2, path3}, {}, {emptyFF, ff, ff});

  // We should notice the duplicate-declaration. The DB still thinks that
  // SomeClass is defined in path1, but this call should not insert the stale
  // data about path1 from the DB.
  EXPECT_EQ(m2.getTypeFile("SomeClass"), nullptr);

  // waitForDB(m2, m_exec);

  // Delete path3.
  m2.update("1:2:4", "1:2:5", {}, {path3}, {});
  // SomeClass should only be defined in path2 now.
  EXPECT_EQ(m2.getTypeFile("SomeClass"), path2.native());
}

TEST_F(SymbolMapTest, TwoFilesDisagreeOnBaseTypes) {
  auto& m = make("/var/www", m_exec);

  FileFacts ffSomeClassDerivesBaseClass{
      .m_types =
          {{.m_name = "SomeClass",
            .m_kind = TypeKind::Class,
            .m_baseTypes = {"BaseClass"}}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  FileFacts ffBaseClass{
      .m_types = {{.m_name = "BaseClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"};

  folly::fs::path pSomeClass1 = "src/SomeClass1.php";
  folly::fs::path pSomeClass2 = "src/SomeClass2.php";
  ASSERT_NE(pSomeClass1, pSomeClass2);
  folly::fs::path pBaseClass = "src/BaseClass.php";
  ASSERT_NE(pBaseClass, pSomeClass1);
  ASSERT_NE(pBaseClass, pSomeClass2);

  // SomeClass is defined in two files, both of which agree that SomeClass
  // extends BaseClass
  m.update(
      "",
      "1:2:3",
      {pSomeClass1, pSomeClass2, pBaseClass},
      {},
      {ffSomeClassDerivesBaseClass, ffSomeClassDerivesBaseClass, ffBaseClass});

  // A collection of assertions that should hold thoughout the test
  auto expectAlways = [&]() {
    EXPECT_EQ(m.getTypeFile("BaseClass"), pBaseClass.native());
    EXPECT_TRUE(m.getBaseTypes("BaseClass", DeriveKind::Extends).empty());
    EXPECT_TRUE(m.getDerivedTypes("SomeClass", DeriveKind::Extends).empty());
  };

  // A collection of assertions that should hold whenever SomeClass is
  // defined in two or more files
  auto expectSomeClassIsDuplicateDefined = [&]() {
    expectAlways();
    EXPECT_EQ(m.getTypeFile("SomeClass"), nullptr);
    EXPECT_TRUE(m.getBaseTypes("SomeClass", DeriveKind::Extends).empty());
    EXPECT_TRUE(m.getDerivedTypes("BaseClass", DeriveKind::Extends).empty());
  };
  expectSomeClassIsDuplicateDefined();

  FileFacts ffSomeClassDerivesNobody{
      .m_types = {{.m_name = "SomeClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab"};

  // pSomeClass1 now believes that SomeClass doesn't extend BaseClass
  m.update("1:2:3", "1:2:4", {pSomeClass1}, {}, {ffSomeClassDerivesBaseClass});
  expectSomeClassIsDuplicateDefined();

  // Remove pSomeClass1 so pSomeClass2 is the source of truth for SomeClass
  m.update("1:2:4", "1:2:5", {}, {pSomeClass1}, {});
  expectAlways();

  EXPECT_EQ(m.getTypeFile("SomeClass"), pSomeClass2.native());
  EXPECT_THAT(
      m.getBaseTypes("SomeClass", DeriveKind::Extends),
      ElementsAre("BaseClass"));
  EXPECT_THAT(
      m.getDerivedTypes("BaseClass", DeriveKind::Extends),
      ElementsAre("SomeClass"));

  // Add pSomeClass1 back in
  m.update("1:2:5", "1:2:6", {pSomeClass1}, {}, {ffSomeClassDerivesNobody});
  expectSomeClassIsDuplicateDefined();

  // Now remove pSomeClass2, so that pSomeClass1 is the source of truth for
  // SomeClass
  m.update("1:2:6", "1:2:7", {}, {pSomeClass2}, {});
  expectAlways();

  EXPECT_EQ(m.getTypeFile("SomeClass"), pSomeClass1.native());
  EXPECT_TRUE(m.getBaseTypes("SomeClass", DeriveKind::Extends).empty());
  EXPECT_TRUE(m.getDerivedTypes("BaseClass", DeriveKind::Extends).empty());
}

TEST_F(SymbolMapTest, MemoryAndDBDisagreeOnFileHash) {
  auto& m1 = make("/var/www", m_exec);
  auto& m2 = make("/var/www", m_exec);

  FileFacts ff{
      .m_types = {{.m_name = "SomeClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  folly::fs::path path1 = {"some/path1.php"};

  m1.update("", "1:2:3", {path1}, {}, {ff});
  waitForDB(m1, m_exec);
  EXPECT_EQ(
      m1.getAllPathsWithHashes().at(Path<std::string>{path1}),
      SHA1{"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"});

  ff.m_sha1hex = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

  m2.update("1:2:3", "1:2:4", {path1}, {}, {ff});
  EXPECT_EQ(
      m2.getAllPathsWithHashes().at(Path<std::string>{path1}),
      SHA1{"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"});
}

TEST_F(SymbolMapTest, PartiallyFillDerivedTypeInfo) {
  auto& m1 = make("/var/www", m_exec);

  FileFacts ff1{
      .m_types =
          {{.m_name = "SomeClass",
            .m_kind = TypeKind::Class,
            .m_baseTypes = {"BaseClass"}}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  FileFacts ff2{
      .m_types =
          {{.m_name = "OtherClass",
            .m_kind = TypeKind::Class,
            .m_baseTypes = {"BaseClass"}}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab"};

  FileFacts ff3{
      .m_types = {{.m_name = "BaseClass", .m_kind = TypeKind::Interface}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac"};

  folly::fs::path p1 = "some/path1.php";
  folly::fs::path p2 = "some/path2.php";
  folly::fs::path p3 = "some/path3.php";

  m1.update("", "1:2:3", {p1, p2, p3}, {}, {ff1, ff2, ff3});
  waitForDB(m1, m_exec);

  auto& m2 = make("/var/www", m_exec);
  m2.update("1:2:3", "1:2:3", {}, {}, {});

  // Fetch the supertypes of SomeClass from the DB
  EXPECT_THAT(
      m2.getBaseTypes("SomeClass", DeriveKind::Extends),
      ElementsAre("BaseClass"));

  // Now query the subtypes of BaseClass. We should remember that SomeClass is a
  // subtype of BaseClass, but we should also fetch information from the DB
  // about OtherClass being a subtype of BaseClass.
  EXPECT_THAT(
      m2.getDerivedTypes("BaseClass", DeriveKind::Extends),
      UnorderedElementsAre("SomeClass", "OtherClass"));
}

TEST_F(SymbolMapTest, BaseTypesWithDifferentCases) {
  auto& m = make("/var/www", m_exec);

  FileFacts ff1{
      .m_types =
          {{.m_name = "SomeClass",
            .m_kind = TypeKind::Class,
            .m_baseTypes = {"BaseClass"}}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  FileFacts ff2{
      .m_types =
          {{.m_name = "SomeClass",
            .m_kind = TypeKind::Class,
            .m_baseTypes = {"baseclass"}}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab"};

  FileFacts ff3{
      .m_types = {{.m_name = "BaseClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac"};

  FileFacts ff4{
      .m_types = {{.m_name = "baseclass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaad"};

  folly::fs::path p1 = "some/path1.php";
  folly::fs::path p2 = "some/path2.php";
  folly::fs::path p3 = "some/path3.php";
  folly::fs::path p4 = "some/path4.php";

  m.update("", "1:2:3", {p1, p2, p3, p4}, {}, {ff1, ff2, ff3, ff4});
  EXPECT_EQ(m.getTypeFile("SomeClass"), nullptr);
  EXPECT_TRUE(m.getBaseTypes("SomeClass", DeriveKind::Extends).empty());
  EXPECT_TRUE(m.getDerivedTypes("BaseClass", DeriveKind::Extends).empty());
  EXPECT_TRUE(m.getDerivedTypes("baseclass", DeriveKind::Extends).empty());

  // Remove references to "baseclass", keep references to "BaseClass"
  m.update("1:2:3", "1:2:4", {}, {p2, p4}, {});
  EXPECT_EQ(m.getTypeFile("SomeClass"), p1.native());
  EXPECT_THAT(
      m.getBaseTypes("SomeClass", DeriveKind::Extends),
      ElementsAre("BaseClass"));
  EXPECT_THAT(
      m.getDerivedTypes("BaseClass", DeriveKind::Extends),
      ElementsAre("SomeClass"));
  EXPECT_THAT(
      m.getDerivedTypes("baseclass", DeriveKind::Extends),
      ElementsAre("SomeClass"));
}

TEST_F(SymbolMapTest, DerivedTypesWithDifferentCases) {
  auto& m = make("/var/www", m_exec);

  FileFacts ff1{
      .m_types =
          {{.m_name = "SomeClass",
            .m_kind = TypeKind::Class,
            .m_baseTypes = {"BaseClass"}},
           {.m_name = "BaseClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  folly::fs::path p1 = "some/path1.php";

  m.update("", "1", {p1}, {}, {ff1});
  EXPECT_EQ(m.getTypeFile("SomeClass").slice(), p1.native());
  EXPECT_THAT(m.getBaseTypes("SomeClass", DeriveKind::Extends),
      ElementsAre("BaseClass"));
  EXPECT_THAT(m.getDerivedTypes("BaseClass", DeriveKind::Extends),
      ElementsAre("SomeClass"));

  // Replace "SomeClass" with "SOMECLASS"
  FileFacts ff2{
      .m_types =
          {{.m_name = "SOMECLASS",
            .m_kind = TypeKind::Class,
            .m_baseTypes = {"BaseClass"}},
           {.m_name = "BaseClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};
  m.update("1", "2", {p1}, {}, {ff2});

  EXPECT_EQ(m.getTypeFile("SOMECLASS"), p1.native());
  EXPECT_THAT(
      m.getBaseTypes("SOMECLASS", DeriveKind::Extends),
      ElementsAre("BaseClass"));
  EXPECT_THAT(
      m.getDerivedTypes("BaseClass", DeriveKind::Extends),
      ElementsAre("SOMECLASS"));
}

TEST_F(SymbolMapTest, GetSymbolsInFileFromDB) {
  auto& m1 = make("/var/www", m_exec);

  FileFacts ff{
      .m_types =
          {{.m_name = "SomeClass", .m_kind = TypeKind::Class},
           {.m_name = "OtherClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  folly::fs::path path = {"some/path1.php"};

  m1.update("", "1:2:3", {path}, {}, {ff});

  EXPECT_EQ(m1.getTypeFile("SomeClass"), path.native());
  EXPECT_THAT(
    m1.getFileTypes(path.native()),
    UnorderedElementsAre("SomeClass", "OtherClass"));
  EXPECT_EQ(m1.getTypeFile("OtherClass"), path.native());

  waitForDB(m1, m_exec);

  auto& m2 = make("/var/www");
  m2.update("1:2:3", "1:2:3", {}, {}, {});

  // getTypeFile() and getFileTypes() both fill in-memory maps from the DB.
  // Make sure we never think we know all the types in a given path when we
  // really only know some of them.
  EXPECT_EQ(m2.getTypeFile("SomeClass"), path.native());
  EXPECT_THAT(
    m2.getFileTypes(path.native()),
    UnorderedElementsAre("SomeClass", "OtherClass"));
  EXPECT_EQ(m2.getTypeFile("OtherClass"), path.native());
}

TEST_F(SymbolMapTest, ErasePathStoredInDB) {
  auto& m1 = make("/var/www", m_exec);

  FileFacts ff{
      .m_types = {{.m_name = "SomeClass", .m_kind = TypeKind::Class}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  folly::fs::path p = {"some/path.php"};

  m1.update("", "1:2:3", {p}, {}, {ff});
  EXPECT_EQ(m1.getTypeFile("SomeClass"), p.native());
  waitForDB(m1, m_exec);

  auto& m2 = make("/var/www", m_exec);
  m2.update("1:2:3", "1:2:4", {}, {p}, {});
  m2.update("1:2:4", "1:2:5", {p}, {}, {ff});
  EXPECT_EQ(m2.getTypeFile("SomeClass"), p.native());
}

TEST_F(SymbolMapTest, GetTypesAndTypeAliasesWithAttribute) {
  auto& m1 = make("/var/www", m_exec);

  FileFacts ff{
      .m_types =
          {TypeDetails{
               .m_name = "SomeClass",
               .m_kind = TypeKind::Class,
               .m_attributes =
                   {{.m_name = "Foo", .m_args = {"apple", 38}},
                    {.m_name = "Bar", .m_args = {nullptr}},
                    {.m_name = "Baz", .m_args = {}}}},
           TypeDetails{
               .m_name = "OtherClass",
               .m_kind = TypeKind::Class,
               .m_attributes = {{.m_name = "Bar"}}},
           TypeDetails{
               .m_name = "SomeTypeAlias",
               .m_kind = TypeKind::TypeAlias,
               .m_attributes = {{.m_name = "Foo", .m_args = {42, "a"}}}}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  folly::fs::path p = {"some/path.php"};

  m1.update("", "1:2:3", {p}, {}, {ff});
  EXPECT_EQ(m1.getTypeFile("SomeClass"), p.native());
  EXPECT_THAT(
      m1.getTypesAndTypeAliasesWithAttribute("Foo"),
      UnorderedElementsAre("SomeClass", "SomeTypeAlias"));
  EXPECT_THAT(
      m1.getTypesAndTypeAliasesWithAttribute("Bar"),
      UnorderedElementsAre("SomeClass", "OtherClass"));
  EXPECT_THAT(
      m1.getAttributesOfType("SomeClass"),
      UnorderedElementsAre("Bar", "Baz", "Foo"));
  EXPECT_THAT(
      m1.getTypeAttributeArgs("SomeClass", "Foo"), ElementsAre("apple", 38));
  EXPECT_THAT(
      m1.getTypeAttributeArgs("SomeClass", "Bar"), ElementsAre(nullptr));
  EXPECT_THAT(m1.getTypeAttributeArgs("SomeClass", "Baz"), IsEmpty());
  EXPECT_THAT(
      m1.getAttributesOfType("SomeTypeAlias"), ElementsAre("Foo"));

  waitForDB(m1, m_exec);

  auto& m2 = make("/var/www", m_exec);
  m2.update("1:2:3", "1:2:3", {}, {}, {});
  EXPECT_THAT(
    m2.getAttributesOfType("SomeClass"),
    UnorderedElementsAre("Bar", "Baz", "Foo"));
  EXPECT_EQ(m2.getTypeFile("SomeClass"), p.native());
  EXPECT_THAT(
    m2.getTypesAndTypeAliasesWithAttribute("Foo"),
    UnorderedElementsAre("SomeClass", "SomeTypeAlias"));
  EXPECT_THAT(
    m2.getTypeAttributeArgs("SomeTypeAlias", "Foo"), ElementsAre(42, "a"));
  EXPECT_THAT(
    m2.getTypesAndTypeAliasesWithAttribute("Bar"),
    UnorderedElementsAre("SomeClass", "OtherClass"));
  EXPECT_THAT(
    m1.getTypeAttributeArgs("SomeClass", "Foo"), ElementsAre("apple", 38));
  EXPECT_THAT(
    m1.getTypeAttributeArgs("SomeClass", "Bar"), ElementsAre(nullptr));
  EXPECT_THAT(m1.getTypeAttributeArgs("SomeClass", "Baz"), IsEmpty());
  EXPECT_THAT(
    m1.getAttributesOfType("SomeTypeAlias"), ElementsAre("Foo"));
}

TEST_F(SymbolMapTest, getTypesWithAttributeFiltersDuplicateDefs) {
  auto& m1 = make("/var/www", m_exec);

  FileFacts ff{
      .m_types =
          {{.m_name = "SomeClass",
            .m_kind = TypeKind::Class,
            .m_attributes = {{.m_name = "Foo"}}}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  folly::fs::path p1 = {"some/path1.php"};
  folly::fs::path p2 = {"some/path2.php"};
  ASSERT_NE(p1.native(), p2.native());

  m1.update("", "1:2:3", {p1, p2}, {}, {ff, ff});
  EXPECT_EQ(m1.getTypeFile("SomeClass"), nullptr);
  EXPECT_TRUE(m1.getTypesAndTypeAliasesWithAttribute("Foo").empty());

  waitForDB(m1, m_exec);

  auto& m2 = make("/var/www", m_exec);
  m2.update("1:2:3", "1:2:3", {}, {}, {});
  EXPECT_EQ(m2.getTypeFile("SomeClass"), nullptr);
  EXPECT_TRUE(m2.getTypesAndTypeAliasesWithAttribute("Foo").empty());

  m2.update("1:2:3", "1:2:4", {}, {p2}, {});
  auto fooTypes = m2.getTypesAndTypeAliasesWithAttribute("Foo");
  EXPECT_THAT(fooTypes, ElementsAre("SomeClass"));
  EXPECT_EQ(m2.getTypeFile("SomeClass"), p1.native());
}

TEST_F(SymbolMapTest, GetMethodsWithAttribute) {
  auto& m1 = make("/var/www");

  FileFacts ff1{
      .m_types =
          {
              TypeDetails{
                  .m_name = "C1",
                  .m_methods =
                      {MethodDetails{
                           .m_name = "m1",
                           .m_attributes = {{.m_name = "A1", .m_args = {1}}}},
                       MethodDetails{
                           .m_name = "m2",
                           .m_attributes = {{.m_name = "A1", .m_args = {2}}},
                       }}},
          },
      .m_sha1hex = kSHA};
  folly::fs::path p1{"some/path1.php"};
  m1.update("", "1", {p1}, {}, {ff1});

  auto testMap = [&p1](auto& m) {
    auto methods = m.getMethodsWithAttribute("A1");
    EXPECT_THAT(methods, AllOf(
          SizeIs(2),
          Contains(
            MethodDecl<std::string>{
                .m_type =
                    {.m_name =
                         Symbol<std::string, SymKind::Type>{std::string{"C1"}},
                     .m_path = Path<std::string>{p1}},
                .m_method =
                    Symbol<std::string, SymKind::Function>{std::string{"m1"}}})));

    EXPECT_THAT(m.getAttributesOfMethod("C1", "m1"), ElementsAre("A1"));
    EXPECT_THAT(m.getMethodAttributeArgs("C1", "m1", "A1"), ElementsAre(1));
    EXPECT_THAT(m.getAttributesOfMethod("C1", "m2"), ElementsAre("A1"));
    EXPECT_THAT(m.getMethodAttributeArgs("C1", "m2", "A1"), ElementsAre(2));
  };
  testMap(m1);

  m1.waitForDBUpdate();
  auto& m2 = make("/var/www");
  m2.update("1", "1", {}, {}, {});
  testMap(m2);
}

TEST_F(SymbolMapTest, GetAttributesOfRenamedMethod) {
  auto& m1 = make("/var/www");

  // Create method `C1::m1`
  FileFacts ff1{
      .m_types = {TypeDetails{
          .m_name = "C1",
          .m_methods =
              {
                  MethodDetails{
                      .m_name = "m1",
                      .m_attributes = {{.m_name = "A1", .m_args = {1}}}},
              }}},
      .m_sha1hex = kSHA};
  folly::fs::path p1{"some/path1.php"};
  m1.update("", "1", {p1}, {}, {ff1});

  {
    auto methods = m1.getMethodsWithAttribute("A1");
    ASSERT_EQ(methods.size(), 1);
    EXPECT_EQ(methods[0].m_type.m_name.slice(), "C1");
    EXPECT_EQ(methods[0].m_method.m_name.slice(), "m1");
  }

  // Flush to the DB so m2 can see `C1::m1`
  m1.waitForDBUpdate();

  // m2 doesn't have `C1::m1` in its in-memory map but can see it in the DB
  auto& m2 = make("/var/www");

  // Rename method `C1::m1` to `C1::m2`
  FileFacts ff2{
      .m_types = {TypeDetails{
          .m_name = "C1",
          .m_methods =
              {
                  MethodDetails{
                      .m_name = "m2",
                      .m_attributes = {{.m_name = "A1", .m_args = {1}}}},
              }}},
      .m_sha1hex = kSHA};

  m2.update("1", "2", {p1}, {}, {ff2});

  {
    auto methods = m2.getMethodsWithAttribute("A1");
    ASSERT_EQ(methods.size(), 1);
    EXPECT_EQ(methods[0].m_type.m_name.slice(), "C1");
    EXPECT_EQ(methods[0].m_method.m_name.slice(), "m2");
  }
}

TEST_F(SymbolMapTest, GetFilesWithAttribute) {
  auto& m1 = make("/var/www");

  FileFacts ff1{
      .m_attributes = {{.m_name = "A1", .m_args = {1}}}, .m_sha1hex = kSHA};
  folly::fs::path p1{"some/path1.php"};
  m1.update("", "1", {p1}, {}, {ff1});

  auto testMap = [&p1](auto& m) {
    auto files = m.getFilesWithAttribute("A1");
    EXPECT_THAT(files, ElementsAre(p1.native()));

    auto attrs = m.getAttributesOfFile(Path{p1.native()});
    EXPECT_THAT(attrs, ElementsAre("A1"));

    auto args = m.getFileAttributeArgs(Path{p1.native()}, "A1");
    EXPECT_THAT(args, ElementsAre(1));
  };
  testMap(m1);

  m1.waitForDBUpdate();
  auto& m2 = make("/var/www");
  m2.update("1", "1", {}, {}, {});
  testMap(m2);
}

TEST_F(SymbolMapTest, TransitiveSubtypes) {
  auto& m1 = make("/var/www");

  FileFacts ff{
      .m_types =
          {TypeDetails{.m_name = "C0", .m_kind = TypeKind::Class},
           TypeDetails{
               .m_name = "C1",
               .m_kind = TypeKind::Class,
               .m_baseTypes = {"C0"}},
           TypeDetails{
               .m_name = "C2",
               .m_kind = TypeKind::Class,
               .m_baseTypes = {"C1", "T0"}},
           TypeDetails{
               .m_name = "I1",
               .m_kind = TypeKind::Interface,
               .m_baseTypes = {"C1"}},
           TypeDetails{
               .m_name = "C3",
               .m_kind = TypeKind::Class,
               .m_baseTypes = {"I1"}},
           TypeDetails{
               .m_name = "T0",
               .m_kind = TypeKind::Trait,
               .m_requireExtends = {"C0"}},
           TypeDetails{
               .m_name = "T1",
               .m_kind = TypeKind::Trait,
               .m_requireImplements = {"I1"}}},
      .m_sha1hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};

  folly::fs::path p = {"some/path.php"};

  m1.update("", "1:2:3", {p}, {}, {ff});

  EXPECT_THAT(
    m1.getTransitiveDerivedTypes("C0"),
    UnorderedElementsAre("C1", "C2", "C3", "I1", "T0", "T1"));

  EXPECT_THAT(
    m1.getTransitiveDerivedTypes("C0", static_cast<int>(TypeKind::Class)),
    UnorderedElementsAre("C1", "C2"));

  EXPECT_THAT(
    m1.getTransitiveDerivedTypes(
      "C0", kTypeKindAll, static_cast<int>(DeriveKind::Extends)),
    UnorderedElementsAre("C1", "C2", "C3", "I1"));

  EXPECT_THAT(
    m1.getTransitiveDerivedTypes(
        "C0", kTypeKindAll, static_cast<int>(DeriveKind::RequireExtends)),
    ElementsAre("T0"));
}

TEST_F(SymbolMapTest, ConcurrentFillsFromDB) {
  auto& dbUpdater = make("/var/www", m_exec);
  auto& map = make("/var/www");

  auto makeSym = [](size_t i) -> std::string {
    return folly::sformat("Class{}", i);
  };

  auto makePath = [](size_t i) -> folly::fs::path {
    return folly::sformat("some/path{}.php", i);
  };

  size_t const numSymbols = 100;

  std::vector<folly::fs::path> paths;
  std::vector<FileFacts> facts;
  for (auto i = 0; i < numSymbols; ++i) {
    FileFacts ff;
    std::vector<std::string> baseTypes;
    baseTypes.reserve(i);
    for (auto j = 0; j < i; ++j) {
      baseTypes.push_back(makeSym(j));
    }
    ff.m_types = {
        {.m_name = makeSym(i),
         .m_kind = TypeKind::Class,
         .m_baseTypes = std::move(baseTypes)}};
    ff.m_sha1hex = folly::sformat("{:a>40x}", i);

    paths.push_back(makePath(i));
    facts.push_back(std::move(ff));
  }

  // Update the DB with all path information
  dbUpdater.update("", "1:2:3", std::move(paths), {}, std::move(facts));
  waitForDB(dbUpdater, m_exec);

  map.update("1:2:3", "1:2:3", {}, {}, {});

  // Load symbols concurrently
  size_t const numThreads = 20;
  folly::CPUThreadPoolExecutor exec{numThreads};
  std::vector<folly::Future<folly::Unit>> futures;
  for (auto i = 0; i < numThreads; ++i) {
    futures.push_back(folly::via(&exec, [&]() {
      for (auto j = 0; j < numSymbols; ++j) {
        EXPECT_EQ(map.getKind(makeSym(j)), TypeKind::Class);
        EXPECT_EQ(map.getTypeFile(makeSym(j)), makePath(j).native());
        EXPECT_EQ(map.getBaseTypes(makeSym(j), DeriveKind::Extends).size(), j);
        EXPECT_EQ(
            map.getDerivedTypes(makeSym(j), DeriveKind::Extends).size(),
            numSymbols - j - 1);
      }
    }));
  }

  folly::collect(futures);
}

} // namespace Facts
} // namespace HPHP
