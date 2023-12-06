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
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

#include <folly/ScopeGuard.h>
#include <folly/concurrency/ConcurrentHashMap.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/executors/ManualExecutor.h>
#include <folly/experimental/TestUtil.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include "hphp/runtime/ext/facts/exception.h"
#include "hphp/runtime/ext/facts/file-facts.h"
#include "hphp/runtime/ext/facts/sqlite-autoload-db.h"
#include "hphp/runtime/ext/facts/sqlite-key.h"
#include "hphp/runtime/ext/facts/string-ptr.h"
#include "hphp/runtime/ext/facts/symbol-map.h"
#include "hphp/util/bstring.h"
#include "hphp/util/hash-set.h"
#include "hphp/util/hash.h"

using ::testing::_;
using ::testing::AllOf;
using ::testing::Contains;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::IsEmpty;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAre;

/**
 * Implements our StringPtr class in terms of std::string.
 *
 * For unit-testing purposes only. By contrast, see
 * string_data_ptr.cpp, which implements this class in terms of
 * HPHP::StringData for production use.
 */

namespace fs = std::filesystem;

namespace HPHP {

struct StringData {
 public:
  // These two constructions allow implicit construction for convenience
  // in unit tests.
  /* implicit */ StringData(const char* s) : m_impl{s} {};
  /* implicit */ StringData(std::string&& s) : m_impl{std::move(s)} {}

  explicit StringData(std::string_view s) : m_impl{s} {}

  std::string* impl() const {
    return &m_impl;
  }
  std::string_view slice() const noexcept {
    return std::string_view{m_impl};
  }
  bool empty() const {
    return m_impl.empty();
  }
  size_t size() const {
    return m_impl.size();
  }
  strhash_t hash() const noexcept {
    strhash_t h = hash_string_i_unsafe(m_impl.c_str(), m_impl.size());
    assertx(h >= 0);
    return h;
  }
  bool same(const StringData& o) const noexcept {
    return m_impl == o.m_impl;
  }
  bool isame(const StringData& o) const noexcept {
    auto lower = [](const std::string& _s) {
      std::string s = _s;
      std::transform(s.begin(), s.end(), s.begin(), ::tolower);
      return s;
    };
    return lower(m_impl) == lower(o.m_impl);
  }

 private:
  mutable std::string m_impl;
};

namespace Facts {

/**
 * Insert-only store of static pointers
 */
folly::ConcurrentHashMap<StringPtr, std::unique_ptr<StringData>> s_stringTable;

std::string_view StringPtr::StringPtr::slice() const noexcept {
  assertx(m_impl != nullptr);
  return std::string_view{*m_impl->impl()};
}

int StringPtr::StringPtr::size() const noexcept {
  assertx(m_impl != nullptr);
  return m_impl->impl()->size();
}

bool StringPtr::StringPtr::empty() const noexcept {
  assertx(m_impl != nullptr);
  return m_impl->impl()->empty();
}

strhash_t StringPtr::StringPtr::hash() const noexcept {
  assertx(m_impl != nullptr);
  return hash_string_i(m_impl->impl()->c_str(), m_impl->impl()->size());
}

bool StringPtr::StringPtr::same(const StringPtr& s) const noexcept {
  if (m_impl->impl() == s.m_impl->impl()) {
    return true;
  }
  if (m_impl->impl() == nullptr || s.m_impl->impl() == nullptr) {
    return false;
  }
  return *m_impl->impl() == *s.m_impl->impl();
}

bool StringPtr::StringPtr::isame(const StringPtr& s) const noexcept {
  if (m_impl->impl() == s.m_impl->impl()) {
    return true;
  }
  if (m_impl->impl() == nullptr || s.m_impl->impl() == nullptr) {
    return false;
  }
  if (m_impl->impl()->size() != s.m_impl->impl()->size()) {
    return false;
  }
  return bstrcaseeq(
      m_impl->impl()->c_str(),
      s.m_impl->impl()->c_str(),
      m_impl->impl()->size());
}

StringPtr makeStringPtr(const StringData& s) {
  auto it = s_stringTable.find(StringPtr{&s});
  if (it != s_stringTable.end()) {
    return it->first;
  }

  auto staticStr = std::make_unique<StringData>(s);
  auto strKey = StringPtr{staticStr.get()};
  return StringPtr{
      s_stringTable.insert(strKey, std::move(staticStr)).first->first};
}

StringPtr makeStringPtr(std::string_view sv) {
  return makeStringPtr(StringData{sv});
}

std::ostream& operator<<(std::ostream& os, const StringPtr& s) {
  if (s.get()->impl() == nullptr) {
    return os << "<nullptr>";
  }
  return os << *s.get()->impl();
}

template <SymKind k>
void PrintTo(const Symbol<k>& symbol, ::std::ostream* os) {
  *os << symbol.slice();
}

template <SymKind k>
bool operator==(const Symbol<k>& symbol, const char* str) {
  return symbol.slice() == str;
}

template <SymKind k, typename T, typename U>
void PrintTo(
    const std::tuple<Symbol<k>, Path, T, U>& typeInfo,
    ::std::ostream* os) {
  *os << std::get<0>(typeInfo).slice();
}

void PrintTo(const Path& path, ::std::ostream* os) {
  if (path.m_path == nullptr) {
    *os << "<unset>";
  } else {
    *os << path.slice();
  }
}

template <SymKind k, typename T, typename U>
bool operator==(
    const std::tuple<Symbol<k>, Path, T, U>& typeInfo,
    const char* str) {
  return std::get<0>(typeInfo) == str;
}

bool operator==(const Path& path, const std::string& other) {
  return (path == std::string_view(other));
}

namespace {

constexpr int kTypeFlagAbstractBit = 1;
constexpr int kTypeFlagFinalBit = 2;

struct MockAutoloadDB : public AutoloadDB {
  MOCK_METHOD(bool, isReadOnly, (), (const override));
  MOCK_METHOD(void, commit, (), (override));

  MOCK_METHOD(
      void,
      insertPath,
      (const std::filesystem::path& path),
      (override));
  MOCK_METHOD(void, erasePath, (const std::filesystem::path& path), (override));

  MOCK_METHOD(
      void,
      insertSha1Hex,
      (const std::filesystem::path& path, const Optional<std::string>& sha1hex),
      (override));
  MOCK_METHOD(
      std::string,
      getSha1Hex,
      (const std::filesystem::path& path),
      (override));

  // Types
  MOCK_METHOD(
      void,
      insertType,
      (std::string_view type,
       const std::filesystem::path& path,
       TypeKind kind,
       TypeFlagMask flags),
      (override));
  MOCK_METHOD(
      std::vector<std::filesystem::path>,
      getTypePath,
      (std::string_view type),
      (override));
  MOCK_METHOD(
      std::vector<std::string>,
      getPathTypes,
      (const std::filesystem::path& path),
      (override));

  MOCK_METHOD(
      KindAndFlags,
      getKindAndFlags,
      (std::string_view type, const std::filesystem::path& path),
      (override));

  // Inheritance
  MOCK_METHOD(
      void,
      insertBaseType,
      (const std::filesystem::path& path,
       std::string_view derivedType,
       DeriveKind kind,
       std::string_view baseType),
      (override));
  MOCK_METHOD(
      std::vector<std::string>,
      getBaseTypes,
      (const std::filesystem::path& path,
       std::string_view derivedType,
       DeriveKind kind),
      (override));

  /**
   * Return all types extending the given baseType, along with the paths which
   * claim the derived type extends this baseType.
   *
   * Returns [(pathWhereDerivedTypeExtendsBaseType, derivedType)]
   */
  MOCK_METHOD(
      std::vector<SymbolPath>,
      getDerivedTypes,
      (std::string_view baseType, DeriveKind kind),
      (override));

  // Attributes

  MOCK_METHOD(
      void,
      insertTypeAttribute,
      (const std::filesystem::path& path,
       std::string_view type,
       std::string_view attributeName,
       Optional<int> attributePosition,
       const folly::dynamic* attributeValue),
      (override));

  MOCK_METHOD(
      void,
      insertMethodAttribute,
      (const std::filesystem::path& path,
       std::string_view type,
       std::string_view method,
       std::string_view attributeName,
       Optional<int> attributePosition,
       const folly::dynamic* attributeValue),
      (override));

  MOCK_METHOD(
      void,
      insertFileAttribute,
      (const std::filesystem::path& path,
       std::string_view attributeName,
       Optional<int> attributePosition,
       const folly::dynamic* attributeValue),
      (override));

  MOCK_METHOD(
      std::vector<std::string>,
      getAttributesOfType,
      (std::string_view type, const std::filesystem::path& path),
      (override));

  MOCK_METHOD(
      std::vector<std::string>,
      getAttributesOfMethod,
      (std::string_view type,
       std::string_view method,
       const std::filesystem::path& path),
      (override));

  MOCK_METHOD(
      std::vector<std::string>,
      getAttributesOfFile,
      (const std::filesystem::path& path),
      (override));

  MOCK_METHOD(
      std::vector<folly::dynamic>,
      getTypeAttributeArgs,
      (std::string_view type,
       std::string_view path,
       std::string_view attributeName),
      (override));

  MOCK_METHOD(
      std::vector<folly::dynamic>,
      getTypeAliasAttributeArgs,
      (std::string_view typeAlias,
       std::string_view path,
       std::string_view attributeName),
      (override));

  MOCK_METHOD(
      std::vector<folly::dynamic>,
      getMethodAttributeArgs,
      (std::string_view type,
       std::string_view method,
       std::string_view path,
       std::string_view attributeName),
      (override));

  MOCK_METHOD(
      std::vector<folly::dynamic>,
      getFileAttributeArgs,
      (std::string_view path, std::string_view attributeName),
      (override));

  MOCK_METHOD(
      std::vector<SymbolPath>,
      getTypesWithAttribute,
      (std::string_view attributeName),
      (override));
  MOCK_METHOD(
      std::vector<SymbolPath>,
      getTypeAliasesWithAttribute,
      (std::string_view attributeName),
      (override));

  MOCK_METHOD(
      std::vector<MethodPath>,
      getPathMethods,
      (std::string_view path),
      (override));

  MOCK_METHOD(
      std::vector<MethodPath>,
      getMethodsWithAttribute,
      (std::string_view attributeName),
      (override));

  MOCK_METHOD(
      std::vector<std::filesystem::path>,
      getFilesWithAttribute,
      (std::string_view attributeName),
      (override));
  MOCK_METHOD(
      std::vector<std::filesystem::path>,
      getFilesWithAttributeAndAnyValue,
      (const std::string_view attributeName,
       const folly::dynamic& attributeValue),
      (override));

  // Functions
  MOCK_METHOD(
      void,
      insertFunction,
      (std::string_view function, const std::filesystem::path& path),
      (override));
  MOCK_METHOD(
      std::vector<std::filesystem::path>,
      getFunctionPath,
      (std::string_view function),
      (override));
  MOCK_METHOD(
      std::vector<std::string>,
      getPathFunctions,
      (const std::filesystem::path& path),
      (override));

  // Constants
  MOCK_METHOD(
      void,
      insertConstant,
      (std::string_view constant, const std::filesystem::path& path),
      (override));
  MOCK_METHOD(
      std::vector<std::filesystem::path>,
      getConstantPath,
      (std::string_view constant),
      (override));
  MOCK_METHOD(
      std::vector<std::string>,
      getPathConstants,
      (const std::filesystem::path& path),
      (override));

  // Modules
  MOCK_METHOD(
      void,
      insertModule,
      (std::string_view, const std::filesystem::path&),
      (override));
  MOCK_METHOD(
      std::vector<std::filesystem::path>,
      getModulePath,
      (std::string_view),
      (override));
  MOCK_METHOD(
      std::vector<std::string>,
      getPathModules,
      (const std::filesystem::path&),
      (override));

  /**
   * Return a list of all paths defined in the given root, as absolute paths.
   *
   * Paths come paired with the last known SHA1 hash of the file.
   *
   * Returns results in the form of a lazy generator.
   */
  MOCK_METHOD(MultiResult<PathAndHash>, getAllPathsAndHashes, (), (override));

  MOCK_METHOD(void, insertClock, (const Clock& clock), (override));
  MOCK_METHOD(Clock, getClock, (), (override));
  MOCK_METHOD(void, runPostBuildOptimizations, (), (override));

  void DelegateToFake() {
    ON_CALL(*this, isReadOnly()).WillByDefault(Return(false));
    ON_CALL(*this, insertClock(_)).WillByDefault(SaveArg<0>(&clock_));
    ON_CALL(*this, getClock()).WillByDefault(Return(clock_));
  }

 private:
  Clock clock_;
};

/**
 * RAII wrapper which ensures we finish draining the given
 * ManualExecutor before ending each test.
 */
struct SymbolMapWrapper {
  SymbolMapWrapper(
      std::unique_ptr<SymbolMap> m,
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
      : m_exec{std::move(old.m_exec)}, m_map{std::move(old.m_map)} {}
  SymbolMapWrapper& operator=(SymbolMapWrapper&& old) noexcept = delete;

  std::shared_ptr<folly::ManualExecutor> m_exec;
  std::unique_ptr<SymbolMap> m_map;
};

/**
 * If a map has a manual executor assigned to it, use this to
 * explicitly tell the executor to run.
 */
void waitForDB(SymbolMap& m, std::shared_ptr<folly::ManualExecutor>& exec) {
  ASSERT_EQ(m.m_exec.get(), static_cast<folly::Executor*>(exec.get()));
  exec->drain();
  m.waitForDBUpdate();
}

std::string getSha1Hex(const FileFacts& ff) {
  return SHA1{hackc::hash_facts(ff)}.toString();
}

void update(
    SymbolMap& m,
    std::string_view since,
    std::string_view clock,
    std::vector<fs::path> alteredPaths,
    std::vector<fs::path> deletedPaths,
    std::vector<FileFacts> facts) {
  for (auto& ff : facts) {
    ff.sha1sum = getSha1Hex(ff);
  }
  m.update(
      Clock{.m_clock = std::string{since}},
      Clock{.m_clock = std::string{clock}},
      std::move(alteredPaths),
      std::move(deletedPaths),
      std::move(facts));
}

} // namespace

class SymbolMapTest : public ::testing::TestWithParam<bool> {
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

  SymbolMap& make(
      std::string root,
      std::shared_ptr<folly::ManualExecutor> exec = nullptr,
      std::vector<std::string> indexedMethodAttributesVec = {}) {
    auto dbPath = m_tmpdir->path() /
        folly::to<std::string>(
                      "autoload_", std::hash<std::string>{}(root), "_db.sql3");
    hphp_vector_set<Symbol<SymKind::Type>> indexedMethodAttributes;
    indexedMethodAttributes.reserve(indexedMethodAttributesVec.size());
    for (auto& attr : indexedMethodAttributesVec) {
      indexedMethodAttributes.insert(Symbol<SymKind::Type>{attr});
    }
    m_wrappers.push_back(SymbolMapWrapper{
        std::make_unique<SymbolMap>(
            std::move(root),
            [dbPath]() -> std::shared_ptr<AutoloadDB> {
              return SQLiteAutoloadDB::get(SQLiteKey::readWriteCreate(
                  fs::path{dbPath.native()}, static_cast<::gid_t>(-1), 0644));
            },
            std::move(indexedMethodAttributes)),
        std::move(exec)});
    return *m_wrappers.back().m_map;
  }

  SymbolMap& make(
      std::string root,
      std::shared_ptr<MockAutoloadDB> db,
      std::shared_ptr<folly::ManualExecutor> exec = nullptr,
      std::vector<std::string> indexedMethodAttributesVec = {}) {
    hphp_vector_set<Symbol<SymKind::Type>> indexedMethodAttributes;
    indexedMethodAttributes.reserve(indexedMethodAttributesVec.size());
    for (auto& attr : indexedMethodAttributesVec) {
      indexedMethodAttributes.insert(Symbol<SymKind::Type>{attr});
    }
    m_wrappers.push_back(SymbolMapWrapper{
        std::make_unique<SymbolMap>(
            std::move(root),
            [&db]() -> std::shared_ptr<AutoloadDB> { return db; },
            std::move(indexedMethodAttributes)),
        std::move(exec)});
    return *m_wrappers.back().m_map;
  }

  std::unique_ptr<folly::test::TemporaryDirectory> m_tmpdir;
  std::shared_ptr<folly::ManualExecutor> m_exec;
  std::shared_ptr<folly::ManualExecutor> m_exec2;
  std::vector<SymbolMapWrapper> m_wrappers;
};

TEST_F(SymbolMapTest, NewModules) {
  auto db = std::make_shared<MockAutoloadDB>();
  db->DelegateToFake();

  auto& m = make("/var/www", db);
  fs::path path1 = {"some/path1.php"};

  FileFacts ff{
      .modules = {{.name = "some_module"}, {.name = "some_other_module"}}};

  EXPECT_CALL(*db, insertModule("some_module", path1));
  EXPECT_CALL(*db, insertModule("some_other_module", path1));
  update(m, "", "1:2:3", {path1}, {}, {ff});

  // We didn't actually write to any database, so if the symbol map is working,
  // we should still get the results.
  EXPECT_THAT(m.getModuleFile("some_module"), Eq(path1));
  EXPECT_THAT(m.getModuleFile("some_other_module"), Eq(path1));
  EXPECT_THAT(
      m.getFileModules(path1),
      UnorderedElementsAre("some_module", "some_other_module"));

  m_wrappers.clear();
}

TEST_F(SymbolMapTest, ModulesFromDB) {
  auto db = std::make_shared<MockAutoloadDB>();
  db->DelegateToFake();

  auto& m = make("/var/www", db);
  fs::path path1 = {"some/path1.php"};
  fs::path path2 = {"some/path2.php"};

  EXPECT_CALL(*db, insertPath(_)).Times(0);

  EXPECT_CALL(*db, getModulePath("some_new_module"))
      .WillOnce(Return(std::vector<fs::path>{path1}));
  EXPECT_CALL(*db, getModulePath("some_other_new_module"))
      .WillOnce(Return(std::vector<fs::path>{path1}));
  EXPECT_CALL(*db, getModulePath("some_new_third_module"))
      .WillOnce(Return(std::vector<fs::path>{path2}));

  EXPECT_CALL(*db, getPathModules(path1))
      .WillOnce(Return(std::vector<std::string>{
          "some_new_module", "some_other_new_module"}));
  EXPECT_CALL(*db, getPathModules(path2))
      .WillOnce(Return(std::vector<std::string>{"some_new_third_module"}));

  EXPECT_THAT(m.getModuleFile("some_new_module"), Eq(path1));
  EXPECT_THAT(m.getModuleFile("some_other_new_module"), Eq(path1));
  EXPECT_THAT(m.getModuleFile("some_new_third_module"), Eq(path2));
  EXPECT_THAT(
      m.getFileModules(path1),
      UnorderedElementsAre("some_new_module", "some_other_new_module"));
  EXPECT_THAT(
      m.getFileModules(path2), UnorderedElementsAre("some_new_third_module"));

  m_wrappers.clear();
}

TEST_F(SymbolMapTest, OverwriteExistingDbModules) {
  const char* kFirstPath1 = "k-1-1";
  const char* kFirstPath2 = "k-1-2";
  const char* kFirstPath3 = "k-1-3";
  const char* kSecondPath1 = "k-2-1";
  const char* kSecondPath2 = "k-2-2";
  const char* kSecondPath3 = "k-2-3";

  auto db = std::make_shared<NiceMock<MockAutoloadDB>>();
  db->DelegateToFake();

  auto& m = make("/var/www", db);
  fs::path path1 = {"some/path1.php"};
  fs::path path2 = {"some/path2.php"};

  // These paths won't be in memory, so the symbol map will try to get
  // them from the database.
  EXPECT_CALL(*db, getModulePath(kFirstPath1))
      .WillOnce(Return(std::vector<fs::path>{path1}));
  EXPECT_CALL(*db, getModulePath(kFirstPath2))
      .WillOnce(Return(std::vector<fs::path>{path1}));
  EXPECT_CALL(*db, getModulePath(kFirstPath3))
      .WillOnce(Return(std::vector<fs::path>{path2}));

  EXPECT_CALL(*db, getPathModules(path1))
      .WillOnce(Return(std::vector<std::string>{kFirstPath1, kFirstPath2}));
  EXPECT_CALL(*db, getPathModules(path2))
      .WillOnce(Return(std::vector<std::string>{kFirstPath3}));

  // Everything should be read from the database.
  EXPECT_THAT(m.getModuleFile(kFirstPath1), Eq(path1));
  EXPECT_THAT(m.getModuleFile(kFirstPath2), Eq(path1));
  EXPECT_THAT(m.getModuleFile(kFirstPath3), Eq(path2));
  EXPECT_THAT(
      m.getFileModules(path1), UnorderedElementsAre(kFirstPath1, kFirstPath2));
  EXPECT_THAT(m.getFileModules(path2), UnorderedElementsAre(kFirstPath3));

  // Now we overwrite it with new information.  The new information should
  // be returned by the symbol map and the db should get updated.
  FileFacts ff1{.modules = {{.name = kSecondPath1}}};
  FileFacts ff2{.modules = {{.name = kSecondPath2}, {.name = kSecondPath3}}};

  EXPECT_CALL(*db, erasePath(path1));
  EXPECT_CALL(*db, erasePath(path2));
  EXPECT_CALL(*db, insertModule(kSecondPath1, path1));
  EXPECT_CALL(*db, insertModule(kSecondPath2, path2));
  EXPECT_CALL(*db, insertModule(kSecondPath3, path2));

  update(m, "", "1:2:3", {path1, path2}, {}, {ff1, ff2});

  // TODO:  This doesn't seem right.  It seems like it should happen for the old
  // values
  /*
  EXPECT_CALL(db, getModulePath(kSecondPath1))
      .WillOnce(Return(std::vector<fs::path>{}));
  EXPECT_CALL(db, getModulePath(kSecondPath2))
      .WillOnce(Return(std::vector<fs::path>{}));
  EXPECT_CALL(db, getModulePath(kSecondPath3))
      .WvillOnce(Return(std::vector<fs::path>{}));
  */

  // The in memory map should already know these were blown away.
  Path empty_path{nullptr};
  EXPECT_CALL(*db, getModulePath(kFirstPath1)).Times(0);
  EXPECT_CALL(*db, getModulePath(kFirstPath2)).Times(0);
  EXPECT_CALL(*db, getModulePath(kFirstPath3)).Times(0);
  EXPECT_THAT(m.getModuleFile(kFirstPath1), Eq(empty_path));
  EXPECT_THAT(m.getModuleFile(kFirstPath2), Eq(empty_path));
  EXPECT_THAT(m.getModuleFile(kFirstPath3), Eq(empty_path));

  std::vector<fs::path> empty_path_list{};
  EXPECT_CALL(*db, getModulePath(kSecondPath1))
      .WillOnce(Return(empty_path_list));
  EXPECT_CALL(*db, getModulePath(kSecondPath2))
      .WillOnce(Return(empty_path_list));
  EXPECT_CALL(*db, getModulePath(kSecondPath3))
      .WillOnce(Return(empty_path_list));
  EXPECT_CALL(*db, getPathModules(path1)).Times(0);
  EXPECT_CALL(*db, getPathModules(path2)).Times(0);
  EXPECT_THAT(m.getModuleFile(kSecondPath1), Eq(path1));
  EXPECT_THAT(m.getModuleFile(kSecondPath2), Eq(path2));
  EXPECT_THAT(m.getModuleFile(kSecondPath3), Eq(path2));

  EXPECT_THAT(m.getFileModules(path1), UnorderedElementsAre(kSecondPath1));
  EXPECT_THAT(
      m.getFileModules(path2),
      UnorderedElementsAre(kSecondPath2, kSecondPath3));

  m_wrappers.clear();
}

TEST_F(SymbolMapTest, addPaths) {
  auto& m = make("/var/www");

  FileFacts ff{
      .types =
          {{.name = "SomeClass",
            .kind = TypeKind::Class,
            .base_types = {"BaseClass"}},
           {.name = "BaseClass", .kind = TypeKind::Class},
           {.name = "SomeTypeAlias", .kind = TypeKind::TypeAlias}},
      .functions = {"some_fn"},
      .constants = {"SOME_CONSTANT"},
      .modules = {{.name = "some_module"}, {.name = "some_other_module"}}};

  // Define symbols in path
  fs::path path1 = {"some/path1.php"};
  update(m, "", "1:2:3", {path1}, {}, {ff});
  EXPECT_EQ(m.getClock().m_clock, "1:2:3");

  // Symbol -> file
  EXPECT_EQ(m.getTypeFile("SomeClass"), path1.native());
  EXPECT_EQ(m.getFunctionFile("some_fn"), path1.native());
  EXPECT_EQ(m.getConstantFile("SOME_CONSTANT"), path1.native());
  EXPECT_EQ(m.getTypeAliasFile("SomeTypeAlias"), path1.native());
  EXPECT_EQ(m.getModuleFile("some_module"), path1.native());

  // Types and type aliases are still different
  EXPECT_EQ(m.getTypeAliasFile("SomeClass"), nullptr);
  EXPECT_EQ(m.getTypeFile("SomeTypeAlias"), nullptr);

  // file -> symbols
  EXPECT_THAT(
      m.getFileTypes(path1), UnorderedElementsAre("BaseClass", "SomeClass"));
  EXPECT_EQ(m.getFileFunctions(path1).at(0), "some_fn");
  EXPECT_EQ(m.getFileConstants(path1).at(0), "SOME_CONSTANT");
  EXPECT_EQ(m.getFileTypeAliases(path1).at(0), "SomeTypeAlias");
  EXPECT_THAT(
      m.getFileModules(path1),
      UnorderedElementsAre("some_module", "some_other_module"));

  // Check for case insensitivity
  EXPECT_EQ(m.getTypeFile("someclass"), path1.native());
  EXPECT_EQ(m.getFunctionFile("SOME_FN"), path1.native());
  EXPECT_EQ(m.getTypeAliasFile("sometypealias"), path1.native());

  // Check for case sensitivity
  EXPECT_EQ(m.getConstantFile("Some_Constant"), nullptr);
  EXPECT_EQ(m.getModuleFile("Some_Module"), nullptr);

  // Check for undefined symbols
  EXPECT_EQ(m.getTypeFile("UndefinedClass"), nullptr);
  EXPECT_EQ(m.getFunctionFile("undefined_fn"), nullptr);
  EXPECT_EQ(m.getConstantFile("UNDEFINED_CONSTANT"), nullptr);
  EXPECT_EQ(m.getTypeAliasFile("UndefinedTypeAlias"), nullptr);
  EXPECT_EQ(m.getModuleFile("undefined_module"), nullptr);

  // Check inheritance
  EXPECT_EQ(
      m.getBaseTypes("SomeClass", DeriveKind::Extends).at(0), "BaseClass");
  EXPECT_TRUE(m.getDerivedTypes("SomeClass", DeriveKind::Extends).empty());
  EXPECT_TRUE(m.getBaseTypes("BaseClass", DeriveKind::Extends).empty());
  EXPECT_EQ(
      m.getDerivedTypes("BaseClass", DeriveKind::Extends).at(0), "SomeClass");

  // Define duplicate symbols in path2
  fs::path path2 = {"some/path2.php"};
  ASSERT_NE(path1, path2);

  update(m, "1:2:3", "1:2:4", {path2}, {}, {ff});
  EXPECT_EQ(m.getClock().m_clock, "1:2:4");

  // Remove path1, leaving the symbols defined in path2
  update(m, "1:2:4", "1:2:5", {}, {path1}, {});
  EXPECT_EQ(m.getClock().m_clock, "1:2:5");
  EXPECT_EQ(m.getTypeFile("SomeClass"), path2.native());
  EXPECT_EQ(m.getFunctionFile("some_fn"), path2.native());
  EXPECT_EQ(m.getConstantFile("SOME_CONSTANT"), path2.native());
  EXPECT_EQ(m.getTypeAliasFile("SomeTypeAlias"), path2.native());
  EXPECT_EQ(m.getModuleFile("some_module"), path2.native());

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
      .types =
          {{.name = "SomeClass", .kind = TypeKind::Class},
           {.name = "SomeTypeAlias", .kind = TypeKind::TypeAlias}},
      .functions = {"some_fn"},
      .constants = {"SOME_CONSTANT"}};

  fs::path path1 = {"some/path1.php"};
  fs::path path2 = {"some/path2.php"};
  ASSERT_NE(path1, path2);
  update(m, "", "1:2:3", {path1, path2}, {}, {ff, ff});

  EXPECT_EQ(m.getClock().m_clock, "1:2:3");

  // Remove the duplicate definition and verify that we're back in a sane
  // state
  update(m, "1:2:3", "1:2:4", {}, {path1}, {});
  EXPECT_EQ(m.getTypeFile("SomeClass"), path2.native());
  EXPECT_EQ(m.getFunctionFile("some_fn"), path2.native());
  EXPECT_EQ(m.getConstantFile("SOME_CONSTANT"), path2.native());
  EXPECT_EQ(m.getTypeAliasFile("SomeTypeAlias"), path2.native());
}

TEST_F(SymbolMapTest, DBFill) {
  auto& m1 = make("/var/www");
  auto& m2 = make("/var/www");

  FileFacts ff{
      .types =
          {{.name = "SomeClass",
            .kind = TypeKind::Class,
            .base_types = {"BaseClass"}},
           {.name = "BaseClass", .kind = TypeKind::Class},
           {.name = "SomeTypeAlias", .kind = TypeKind::TypeAlias}},
      .functions = {"some_fn"},
      .constants = {"SOME_CONSTANT"}};

  fs::path path = {"some/path.php"};

  update(m1, "", "1:2:3", {path}, {}, {ff});

  // symbol -> file
  EXPECT_EQ(m1.getTypeFile("SomeClass"), path.native());
  EXPECT_EQ(m1.getFunctionFile("some_fn"), path.native());
  EXPECT_EQ(m1.getConstantFile("SOME_CONSTANT"), path.native());
  EXPECT_EQ(m1.getTypeAliasFile("SomeTypeAlias"), path.native());

  // file -> symbol
  EXPECT_THAT(
      m1.getFileTypes(path), UnorderedElementsAre("BaseClass", "SomeClass"));
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

  update(m2, "1:2:3", "2:3:4", {}, {}, {});

  // file -> symbol
  EXPECT_THAT(
      m2.getFileTypes(path), UnorderedElementsAre("BaseClass", "SomeClass"));
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
  FileFacts ff{.types = {{.name = "HTTPDNSURL", .kind = TypeKind::Class}}};

  fs::path path = {"some/path.php"};
  update(m, "", "1:2:3", {path}, {}, {ff});

  EXPECT_EQ(m.getTypeName("httpdnsurl")->slice(), "HTTPDNSURL");
  EXPECT_EQ(m.getFileTypes(path).at(0).slice(), "HTTPDNSURL");
  EXPECT_NE(m.getFileTypes(path).at(0).slice(), "HttpDnsUrl");

  // Classes are case-insensitive, but changing the case of a class
  // should change the canonical representation
  ff.types.at(0).name = "HttpDnsUrl";
  update(m, "1:2:3", "1:2:4", {path}, {}, {ff});
  auto types = m.getFileTypes(path);
  EXPECT_EQ(m.getTypeName("httpdnsurl")->slice(), "HttpDnsUrl");
  EXPECT_EQ(types.at(0).slice(), "HttpDnsUrl");
  EXPECT_NE(types.at(0).slice(), "HTTPDNSURL");

  m.waitForDBUpdate();

  auto& m2 = make("/var/www");
  EXPECT_EQ(m2.dbClock().m_clock, "1:2:4");
  EXPECT_TRUE(m2.dbClock().m_mergebase.empty());
  update(m2, "1:2:4", "1:2:4", {}, {}, {});
  EXPECT_EQ(m2.getClock().m_clock, "1:2:4");
  auto dbTypes = m2.getFileTypes(path);
  EXPECT_EQ(m.getTypeName("httpdnsurl")->slice(), "HttpDnsUrl");
  EXPECT_EQ(types.at(0).slice(), "HttpDnsUrl");
}

TEST_F(SymbolMapTest, ChangeBaseClassSymbolCase) {
  auto& m = make("/var/www");
  FileFacts ff1{
      .types = {
          {.name = "SomeClass",
           .kind = TypeKind::Class,
           .base_types = {"baseclass"}}}};

  FileFacts ff2{.types = {{.name = "baseclass", .kind = TypeKind::Class}}};

  fs::path path1 = {"some/path1.php"};
  fs::path path2 = {"some/path2.php"};
  update(m, "", "1:2:3", {path1, path2}, {}, {ff1, ff2});
  EXPECT_THAT(
      m.getBaseTypes("SomeClass", DeriveKind::Extends),
      ElementsAre("baseclass"));
  EXPECT_THAT(
      m.getDerivedTypes("baseclass", DeriveKind::Extends),
      ElementsAre("SomeClass"));

  ff1.types.at(0).base_types.at(0) = "BaseClass";
  ff2.types.at(0).name = "BaseClass";
  update(m, "1:2:3", "1:2:4", {path1, path2}, {}, {ff1, ff2});
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
      .types =
          {{.name = "CamelCasedType", .kind = TypeKind::Class},
           {.name = "CamelCasedTypeAlias", .kind = TypeKind::TypeAlias}},
      .functions = {{"CamelCasedFunction"}},
      .constants = {{"CamelCasedConstant"}}};

  fs::path path = {"some/path.php"};
  update(m1, "", "1:2:3", {path}, {}, {ff});
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

  FileFacts ff{.types = {{.name = "SomeClass", .kind = TypeKind::Class}}};

  fs::path path1 = {"some/path.php"};
  fs::path path2 = {"some/path2.php"};
  ASSERT_NE(path1, path2);

  update(m1, "", "1:2:3", {path1, path2}, {}, {ff, ff});
  EXPECT_EQ(m1.getFileTypes(path1).at(0).slice(), "SomeClass");
  EXPECT_EQ(m1.getFileTypes(path2).at(0).slice(), "SomeClass");

  m1.waitForDBUpdate();
  update(m2, "1:2:3", "1:2:3", {}, {}, {});

  EXPECT_EQ(m2.getFileTypes(path1).at(0).slice(), "SomeClass");
  EXPECT_EQ(m2.getFileTypes(path2).at(0).slice(), "SomeClass");

  EXPECT_EQ(m2.getFileTypes(path1).at(0).slice(), "SomeClass");
  EXPECT_EQ(m2.getFileTypes(path2).at(0).slice(), "SomeClass");
}

TEST_F(SymbolMapTest, CopiedFile) {
  auto& m1 = make("/var/www");

  fs::path path1 = {"some/path.php"};
  fs::path path2 = {"some/path2.php"};
  ASSERT_NE(path1, path2);

  FileFacts ff{
      .types =
          {{.name = "SomeClass", .kind = TypeKind::Class},
           {.name = "OtherClass", .kind = TypeKind::Class},
           {.name = "SomeTypeAlias", .kind = TypeKind::TypeAlias}},
      .functions = {"SomeFunction"},
      .constants = {"SomeConstant"}};

  update(m1, "", "1:2:3", {path1}, {}, {ff});
  m1.waitForDBUpdate();

  auto& m2 = make("/var/www");
  update(m2, "1:2:3", "1:2:4", {path2}, {}, {ff});

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

TEST_F(SymbolMapTest, DoesNotFillDeadPathFromDB) {
  auto& m1 = make("/var/www", m_exec);
  auto& m2 = make("/var/www", m_exec);
  auto& m3 = make("/var/www", m_exec);

  fs::path path = {"some/path.php"};
  FileFacts ff{.types = {{.name = "SomeClass", .kind = TypeKind::Class}}};

  update(m1, "", "1:2:3", {path}, {}, {ff});
  waitForDB(m1, m_exec);

  update(m2, "1:2:3", "1:2:4", {}, {path}, {});
  waitForDB(m2, m_exec);
  EXPECT_EQ(m2.getTypeFile("SomeClass"), nullptr);

  update(m3, "1:2:4", "1:2:4", {}, {}, {});
  EXPECT_EQ(m3.getTypeFile("SomeClass"), nullptr);
}

TEST_F(SymbolMapTest, UpdateOnlyIfCorrectSince) {
  auto& m = make("/var/www");
  update(m, "", "1:2:3", {}, {}, {});
  EXPECT_EQ(m.getClock().m_clock, "1:2:3");

  fs::path path = {"some/path.php"};
  FileFacts ff{.types = {{.name = "SomeClass", .kind = TypeKind::Class}}};

  // since token is "4:5:6" but needs to be "1:2:3". It's not, so
  // `update()` throws and we don't change any data in `m`.
  ASSERT_THROW(update(m, "4:5:6", "4:5:7", {path}, {}, {ff}), UpdateExc);
  EXPECT_EQ(m.getTypeFile("SomeClass"), nullptr);
}

TEST_F(SymbolMapTest, DelayedDBUpdateDoesNotMakeResultsIncorrect) {
  auto& m = make("/var/www");

  fs::path path1 = {"some/path1.php"};
  fs::path path2 = {"some/path2.php"};
  ASSERT_NE(path1, path2);

  FileFacts ff{
      .types = {
          {.name = "SomeClass",
           .kind = TypeKind::Class,
           .base_types = {"BaseClass"}},
          {.name = "BaseClass", .kind = TypeKind::Class}}};

  update(m, "", "1:2:3", {path1, path2}, {}, {ff, ff});
  EXPECT_EQ(m.getClock().m_clock, "1:2:3");

  m.waitForDBUpdate();

  auto& m2 = make("/var/www", m_exec);

  update(m2, "1:2:3", "1:2:4", {}, {path2}, {});
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

  update(m2, "1:2:4", "1:2:5", {}, {path1}, {});
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

  fs::path p = {"some/path1.php"};

  FileFacts ff{
      .types = {
          {.name = "C1", .kind = TypeKind::Class},
          {.name = "I1", .kind = TypeKind::Interface},
          {.name = "T1", .kind = TypeKind::Trait},
          {.name = "E1", .kind = TypeKind::Enum}}};

  update(m1, "", "1:2:3", {p}, {}, {ff});

  EXPECT_EQ(m1.getKind("C1"), TypeKind::Class);
  EXPECT_EQ(m1.getKind("I1"), TypeKind::Interface);
  EXPECT_EQ(m1.getKind("T1"), TypeKind::Trait);
  EXPECT_EQ(m1.getKind("E1"), TypeKind::Enum);
  EXPECT_EQ(m1.getKind("Bogus"), TypeKind::Unknown);

  m1.waitForDBUpdate();

  auto& m2 = make("/var/www");
  update(m2, "1:2:3", "1:2:3", {}, {}, {});

  EXPECT_EQ(m2.getKind("C1"), TypeKind::Class);
  EXPECT_EQ(m2.getKind("I1"), TypeKind::Interface);
  EXPECT_EQ(m2.getKind("T1"), TypeKind::Trait);
  EXPECT_EQ(m2.getKind("E1"), TypeKind::Enum);
  EXPECT_EQ(m2.getKind("Bogus"), TypeKind::Unknown);
}

TEST_F(SymbolMapTest, TypeIsAbstractOrFinal) {
  FileFacts ff{
      .types = {
          {.name = "Abstract",
           .kind = TypeKind::Class,
           .flags = kTypeFlagAbstractBit},
          {.name = "Final",
           .kind = TypeKind::Class,
           .flags = kTypeFlagFinalBit},
          {.name = "AbstractFinal",
           .kind = TypeKind::Trait,
           .flags = kTypeFlagAbstractBit | kTypeFlagFinalBit}}};

  fs::path p = {"some/path1.php"};

  auto& m1 = make("/var/www");
  update(m1, "", "1:2:3", {p}, {}, {ff});

  EXPECT_TRUE(m1.isTypeAbstract("Abstract"));
  EXPECT_FALSE(m1.isTypeFinal("Abstract"));

  EXPECT_FALSE(m1.isTypeAbstract("Final"));
  EXPECT_TRUE(m1.isTypeFinal("Final"));

  EXPECT_TRUE(m1.isTypeAbstract("AbstractFinal"));
  EXPECT_TRUE(m1.isTypeFinal("AbstractFinal"));

  m1.waitForDBUpdate();
  auto& m2 = make("/var/www");
  update(m2, "1:2:3", "1:2:3", {}, {}, {});

  EXPECT_TRUE(m2.isTypeAbstract("Abstract"));
  EXPECT_FALSE(m2.isTypeFinal("Abstract"));

  EXPECT_FALSE(m2.isTypeAbstract("Final"));
  EXPECT_TRUE(m2.isTypeFinal("Final"));

  EXPECT_TRUE(m2.isTypeAbstract("AbstractFinal"));
  EXPECT_TRUE(m2.isTypeFinal("AbstractFinal"));
}

TEST_F(SymbolMapTest, OverwriteKind) {
  auto& m1 = make("/var/www");

  fs::path p1 = {"some/path1.php"};
  fs::path p2 = {"some/path2.php"};
  ASSERT_NE(p1.native(), p2.native());

  FileFacts ff1{.types = {{.name = "Foo", .kind = TypeKind::Class}}};

  FileFacts ff2{.types = {{.name = "Foo", .kind = TypeKind::Interface}}};

  // Duplicate declaration
  update(m1, "", "1:2:3", {p1, p2}, {}, {ff1, ff2});

  // Remove the interface definition, keep the class definition
  update(m1, "1:2:3", "1:2:4", {}, {p2}, {});
  EXPECT_EQ(m1.getKind("Foo"), TypeKind::Class);

  // Re-add the interface definition, remove the class definition
  update(m1, "1:2:4", "1:2:5", {p2}, {p1}, {ff2});
  EXPECT_EQ(m1.getKind("Foo"), TypeKind::Interface);

  // Change the interface definition to a class definition
  update(m1, "1:2:5", "1:2:6", {p2}, {}, {ff1});
  EXPECT_EQ(m1.getKind("Foo"), TypeKind::Class);
}

TEST_F(SymbolMapTest, DeriveKinds) {
  auto& m1 = make("/var/www1");

  fs::path path = {"some/path.php"};
  FileFacts ff{
      .types = {
          {.name = "BaseClass", .kind = TypeKind::Class},
          {.name = "BaseInterface", .kind = TypeKind::Interface},
          {.name = "SomeTrait",
           .kind = TypeKind::Trait,
           .require_extends = {"BaseClass"},
           .require_implements = {"BaseInterface"}}}};

  update(m1, "", "1:2:3", {path}, {}, {ff});
  EXPECT_EQ(m1.getClock().m_clock, "1:2:3");
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

  auto& someTraitFacts = ff.types[2];
  ASSERT_EQ(someTraitFacts.name, "SomeTrait");
  someTraitFacts.require_extends = {};
  someTraitFacts.require_implements = {};
  update(m1, "1:2:3", "1:2:4", {path}, {}, {ff});
}

TEST_F(SymbolMapTest, DeriveKindsRequireClass) {
  auto& m1 = make("/var/www1");

  fs::path path = {"some/path.php"};
  FileFacts ff{
      .types = {
          {.name = "BaseClass", .kind = TypeKind::Class},
          {.name = "SomeTrait",
           .kind = TypeKind::Trait,
           .require_class = {"BaseClass"}}}};

  update(m1, "", "1:2:3", {path}, {}, {ff});
  EXPECT_EQ(m1.getClock().m_clock, "1:2:3");
  EXPECT_TRUE(m1.getBaseTypes("SomeTrait", DeriveKind::Extends).empty());
  EXPECT_TRUE(m1.getBaseTypes("SomeTrait", DeriveKind::RequireExtends).empty());
  EXPECT_THAT(
      m1.getBaseTypes("SomeTrait", DeriveKind::RequireClass),
      ElementsAre("BaseClass"));
  EXPECT_THAT(
      m1.getDerivedTypes("BaseClass", DeriveKind::RequireClass),
      ElementsAre("SomeTrait"));

  auto& someTraitFacts = ff.types[1];
  ASSERT_EQ(someTraitFacts.name, "SomeTrait");
  someTraitFacts.require_class = {};
  update(m1, "1:2:3", "1:2:4", {path}, {}, {ff});
}

TEST_F(SymbolMapTest, MultipleRoots) {
  auto& m1 = make("/var/www1");

  fs::path path = {"some/path.php"};
  FileFacts ff{.types = {{.name = "SomeClass", .kind = TypeKind::Class}}};

  update(m1, "", "1:2:3", {path}, {}, {ff});
  EXPECT_EQ(m1.getClock().m_clock, "1:2:3");
  EXPECT_EQ(m1.getFileTypes(path).at(0).slice(), "SomeClass");
  EXPECT_EQ(m1.getTypeFile("SomeClass"), path.native());

  m1.waitForDBUpdate();

  auto& m2 = make("/var/www2");
  EXPECT_EQ(m2.getClock().m_clock, "");

  update(m2, "", "1:2:3", {path}, {}, {ff});
  EXPECT_EQ(m2.getClock().m_clock, "1:2:3");
  EXPECT_EQ(m2.getFileTypes(path).at(0).slice(), "SomeClass");
  EXPECT_EQ(m2.getTypeFile("SomeClass"), path.native());
}

TEST_F(SymbolMapTest, InterleaveDBUpdates) {
  auto& m1 = make("/var/www", m_exec);
  auto& m2 = make("/var/www", m_exec);

  fs::path path = {"some/path.php"};

  FileFacts ff{.functions = {"some_fn"}};

  update(m1, "", "1:2:3", {path}, {}, {ff});
  EXPECT_EQ(m1.getClock().m_clock, "1:2:3");
  EXPECT_EQ(m1.getFileFunctions(path).at(0).slice(), "some_fn");
  EXPECT_EQ(m1.getFunctionFile("some_fn"), path.native());

  update(m2, "", "1:2:3", {path}, {}, {ff});
  EXPECT_EQ(m2.getClock().m_clock, "1:2:3");
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
      .types = {
          {.name = "SomeClass",
           .kind = TypeKind::Class,
           .base_types = {"BaseClass"}},
          {.name = "BaseClass", .kind = TypeKind::Class}}};

  fs::path path = "some/path1.php";
  update(m, "", "1:2:3", {path}, {}, {ff});

  EXPECT_EQ(
      m.getBaseTypes("SomeClass", DeriveKind::Extends).at(0).slice(),
      "BaseClass");
  EXPECT_EQ(
      m.getDerivedTypes("BaseClass", DeriveKind::Extends).at(0).slice(),
      "SomeClass");

  ff.types.at(0).base_types = {};
  update(m, "1:2:3", "1:2:4", {path}, {}, {ff});

  EXPECT_TRUE(m.getBaseTypes("SomeClass", DeriveKind::Extends).empty());
  EXPECT_TRUE(m.getDerivedTypes("BaseClass", DeriveKind::Extends).empty());
}

TEST_F(SymbolMapTest, DuplicateDefineDerivedType) {
  auto& m = make("/var/www", m_exec);

  FileFacts ff1{
      .types = {
          {.name = "SomeClass",
           .kind = TypeKind::Class,
           .base_types = {"BaseClass"}}}};

  FileFacts ff2{.types = {{.name = "BaseClass", .kind = TypeKind::Class}}};

  fs::path path1 = "some/path1.php";
  fs::path path2 = "some/path2.php";
  fs::path path3 = "some/path3.php";
  ASSERT_NE(path1, path2);
  ASSERT_NE(path1, path3);
  ASSERT_NE(path2, path3);

  // BaseClass has one definition, but SomeClass is duplicate-defined
  update(m, "", "1:2:3", {path1, path2, path3}, {}, {ff1, ff1, ff2});

  // Delete path2, SomeClass now has one definition
  update(m, "1:2:3", "1:2:4", {}, {path2}, {});

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
      .types = {
          {.name = "SomeClass",
           .kind = TypeKind::Class,
           .base_types = {"BaseClass"}},
          {.name = "BaseClass", .kind = TypeKind::Class}}};

  fs::path path1 = {"some/path1.php"};
  fs::path path2 = {"some/path2.php"};
  ASSERT_NE(path1, path2);

  // The repo looks like this at timestamps "1:2:3" and "1:2:4":
  // "1:2:3": {path1: ff}
  // <ff is moved from path1 to path2>
  // "1:2:4": {path2: ff}

  // m1 learns about the repo at timestamps "1:2:3" and "1:2:4", but
  // m2 only knows about the repo at "1:2:4". m2 then updates the DB
  // before m1 succeeds in updating the DB.

  update(m1, "", "1:2:3", {path1}, {}, {ff});
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
  ASSERT_THROW(update(m2, "1:2:3", "1:2:4", {path2}, {path1}, {ff}), UpdateExc);

  // We do a non-incremental update, passing m2 the full repo
  update(m2, "", "1:2:4", {path2}, {}, {ff});
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
  update(m3, "1:2:4", "1:2:4", {}, {}, {});
  EXPECT_EQ(m3.getClock().m_clock, "1:2:4");
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
      .types = {
          {.name = "C1",
           .kind = TypeKind::Class,
           .attributes = {{.name = "A1"}}}}};
  fs::path p1{"p1.php"};

  update(m, "", "1", {p1}, {}, {ffWithAttr});
  EXPECT_THAT(m.getAttributesOfType("C1"), ElementsAre("A1"));
  EXPECT_THAT(m.getTypesWithAttribute("A1"), ElementsAre("C1"));

  FileFacts ffEmpty{};
  FileFacts ffNoAttr{.types = {{.name = "C1", .kind = TypeKind::Class}}};
  fs::path p2{"p2.php"};

  update(m, "1", "2", {p1, p2}, {}, {ffEmpty, ffNoAttr});
  EXPECT_THAT(m.getAttributesOfType("C1"), IsEmpty());
  EXPECT_THAT(m.getTypesWithAttribute("A1"), IsEmpty());
}

TEST_F(SymbolMapTest, RemovePathFromExistingFile) {
  auto& m = make("/var/www");

  FileFacts ff{.types = {{.name = "SomeClass", .kind = TypeKind::Class}}};

  FileFacts emptyFF{};

  fs::path path1 = {"some/path1.php"};

  update(m, "", "1:2:3", {path1}, {}, {ff});
  EXPECT_EQ(m.getTypeFile("SomeClass"), path1.native());
  update(m, "1:2:3", "1:2:4", {path1}, {}, {emptyFF});
  EXPECT_EQ(m.getTypeFile("SomeClass"), nullptr);
}

TEST_F(SymbolMapTest, MoveAndCopySymbol) {
  auto& m1 = make("/var/www", m_exec);
  auto& m2 = make("/var/www", m_exec);

  FileFacts ff{.types = {{.name = "SomeClass", .kind = TypeKind::Class}}};

  FileFacts emptyFF{};

  fs::path path1 = {"some/path1.php"};
  fs::path path2 = {"some/path2.php"};
  fs::path path3 = {"some/path3.php"};

  // Initialize m2 as dependent on the DB
  update(m1, "", "1:2:3", {path1}, {}, {ff});
  waitForDB(m1, m_exec);
  update(m2, "1:2:3", "1:2:3", {}, {}, {});
  ASSERT_EQ(m2.getTypeFile("SomeClass"), path1.native());

  // Delete SomeClass from path1, simultaneously duplicate-define it in path2
  // and path3
  update(m2, "1:2:3", "1:2:4", {path1, path2, path3}, {}, {emptyFF, ff, ff});

  // Delete path3.
  update(m2, "1:2:4", "1:2:5", {}, {path3}, {});
  // SomeClass should only be defined in path2 now.
  EXPECT_EQ(m2.getTypeFile("SomeClass"), path2.native());
}

TEST_F(SymbolMapTest, AttrQueriesDoNotConfuseTypeAndTypeAlias) {
  auto& m1 = make("/var/www", m_exec);
  fs::path p = "foo.php";
  FileFacts ffTypeAliasWithAttr{
      .types = {
          {.name = "SomeClass",
           .kind = TypeKind::Class,
           .attributes = {{.name = "ClassAttr"}}},
          {.name = "SomeTypeAlias",
           .kind = TypeKind::TypeAlias,
           .attributes = {{.name = "AliasAttr"}}}}};
  update(m1, "", "1", {p}, {}, {ffTypeAliasWithAttr});

  EXPECT_THAT(m1.getAttributesOfType("SomeClass"), ElementsAre("ClassAttr"));
  // Empty because we ask for a type alias, not a type
  EXPECT_THAT(m1.getAttributesOfTypeAlias("SomeClass"), ElementsAre());

  EXPECT_THAT(
      m1.getAttributesOfTypeAlias("SomeTypeAlias"), ElementsAre("AliasAttr"));
  // Empty because we ask for a type, not a type alias
  EXPECT_THAT(m1.getAttributesOfType("SomeTypeAlias"), ElementsAre());

  // Create a second map and fill it from the DB
  waitForDB(m1, m_exec);
  auto& m2 = make("/var/www", m_exec);
  update(m2, "1", "1", {}, {}, {});

  EXPECT_THAT(m2.getAttributesOfType("SomeClass"), ElementsAre("ClassAttr"));
  // Empty because we ask for a type alias, not a type
  EXPECT_THAT(m2.getAttributesOfTypeAlias("SomeClass"), ElementsAre());

  // Empty because we ask for a type, not a type alias
  EXPECT_THAT(m2.getAttributesOfType("SomeTypeAlias"), ElementsAre());
  EXPECT_THAT(
      m2.getAttributesOfTypeAlias("SomeTypeAlias"), ElementsAre("AliasAttr"));
}

TEST_F(SymbolMapTest, TwoFilesDisagreeOnBaseTypes) {
  auto& m = make("/var/www", m_exec);

  FileFacts ffSomeClassDerivesBaseClass{
      .types = {
          {.name = "SomeClass",
           .kind = TypeKind::Class,
           .base_types = {"BaseClass"}}}};

  FileFacts ffBaseClass{
      .types = {{.name = "BaseClass", .kind = TypeKind::Class}}};

  fs::path pSomeClass1 = "src/SomeClass1.php";
  fs::path pSomeClass2 = "src/SomeClass2.php";
  ASSERT_NE(pSomeClass1, pSomeClass2);
  fs::path pBaseClass = "src/BaseClass.php";
  ASSERT_NE(pBaseClass, pSomeClass1);
  ASSERT_NE(pBaseClass, pSomeClass2);

  // SomeClass is defined in two files, both of which agree that SomeClass
  // extends BaseClass
  update(
      m,
      "",
      "1:2:3",
      {pSomeClass1, pSomeClass2, pBaseClass},
      {},
      {ffSomeClassDerivesBaseClass, ffSomeClassDerivesBaseClass, ffBaseClass});

  // A collection of assertions that should hold whenever types are not
  // duplicate-defined
  auto expectAlways = [&]() {
    EXPECT_EQ(m.getTypeFile("BaseClass"), pBaseClass.native());
    EXPECT_TRUE(m.getBaseTypes("BaseClass", DeriveKind::Extends).empty());
    EXPECT_TRUE(m.getDerivedTypes("SomeClass", DeriveKind::Extends).empty());
  };

  FileFacts ffSomeClassDerivesNobody{
      .types = {{.name = "SomeClass", .kind = TypeKind::Class}}};

  // pSomeClass1 now believes that SomeClass doesn't extend BaseClass
  update(m, "1:2:3", "1:2:4", {pSomeClass1}, {}, {ffSomeClassDerivesBaseClass});

  // Remove pSomeClass1 so pSomeClass2 is the source of truth for SomeClass
  update(m, "1:2:4", "1:2:5", {}, {pSomeClass1}, {});
  expectAlways();

  EXPECT_EQ(m.getTypeFile("SomeClass"), pSomeClass2.native());
  EXPECT_THAT(
      m.getBaseTypes("SomeClass", DeriveKind::Extends),
      ElementsAre("BaseClass"));
  EXPECT_THAT(
      m.getDerivedTypes("BaseClass", DeriveKind::Extends),
      ElementsAre("SomeClass"));

  // Add pSomeClass1 back in
  update(m, "1:2:5", "1:2:6", {pSomeClass1}, {}, {ffSomeClassDerivesNobody});

  // Now remove pSomeClass2, so that pSomeClass1 is the source of truth for
  // SomeClass
  update(m, "1:2:6", "1:2:7", {}, {pSomeClass2}, {});
  expectAlways();

  EXPECT_EQ(m.getTypeFile("SomeClass"), pSomeClass1.native());
  EXPECT_TRUE(m.getBaseTypes("SomeClass", DeriveKind::Extends).empty());
  EXPECT_TRUE(m.getDerivedTypes("BaseClass", DeriveKind::Extends).empty());
}

TEST_F(SymbolMapTest, MemoryAndDBDisagreeOnFileHash) {
  auto& m1 = make("/var/www", m_exec);
  auto& m2 = make("/var/www", m_exec);

  FileFacts ff{.types = {{.name = "SomeClass", .kind = TypeKind::Class}}};

  fs::path path1 = {"some/path1.php"};

  update(m1, "", "1:2:3", {path1}, {}, {ff});
  waitForDB(m1, m_exec);
  auto oldHash = getSha1Hex(ff);
  EXPECT_EQ(m1.getAllPathsWithHashes().at(Path{path1}).toString(), oldHash);

  ff.types[0].name = "OtherClass";
  auto newHash = getSha1Hex(ff);
  ASSERT_NE(oldHash, newHash);

  update(m2, "1:2:3", "1:2:4", {path1}, {}, {ff});
  EXPECT_EQ(m2.getAllPathsWithHashes().at(Path{path1}).toString(), newHash);
}

TEST_F(SymbolMapTest, PartiallyFillDerivedTypeInfo) {
  auto& m1 = make("/var/www", m_exec);

  FileFacts ff1{
      .types = {
          {.name = "SomeClass",
           .kind = TypeKind::Class,
           .base_types = {"BaseClass"}}}};

  FileFacts ff2{
      .types = {
          {.name = "OtherClass",
           .kind = TypeKind::Class,
           .base_types = {"BaseClass"}}}};

  FileFacts ff3{.types = {{.name = "BaseClass", .kind = TypeKind::Interface}}};

  fs::path p1 = "some/path1.php";
  fs::path p2 = "some/path2.php";
  fs::path p3 = "some/path3.php";

  update(m1, "", "1:2:3", {p1, p2, p3}, {}, {ff1, ff2, ff3});
  waitForDB(m1, m_exec);

  auto& m2 = make("/var/www", m_exec);
  update(m2, "1:2:3", "1:2:3", {}, {}, {});

  // Fetch the supertypes of SomeClass from the DB
  EXPECT_THAT(
      m2.getBaseTypes("SomeClass", DeriveKind::Extends),
      ElementsAre("BaseClass"));

  // Now query the subtypes of BaseClass. We should remember that SomeClass is
  // a subtype of BaseClass, but we should also fetch information from the DB
  // about OtherClass being a subtype of BaseClass.
  EXPECT_THAT(
      m2.getDerivedTypes("BaseClass", DeriveKind::Extends),
      UnorderedElementsAre("SomeClass", "OtherClass"));
}

TEST_F(SymbolMapTest, BaseTypesWithDifferentCases) {
  auto& m1 = make("/var/www", m_exec);

  FileFacts ff1{
      .types = {
          {.name = "SomeClass",
           .kind = TypeKind::Class,
           .base_types = {"BaseClass"}}}};

  FileFacts ff2{
      .types = {
          {.name = "SomeClass",
           .kind = TypeKind::Class,
           .base_types = {"baseclass"}}}};

  FileFacts ff3{.types = {{.name = "BaseClass", .kind = TypeKind::Class}}};

  FileFacts ff4{.types = {{.name = "baseclass", .kind = TypeKind::Class}}};

  fs::path p1 = "some/path1.php";
  fs::path p2 = "some/path2.php";
  fs::path p3 = "some/path3.php";
  fs::path p4 = "some/path4.php";

  // Define both "baseclass" and "BaseClass"
  update(m1, "", "1", {p1, p2, p3, p4}, {}, {ff1, ff2, ff3, ff4});
  waitForDB(m1, m_exec);
  auto& m2 = make("/var/www");
  update(m2, "1", "1", {}, {}, {});

  // Remove references to "baseclass", keep references to "BaseClass"
  auto check = [&](auto& map) {
    EXPECT_EQ(map.getTypeFile("SomeClass"), p1.native());
    EXPECT_THAT(
        map.getBaseTypes("SomeClass", DeriveKind::Extends),
        ElementsAre("BaseClass"));
    EXPECT_THAT(
        map.getDerivedTypes("BaseClass", DeriveKind::Extends),
        ElementsAre("SomeClass"));
    EXPECT_THAT(
        map.getDerivedTypes("baseclass", DeriveKind::Extends),
        ElementsAre("SomeClass"));
  };
  update(m1, "1", "2", {}, {p2, p4}, {});
  check(m1);
  update(m2, "1", "2", {}, {p2, p4}, {});
  check(m2);
}

TEST_F(SymbolMapTest, DerivedTypesWithDifferentCases) {
  auto& m = make("/var/www", m_exec);

  FileFacts ff1{
      .types = {
          {.name = "SomeClass",
           .kind = TypeKind::Class,
           .base_types = {"BaseClass"}},
          {.name = "BaseClass", .kind = TypeKind::Class}}};

  fs::path p1 = "some/path1.php";

  update(m, "", "1", {p1}, {}, {ff1});
  EXPECT_EQ(m.getTypeFile("SomeClass").slice(), p1.native());
  EXPECT_THAT(
      m.getBaseTypes("SomeClass", DeriveKind::Extends),
      ElementsAre("BaseClass"));
  EXPECT_THAT(
      m.getDerivedTypes("BaseClass", DeriveKind::Extends),
      ElementsAre("SomeClass"));

  // Replace "SomeClass" with "SOMECLASS"
  FileFacts ff2{
      .types = {
          {.name = "SOMECLASS",
           .kind = TypeKind::Class,
           .base_types = {"BaseClass"}},
          {.name = "BaseClass", .kind = TypeKind::Class}}};
  update(m, "1", "2", {p1}, {}, {ff2});

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
      .types = {
          {.name = "SomeClass", .kind = TypeKind::Class},
          {.name = "OtherClass", .kind = TypeKind::Class}}};

  fs::path path = {"some/path1.php"};

  auto testMap = [&path](auto& map) {
    // getTypeFile() and getFileTypes() both fill in-memory maps from the DB.
    // Make sure we never think we know all the types in a given path when we
    // really only know some of them.
    EXPECT_EQ(map.getTypeFile("SomeClass"), path.native());
    EXPECT_THAT(
        map.getFileTypes(path.native()),
        UnorderedElementsAre("SomeClass", "OtherClass"));
    EXPECT_EQ(map.getTypeFile("OtherClass"), path.native());
  };

  update(m1, "", "1:2:3", {path}, {}, {ff});
  testMap(m1);
  waitForDB(m1, m_exec);

  auto& m2 = make("/var/www");
  update(m2, "1:2:3", "1:2:3", {}, {}, {});
  testMap(m2);
}

TEST_F(SymbolMapTest, ErasePathStoredInDB) {
  auto& m1 = make("/var/www", m_exec);

  FileFacts ff{.types = {{.name = "SomeClass", .kind = TypeKind::Class}}};

  fs::path p = {"some/path.php"};

  update(m1, "", "1:2:3", {p}, {}, {ff});
  EXPECT_EQ(m1.getTypeFile("SomeClass"), p.native());
  waitForDB(m1, m_exec);

  auto& m2 = make("/var/www", m_exec);
  update(m2, "1:2:3", "1:2:4", {}, {p}, {});
  update(m2, "1:2:4", "1:2:5", {p}, {}, {ff});
  EXPECT_EQ(m2.getTypeFile("SomeClass"), p.native());
}

TEST_F(SymbolMapTest, GetTypesAndTypeAliasesWithAttribute) {
  auto& m1 = make("/var/www", m_exec);

  FileFacts ff{
      .types = {
          {.name = "SomeClass",
           .kind = TypeKind::Class,
           .attributes =
               {{.name = "Foo", .args = {"apple", "38"}},
                {.name = "Bar", .args = {""}},
                {.name = "Baz", .args = {}}}},
          {.name = "OtherClass",
           .kind = TypeKind::Class,
           .attributes = {{.name = "Bar"}}},
          {.name = "SomeTypeAlias",
           .kind = TypeKind::TypeAlias,
           .attributes = {{.name = "Foo", .args = {"42", "a"}}}}}};

  fs::path p = {"some/path.php"};

  auto testMap = [&p](auto& map) {
    EXPECT_EQ(map.getTypeFile("SomeClass"), p.native());
    EXPECT_THAT(
        map.getTypesWithAttribute("Foo"), UnorderedElementsAre("SomeClass"));
    EXPECT_THAT(
        map.getTypeAliasesWithAttribute("Foo"),
        UnorderedElementsAre("SomeTypeAlias"));
    EXPECT_THAT(
        map.getTypesWithAttribute("Bar"),
        UnorderedElementsAre("SomeClass", "OtherClass"));
    EXPECT_THAT(
        map.getAttributesOfType("SomeClass"),
        UnorderedElementsAre("Bar", "Baz", "Foo"));
    EXPECT_THAT(
        map.getTypeAliasAttributeArgs("SomeTypeAlias", "Foo"),
        ElementsAre("42", "a"));
    EXPECT_THAT(
        map.getTypeAttributeArgs("SomeClass", "Foo"),
        ElementsAre("apple", "38"));
    EXPECT_THAT(map.getTypeAttributeArgs("SomeClass", "Bar"), ElementsAre(""));
    EXPECT_THAT(map.getTypeAttributeArgs("SomeClass", "Baz"), IsEmpty());
    EXPECT_THAT(
        map.getAttributesOfTypeAlias("SomeTypeAlias"), ElementsAre("Foo"));
  };

  update(m1, "", "1:2:3", {p}, {}, {ff});
  testMap(m1);
  waitForDB(m1, m_exec);

  auto& m2 = make("/var/www", m_exec);
  update(m2, "1:2:3", "1:2:3", {}, {}, {});
  testMap(m2);
}

TEST_F(SymbolMapTest, GetMethodsWithAttribute) {
  auto& m1 = make("/var/www");

  FileFacts ff1{
      .types = {
          {.name = "C1",
           .methods =
               {MethodFacts{
                    .name = "m1",
                    .attributes = {{.name = "A1", .args = {"1"}}}},
                MethodFacts{
                    .name = "m2",
                    .attributes = {{.name = "A1", .args = {"2"}}},
                }}},
      }};
  fs::path p1{"some/path1.php"};
  update(m1, "", "1", {p1}, {}, {ff1});

  auto testMap = [&p1](auto& m) {
    auto methods = m.getMethodsWithAttribute("A1");
    EXPECT_THAT(
        methods,
        AllOf(
            SizeIs(2),
            Contains(MethodDecl{
                .m_type =
                    {.m_name = Symbol<SymKind::Type>{StringData{"C1"}},
                     .m_path = Path{p1}},
                .m_method = Symbol<SymKind::Function>{StringData{"m1"}}})));

    EXPECT_THAT(m.getAttributesOfMethod("C1", "m1"), ElementsAre("A1"));
    EXPECT_THAT(m.getMethodAttributeArgs("C1", "m1", "A1"), ElementsAre("1"));
    EXPECT_THAT(m.getAttributesOfMethod("C1", "m2"), ElementsAre("A1"));
    EXPECT_THAT(m.getMethodAttributeArgs("C1", "m2", "A1"), ElementsAre("2"));
  };
  testMap(m1);

  m1.waitForDBUpdate();
  auto& m2 = make("/var/www");
  update(m2, "1", "1", {}, {}, {});
  testMap(m2);
}

TEST_F(SymbolMapTest, GetAttributesOfRenamedMethod) {
  auto& m1 = make("/var/www");

  // Create method `C1::m1`
  FileFacts ff1{
      .types = {
          {.name = "C1",
           .methods = {
               MethodFacts{
                   .name = "m1", .attributes = {{.name = "A1", .args = {"1"}}}},
           }}}};
  fs::path p1{"some/path1.php"};
  update(m1, "", "1", {p1}, {}, {ff1});

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
      .types = {
          {.name = "C1",
           .methods = {
               MethodFacts{
                   .name = "m2", .attributes = {{.name = "A1", .args = {"1"}}}},
           }}}};

  update(m2, "1", "2", {p1}, {}, {ff2});

  {
    auto methods = m2.getMethodsWithAttribute("A1");
    ASSERT_EQ(methods.size(), 1);
    EXPECT_EQ(methods[0].m_type.m_name.slice(), "C1");
    EXPECT_EQ(methods[0].m_method.m_name.slice(), "m2");
  }
}

TEST_F(SymbolMapTest, OnlyIndexCertainMethodAttrs) {
  // Index A2 but not A1
  auto& m1 = make("/var/www", nullptr, {"A2"});

  // Create method `C1::m1`
  FileFacts ff1{
      .types = {
          {.name = "C1",
           .methods = {
               MethodFacts{
                   .name = "m1",
                   .attributes = {{.name = "A1"}, {.name = "A2"}}},
           }}}};
  fs::path p1{"some/path1.php"};

  auto check = [&](SymbolMap& m) {
    auto attrs = m.getAttributesOfMethod("C1", "m1");
    ASSERT_EQ(attrs.size(), 1);
    EXPECT_EQ(attrs[0].m_name, "A2");

    auto a1Methods = m.getMethodsWithAttribute("A1");
    EXPECT_TRUE(a1Methods.empty());

    auto a2Methods = m.getMethodsWithAttribute("A2");
    ASSERT_EQ(a2Methods.size(), 1);
    EXPECT_EQ(a2Methods[0].m_type.m_name, "C1");
    EXPECT_EQ(a2Methods[0].m_method, "m1");
  };

  update(m1, "", "1", {p1}, {}, {ff1});
  check(m1);
  m1.waitForDBUpdate();

  auto& m2 = make("/var/www", nullptr, {"A2"});
  update(m2, "1", "1", {}, {}, {});
  check(m2);
}

TEST_F(SymbolMapTest, GetFilesWithAttribute) {
  auto& m1 = make("/var/www");

  FileFacts ff1{
      .file_attributes = {
          {.name = "A1", .args = {"1"}}, {.name = "A2", .args = {}}}};
  fs::path p1{"some/path1.php"};
  update(m1, "", "1", {p1}, {}, {ff1});

  auto testMap = [&p1](auto& m) {
    auto a1files = m.getFilesWithAttribute("A1");
    EXPECT_THAT(a1files, ElementsAre(p1.native()));

    auto a2files = m.getFilesWithAttribute("A2");
    EXPECT_THAT(a2files, ElementsAre(p1.native()));

    auto a1valfiles = m.getFilesWithAttributeAndAnyValue("A1", "1");
    EXPECT_THAT(a1valfiles, ElementsAre(p1.native()));

    auto a2valfiles = m.getFilesWithAttributeAndAnyValue("A2", "1");
    EXPECT_THAT(a2valfiles, ElementsAre());

    auto attrs = m.getAttributesOfFile(Path{p1});
    EXPECT_THAT(attrs, UnorderedElementsAre("A1", "A2"));

    auto a1args = m.getFileAttributeArgs(Path{p1}, "A1");
    EXPECT_THAT(a1args, ElementsAre("1"));

    auto a2args = m.getFileAttributeArgs(Path{p1}, "A2");
    EXPECT_THAT(a2args, ElementsAre());
  };
  testMap(m1);

  m1.waitForDBUpdate();
  auto& m2 = make("/var/www");
  update(m2, "1", "1", {}, {}, {});
  testMap(m2);
}

TEST_F(SymbolMapTest, ConcurrentFillsFromDB) {
  auto& dbUpdater = make("/var/www", m_exec);
  auto& map = make("/var/www");

  auto makeSym = [](size_t i) -> std::string {
    return folly::sformat("Class{}", i);
  };

  auto makePath = [](size_t i) -> fs::path {
    return folly::sformat("some/path{}.php", i);
  };

  size_t const numSymbols = 100;

  std::vector<fs::path> paths;
  std::vector<FileFacts> facts;
  for (auto i = 0; i < numSymbols; ++i) {
    rust::Vec<rust::String> baseTypes;
    baseTypes.reserve(i);
    for (auto j = 0; j < i; ++j) {
      baseTypes.push_back(makeSym(j));
    }
    FileFacts ff{
        .types = {
            {.name = makeSym(i),
             .kind = TypeKind::Class,
             .base_types = std::move(baseTypes)}}};

    paths.push_back(makePath(i));
    facts.push_back(std::move(ff));
  }

  // Update the DB with all path information
  update(dbUpdater, "", "1:2:3", std::move(paths), {}, std::move(facts));
  waitForDB(dbUpdater, m_exec);

  update(map, "1:2:3", "1:2:3", {}, {}, {});

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
