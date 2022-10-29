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

#include <filesystem>
#include <memory>
#include <sstream>
#include <string_view>
#include <thread>

#include <folly/Likely.h>
#include <folly/Synchronized.h>
#include <folly/Unit.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/futures/Future.h>
#include <folly/futures/FutureSplitter.h>
#include <folly/logging/xlog.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/request-injection-data.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/watchman.h"
#include "hphp/runtime/ext/facts/exception.h"
#include "hphp/runtime/ext/facts/fact-extractor.h"
#include "hphp/runtime/ext/facts/facts-store.h"
#include "hphp/runtime/ext/facts/file-facts.h"
#include "hphp/runtime/ext/facts/path-and-hash.h"
#include "hphp/runtime/ext/facts/string-ptr.h"
#include "hphp/runtime/ext/facts/symbol-map.h"
#include "hphp/runtime/ext/facts/symbol-types.h"
#include "hphp/runtime/ext/facts/thread-factory.h"
#include "hphp/util/hash-set.h"
#include "hphp/util/logger.h"
#include "hphp/util/sha1.h"

namespace fs = std::filesystem;

namespace HPHP {
namespace Facts {
namespace {

/**
 * Return a path relative to `root` with the same canonical location as `p`. If
 * `p` does not exist within `root`, return `std::nullopt`.
 */
Optional<fs::path> resolvePathRelativeToRoot(
    const fs::path& path,
    const fs::path& root) {
  if (path.is_relative())
    return path;
  if (!fs::exists(path))
    return std::nullopt;
  return fs::relative(path, root);
}

/**
 * Cancel the request timeout when created, restart the request timeout when
 * destroyed.
 */
struct TimeoutSuspender {
  TimeoutSuspender() : m_timeoutSeconds{RID().getTimeout()} {
    RID().setTimeout(0);
  }
  ~TimeoutSuspender() {
    RID().setTimeout(m_timeoutSeconds);
  }
  int m_timeoutSeconds{0};

  TimeoutSuspender(const TimeoutSuspender&) = delete;
  TimeoutSuspender& operator=(const TimeoutSuspender&) = delete;
  TimeoutSuspender(TimeoutSuspender&&) = delete;
  TimeoutSuspender& operator=(TimeoutSuspender&&) = delete;
};

constexpr std::string_view kKindFilterKey{"kind"};
constexpr std::string_view kDeriveKindFilterKey{"derive_kind"};
constexpr std::string_view kAttributeFilterKey{"attributes"};
constexpr std::string_view kExtendsFilterKey{"extends"};
constexpr std::string_view kRequiresFilterKey{"require extends"};
constexpr std::string_view kAttributeNameKey{"name"};
constexpr std::string_view kAttributeParamsKey{"parameters"};

struct KindFilterData {
  bool m_removeClasses = false;
  bool m_removeEnums = false;
  bool m_removeInterfaces = false;
  bool m_removeTraits = false;

  TypeKindMask toMask() const {
    auto mask = kTypeKindAll;
    if (m_removeClasses) {
      mask &= ~static_cast<int>(TypeKind::Class);
    }
    if (m_removeEnums) {
      mask &= ~static_cast<int>(TypeKind::Enum);
    }
    if (m_removeInterfaces) {
      mask &= ~static_cast<int>(TypeKind::Interface);
    }
    if (m_removeTraits) {
      mask &= ~static_cast<int>(TypeKind::Trait);
    }
    return mask;
  }

  constexpr bool doesIncludeEverything() const noexcept {
    return !(
        m_removeClasses || m_removeEnums || m_removeInterfaces ||
        m_removeTraits);
  }

  static constexpr KindFilterData includeEverything() noexcept {
    return {
        .m_removeClasses = false,
        .m_removeEnums = false,
        .m_removeInterfaces = false,
        .m_removeTraits = false};
  }

  static constexpr KindFilterData removeEverything() noexcept {
    return {
        .m_removeClasses = true,
        .m_removeEnums = true,
        .m_removeInterfaces = true,
        .m_removeTraits = true};
  }

  static KindFilterData createFromKeyset(const ArrayData* kindFilter) {
    if (kindFilter == nullptr) {
      // We weren't presented with a kindFilter, so don't remove any kinds
      return KindFilterData::includeEverything();
    }
    // Remove the kinds that aren't mentioned in the filter
    auto data = KindFilterData::removeEverything();
    IterateKV(kindFilter, [&](TypedValue k, TypedValue UNUSED v) {
      if (!tvIsString(k)) {
        return;
      }
      StringPtr kind{k.m_data.pstr};
      if (kind == kTypeKindClass) {
        data.m_removeClasses = false;
      } else if (kind == kTypeKindEnum) {
        data.m_removeEnums = false;
      } else if (kind == kTypeKindInterface) {
        data.m_removeInterfaces = false;
      } else if (kind == kTypeKindTrait) {
        data.m_removeTraits = false;
      }
    });
    return data;
  }
};

struct AttributeFilterData {
  /**
   * Map from 0-indexed argument position to argument value
   */
  using ArgMap = hphp_hash_map<size_t, folly::dynamic>;

  hphp_hash_map<Symbol<SymKind::Type>, ArgMap> m_attrs;

  static AttributeFilterData createFromShapes(const ArrayData* attrFilters) {
    assertx(attrFilters);
    AttributeFilterData filters;
    IterateV(attrFilters, [&](TypedValue v) {
      if (!tvIsArrayLike(v)) {
        return;
      }
      filters.m_attrs.insert(createAttrFilterFromShape(v.m_data.parr));
    });
    return filters;
  }

 private:
  static std::pair<StringPtr, ArgMap> createAttrFilterFromShape(
      const ArrayData* attrShape) {
    assertx(attrShape);
    StringPtr name{nullptr};
    hphp_hash_map<size_t, folly::dynamic> args;
    IterateKV(attrShape, [&](TypedValue k, TypedValue v) {
      if (!tvIsString(k)) {
        return;
      }
      auto const kStr = StringPtr{k.m_data.pstr};
      if (kStr == kAttributeNameKey) {
        if (tvIsString(v)) {
          name = StringPtr{v.m_data.pstr};
        } else if (tvIsLazyClass(v)) {
          name = StringPtr{v.m_data.plazyclass.name()};
        } else if (tvIsClass(v)) {
          name = StringPtr{v.m_data.pclass->name()};
        } else {
          return;
        }
      } else if (kStr == kAttributeParamsKey) {
        if (!tvIsArrayLike(v)) {
          return;
        }
        IterateKV(v.m_data.parr, [&](TypedValue k, TypedValue v) {
          if (!tvIsInt(k)) {
            return;
          }
          args.insert({tvAssertInt(k), createDynamicFromTv(v)});
        });
      }
    });
    return {name, std::move(args)};
  }

  static folly::dynamic createDynamicFromTv(TypedValue v) {
    folly::dynamic result{nullptr};
    if (tvIsBool(v)) {
      result = static_cast<bool>(v.m_data.num);
    } else if (tvIsInt(v)) {
      result = v.m_data.num;
    } else if (tvIsDouble(v)) {
      result = v.m_data.dbl;
    } else if (tvIsString(v)) {
      result = v.m_data.pstr->slice();
    } else if (tvIsVec(v)) {
      IterateV(v.m_data.parr, [&result](TypedValue v) {
        result.push_back(createDynamicFromTv(v));
      });
    } else if (tvIsArrayLike(v)) {
      IterateKV(v.m_data.parr, [&result](TypedValue k, TypedValue v) {
        result[createDynamicFromTv(k)] = createDynamicFromTv(v);
      });
    }
    return result;
  }
};

struct DeriveKindFilterData {
  static constexpr DeriveKindFilterData includeEverything() noexcept {
    return {.m_removeExtends = false, .m_removeRequires = false};
  }

  static constexpr DeriveKindFilterData removeEverything() noexcept {
    return {.m_removeExtends = true, .m_removeRequires = true};
  }

  static DeriveKindFilterData createFromKeyset(
      const ArrayData* deriveKindFilter) {
    DeriveKindFilterData filters = DeriveKindFilterData::removeEverything();
    IterateKV(deriveKindFilter, [&](TypedValue k, UNUSED TypedValue v) {
      if (!tvIsString(k)) {
        return;
      }
      StringPtr key{k.m_data.pstr};
      if (key == kExtendsFilterKey) {
        filters.m_removeExtends = false;
      } else if (key == kRequiresFilterKey) {
        filters.m_removeRequires = false;
      }
    });
    return filters;
  }

  DeriveKindMask toMask() const {
    auto mask = kDeriveKindAll;
    if (m_removeExtends) {
      mask &= ~static_cast<int>(DeriveKind::Extends);
    }
    if (m_removeRequires) {
      mask &= ~static_cast<int>(DeriveKind::RequireClass);
      mask &= ~static_cast<int>(DeriveKind::RequireExtends);
      mask &= ~static_cast<int>(DeriveKind::RequireImplements);
    }
    return mask;
  }

  bool m_removeExtends = false;
  bool m_removeRequires = false;
};

struct InheritanceFilterData {
  KindFilterData m_kindFilters;
  DeriveKindFilterData m_deriveKindFilters;
  AttributeFilterData m_attrFilters;

  static InheritanceFilterData includeEverything() noexcept {
    return {
        .m_kindFilters = KindFilterData::includeEverything(),
        .m_deriveKindFilters = DeriveKindFilterData::includeEverything()};
  }

  static InheritanceFilterData removeEverything() noexcept {
    return {
        .m_kindFilters = KindFilterData::removeEverything(),
        .m_deriveKindFilters = DeriveKindFilterData::removeEverything()};
  }

  static InheritanceFilterData createFromShape(const ArrayData* filters) {
    if (filters == nullptr) {
      return InheritanceFilterData::includeEverything();
    }
    // Default to including everything. If a keyset is provided, include only
    // the members of that keyset.
    InheritanceFilterData inheritanceFilters =
        InheritanceFilterData::includeEverything();
    IterateKV(filters, [&](TypedValue k, TypedValue v) {
      if (!tvIsString(k)) {
        return;
      }
      StringPtr key{k.m_data.pstr};

      if (key == kKindFilterKey) {
        if (tvIsArrayLike(v)) {
          inheritanceFilters.m_kindFilters =
              KindFilterData::createFromKeyset(v.m_data.parr);
        }
      } else if (key == kDeriveKindFilterKey) {
        if (tvIsArrayLike(v)) {
          inheritanceFilters.m_deriveKindFilters =
              DeriveKindFilterData::createFromKeyset(v.m_data.parr);
        }
      } else if (key == kAttributeFilterKey) {
        if (tvIsArrayLike(v)) {
          inheritanceFilters.m_attrFilters =
              AttributeFilterData::createFromShapes(v.m_data.parr);
        }
      }
    });
    return inheritanceFilters;
  }
};

// SHA1 hash of an empty file
constexpr folly::StringPiece kEmptyFileSha1Hash =
    "da39a3ee5e6b4b0d3255bfef95601890afd80709";

std::string curthread() {
  std::stringstream s;
  s << std::this_thread::get_id();
  return s.str();
}

std::vector<fs::path> removeHashes(
    std::vector<PathAndOptionalHash>&& pathsWithHashes) {
  std::vector<fs::path> paths;
  paths.reserve(pathsWithHashes.size());
  for (auto&& pathAndHash : std::move(pathsWithHashes)) {
    paths.push_back(std::move(pathAndHash.m_path));
  }
  return paths;
}

std::string json_from_facts(const FileFacts& facts) {
  auto types = folly::dynamic::array();
  for (auto const& type : facts.m_types) {
    types.push_back(type.m_name);
  }
  // clang-format off
  return folly::toJson(folly::dynamic::object
    ("types", std::move(types))
    ("functions",
     folly::dynamic(facts.m_functions.begin(), facts.m_functions.end()))
    ("constants",
     folly::dynamic(facts.m_constants.begin(), facts.m_constants.end())));
  // clang-format on
}

/**
 * Convert a C++ StringData structure into a Hack `vec<string>`.
 */
template <typename StringPtrIterable>
Array makeVecOfString(StringPtrIterable&& vector) {
  auto ret = VecInit{vector.size()};
  for (auto&& str : std::forward<StringPtrIterable>(vector)) {
    ret.append(VarNR{StrNR{str.get()}}.tv());
  }
  return ret.toArray();
}

/**
 * Convert a C++ StringData structure into a Hack `vec<(string, string)>`.
 */
Array makeVecOfStringString(const std::vector<MethodDecl>& vector) {
  auto ret = VecInit{vector.size()};
  for (auto const& [typeDecl, method] : vector) {
    auto stringStringTuple = VecInit{2};
    stringStringTuple.append(
        make_tv<KindOfPersistentString>(typeDecl.m_name.get()));
    stringStringTuple.append(make_tv<KindOfPersistentString>(method.get()));

    ret.append(stringStringTuple.toArray());
  }
  return ret.toArray();
}

TypedValue tvFromDynamic(folly::dynamic&& dy) {
  if (dy.isBool()) {
    return make_tv<KindOfBoolean>(std::move(dy).getBool());
  }
  if (dy.isInt()) {
    return make_tv<KindOfInt64>(std::move(dy).getInt());
  }
  if (dy.isDouble()) {
    return make_tv<KindOfDouble>(std::move(dy).getInt());
  }
  if (dy.isString()) {
    return make_tv<KindOfPersistentString>(
        makeStaticString(std::move(dy).getString()));
  }
  if (dy.isArray()) {
    VecInit ret{dy.size()};
    for (auto&& v : std::move(dy)) {
      ret.append(tvFromDynamic(std::move(v)));
    }
  }
  if (dy.isObject()) {
    DictInit ret{dy.size()};
    for (auto [k, v] : dy.items()) {
      if (k.isInt()) {
        ret.set(k.getInt(), tvFromDynamic(std::move(v)));
      } else if (k.isString()) {
        ret.set(k.getString(), tvFromDynamic(std::move(v)));
      }
    }
  }
  return make_tv<KindOfNull>();
}

/**
 * Convert a C++ folly::dynamic structure into a Hack `vec<dynamic>`.
 */
template <typename DynamicIterable>
Array makeVecOfDynamic(DynamicIterable&& vector) {
  auto ret = VecInit{vector.size()};
  for (auto&& dy : std::forward<DynamicIterable>(vector)) {
    ret.append(tvFromDynamic(std::move(dy)));
  }
  return ret.toArray();
}

/**
 * Create a `dict<string, string>` out of a container of static strings.
 */
template <typename PairContainer>
Array makeDictOfStringToString(PairContainer&& vector) {
  auto ret = DictInit{vector.size()};
  for (auto&& [k, v] : std::forward<PairContainer>(vector)) {
    ret.set(StrNR{k.get()}, make_tv<KindOfPersistentString>(v.get()));
  }
  return ret.toArray();
}

template <typename F>
auto logPerformance(std::string_view name, F&& func) {
  using namespace std::chrono_literals;

  auto t0 = std::chrono::steady_clock::now();
  auto res = func();
  auto tf = std::chrono::steady_clock::now();
  auto elapsed =
      std::chrono::duration<double, std::chrono::milliseconds::period>{tf - t0};
  if (elapsed > 500ms) {
    XLOGF(
        DBG7,
        "[SLOW] FactsStoreImpl::{} completed in {:.2} ms",
        name,
        elapsed.count());
  } else {
    XLOGF(
        DBG8,
        "FactsStoreImpl::{} completed in {:.2} ms",
        name,
        elapsed.count());
  }
  return res;
}

/**
 * The actual AutoloadMap. Stores one SymbolMap for each kind of symbol.
 */
struct FactsStoreImpl final
    : public FactsStore,
      public std::enable_shared_from_this<FactsStoreImpl> {
  FactsStoreImpl(
      fs::path root,
      AutoloadDB::Handle dbHandle,
      std::shared_ptr<Watcher> watcher,
      hphp_hash_set<std::string> indexedMethodAttributes)
      : m_updateExec{1, make_thread_factory("Autoload update")},
        m_root{std::move(root)},
        m_map{m_root, std::move(dbHandle), std::move(indexedMethodAttributes)},
        m_watcher{std::move(watcher)} {}

  FactsStoreImpl(fs::path root, AutoloadDB::Handle dbHandle)
      : m_updateExec{1, make_thread_factory("Autoload update")},
        m_root{std::move(root)},
        m_map{m_root, std::move(dbHandle)} {}

  ~FactsStoreImpl() override {
    m_closing = true;
    try {
      m_updateFuture.wlock()->getFuture().wait();
    } catch (...) {
      // folly::Future::wait() might throw a FutureInvalid exception, but we're
      // shutting down.
    }
  }

  FactsStoreImpl(const FactsStoreImpl&) = delete;
  FactsStoreImpl(FactsStoreImpl&&) noexcept = delete;
  FactsStoreImpl& operator=(const FactsStoreImpl&) = delete;
  FactsStoreImpl& operator=(FactsStoreImpl&&) noexcept = delete;

  /**
   * This AutoloadMap is capable of building itself.
   */
  bool isNative() const noexcept override {
    return true;
  }

  Holder getNativeHolder() noexcept override {
    return Holder{
        this, [sptr = shared_from_this()]() mutable { sptr.reset(); }};
  }

  /**
   * Return an object representing the last time this store was updated
   */
  Clock getClock() const {
    auto version = m_map.getClock();
    return version.isInitial() ? m_map.dbClock() : version;
  }

  /**
   * Guarantee the map is at least as up-to-date as the codebase was
   * when update() was called.
   *
   * throws SQLiteExc or FactsStoreExc.
   */
  void ensureUpdated() override {
    TimeoutSuspender suspendTimeoutsDuringUpdate;

    folly::SemiFuture<folly::Unit> updateFuture = update();
    updateFuture.wait();
    auto const& res = std::move(updateFuture).result();
    if (res.hasException()) {
      res.throwUnlessValue();
    }
  }

  Array getAllFiles() const override {
    return logPerformance(__func__, [&]() {
      auto allPaths = m_map.getAllPaths();
      auto ret = VecInit{allPaths.size()};
      for (auto const& path : std::move(allPaths)) {
        fs::path p{path.get()->toCppString()};
        assertx(p.is_relative());
        ret.append(VarNR{String{(m_root / p).native()}}.tv());
      }
      return ret.toArray();
    });
  }

  Variant getTypeName(const String& type) override {
    return logPerformance(__func__, [&]() {
      auto name = m_map.getTypeName(*type.get());
      if (!name) {
        return Variant{Variant::NullInit{}};
      } else {
        return Variant{name->get(), Variant::PersistentStrInit{}};
      }
    });
  }

  Variant getKind(const String& type) override {
    return logPerformance(__func__, [&]() {
      auto const* kindStr = makeStaticString(
          toString(m_map.getKind(Symbol<SymKind::Type>{*type.get()})));

      if (kindStr == nullptr || kindStr->empty()) {
        return Variant{Variant::NullInit{}};
      } else {
        return Variant{kindStr, Variant::PersistentStrInit{}};
      }
    });
  }

  bool isTypeAbstract(const String& type) override {
    return logPerformance(
        __func__, [&]() { return m_map.isTypeAbstract(*type.get()); });
  }

  bool isTypeFinal(const String& type) override {
    return logPerformance(
        __func__, [&]() { return m_map.isTypeFinal(*type.get()); });
  }

  Optional<String> getTypeOrTypeAliasFile(const String& type) override {
    return getSymbolFile<SymKind::Type>(
        type, [](SymbolMap& m, Symbol<SymKind::Type> s) {
          return m.getTypeOrTypeAliasFile(s);
        });
  }

  Optional<String> getTypeFile(const String& type) override {
    return logPerformance(__func__, [&]() {
      return getSymbolFile<SymKind::Type>(
          type, [](SymbolMap& m, Symbol<SymKind::Type> s) {
            return m.getTypeFile(s);
          });
    });
  }

  Optional<String> getFunctionFile(const String& function) override {
    return logPerformance(__func__, [&]() {
      return getSymbolFile<SymKind::Function>(
          function, [](SymbolMap& m, Symbol<SymKind::Function> s) {
            return m.getFunctionFile(s);
          });
    });
  }

  Optional<String> getConstantFile(const String& constant) override {
    return logPerformance(__func__, [&]() {
      return getSymbolFile<SymKind::Constant>(
          constant, [](SymbolMap& m, Symbol<SymKind::Constant> s) {
            return m.getConstantFile(s);
          });
    });
  }

  Optional<String> getTypeAliasFile(const String& typeAlias) override {
    return logPerformance(__func__, [&]() {
      return getSymbolFile<SymKind::Type>(
          typeAlias, [](SymbolMap& m, Symbol<SymKind::Type> s) {
            return m.getTypeAliasFile(s);
          });
    });
  }

  Optional<fs::path> getTypeOrTypeAliasFile(std::string_view type) override {
    return getSymbolFile<SymKind::Type>(
        type, [](SymbolMap& m, Symbol<SymKind::Type> s) {
          return m.getTypeOrTypeAliasFile(s);
        });
  }

  Optional<String> getModuleFile(const String& module) override {
    return getSymbolFile<SymKind::Module>(
        module, [](SymbolMap& m, Symbol<SymKind::Module> s) {
          return m.getModuleFile(s);
        });
  }

  Optional<fs::path> getTypeFile(std::string_view type) override {
    return logPerformance(__func__, [&]() {
      return getSymbolFile<SymKind::Type>(
          type, [](SymbolMap& m, Symbol<SymKind::Type> s) {
            return m.getTypeFile(s);
          });
    });
  }

  Optional<fs::path> getFunctionFile(std::string_view func) override {
    return logPerformance(__func__, [&]() {
      return getSymbolFile<SymKind::Function>(
          func, [](SymbolMap& m, Symbol<SymKind::Function> s) {
            return m.getFunctionFile(s);
          });
    });
  }

  Optional<fs::path> getConstantFile(std::string_view name) override {
    return logPerformance(__func__, [&]() {
      return getSymbolFile<SymKind::Constant>(
          name, [](SymbolMap& m, Symbol<SymKind::Constant> s) {
            return m.getConstantFile(s);
          });
    });
  }

  Optional<fs::path> getTypeAliasFile(std::string_view name) override {
    return logPerformance(__func__, [&]() {
      return getSymbolFile<SymKind::Type>(
          name, [](SymbolMap& m, Symbol<SymKind::Type> s) {
            return m.getTypeAliasFile(s);
          });
    });
  }

  Optional<fs::path> getModuleFile(std::string_view module) override {
    return getSymbolFile<SymKind::Module>(
        module, [](SymbolMap& m, Symbol<SymKind::Module> s) {
          return m.getModuleFile(s);
        });
  }

  Array getFileTypes(const String& path) override {
    return logPerformance(__func__, [&]() {
      return getFileSymbols<SymKind::Type>(
          path, [](SymbolMap& m, Path s) { return m.getFileTypes(s); });
    });
  }

  Array getFileFunctions(const String& path) override {
    return logPerformance(__func__, [&]() {
      return getFileSymbols<SymKind::Function>(
          path, [](SymbolMap& m, Path s) { return m.getFileFunctions(s); });
    });
  }

  Array getFileConstants(const String& path) override {
    return logPerformance(__func__, [&]() {
      return getFileSymbols<SymKind::Constant>(
          path, [](SymbolMap& m, Path s) { return m.getFileConstants(s); });
    });
  }

  Array getFileTypeAliases(const String& path) override {
    return logPerformance(__func__, [&]() {
      return getFileSymbols<SymKind::Type>(
          path, [](SymbolMap& m, Path s) { return m.getFileTypeAliases(s); });
    });
  }

  Array getFileModules(const String& path) override {
    return logPerformance(__func__, [&]() {
      return getFileSymbols<SymKind::Module>(
          path, [](SymbolMap& m, Path s) { return m.getFileModules(s); });
    });
  }

  Array getBaseTypes(const String& derivedType, const Variant& filters)
      override {
    return logPerformance(__func__, [&]() {
      return getBaseTypes(
          derivedType,
          InheritanceFilterData::createFromShape(
              filters.isArray() ? filters.getArrayData() : nullptr));
    });
  }

  Array getDerivedTypes(const String& baseType, const Variant& filters)
      override {
    return logPerformance(__func__, [&]() {
      return getDerivedTypes(
          baseType,
          InheritanceFilterData::createFromShape(
              filters.isArray() ? filters.getArrayData() : nullptr));
    });
  }

  Array getTransitiveDerivedTypes(
      const String& baseType,
      const Variant& filters) override {
    return logPerformance(__func__, [&]() {
      return getTransitiveDerivedTypes(
          baseType,
          InheritanceFilterData::createFromShape(
              filters.isArray() ? filters.getArrayData() : nullptr));
    });
  }

  Array getTypesWithAttribute(const String& attr) override {
    return logPerformance(__func__, [&]() {
      return makeVecOfString(m_map.getTypesWithAttribute(*attr.get()));
    });
  }

  Array getTypeAliasesWithAttribute(const String& attr) override {
    return logPerformance(__func__, [&]() {
      return makeVecOfString(m_map.getTypeAliasesWithAttribute(*attr.get()));
    });
  }

  Array getMethodsWithAttribute(const String& attr) override {
    return logPerformance(__func__, [&]() {
      return makeVecOfStringString(m_map.getMethodsWithAttribute(*attr.get()));
    });
  }

  Array getFilesWithAttribute(const String& attr) override {
    return logPerformance(__func__, [&]() {
      return makeVecOfString(m_map.getFilesWithAttribute(*attr.get()));
    });
  }

  Array getTypeAttributes(const String& type) override {
    return logPerformance(__func__, [&]() {
      return makeVecOfString(m_map.getAttributesOfType(*type.get()));
    });
  }

  Array getTypeAliasAttributes(const String& typeAlias) override {
    return logPerformance(__func__, [&]() {
      return makeVecOfString(m_map.getAttributesOfTypeAlias(*typeAlias.get()));
    });
  }

  Array getMethodAttributes(const String& type, const String& method) override {
    return logPerformance(__func__, [&]() {
      return makeVecOfString(
          m_map.getAttributesOfMethod(*type.get(), *method.get()));
    });
  }

  Array getFileAttributes(const String& file) override {
    return logPerformance(__func__, [&]() {
      return makeVecOfString(m_map.getAttributesOfFile(Path{*file.get()}));
    });
  }

  Array getTypeAttrArgs(const String& type, const String& attribute) override {
    return logPerformance(__func__, [&]() {
      return makeVecOfDynamic(
          m_map.getTypeAttributeArgs(*type.get(), *attribute.get()));
    });
  }

  Array getTypeAliasAttrArgs(const String& typeAlias, const String& attribute)
      override {
    return logPerformance(__func__, [&]() {
      return makeVecOfDynamic(
          m_map.getTypeAliasAttributeArgs(*typeAlias.get(), *attribute.get()));
    });
  }

  Array getMethodAttrArgs(
      const String& type,
      const String& method,
      const String& attribute) override {
    return logPerformance(__func__, [&]() {
      return makeVecOfDynamic(m_map.getMethodAttributeArgs(
          *type.get(), *method.get(), *attribute.get()));
    });
  }

  Array getFileAttrArgs(const String& file, const String& attribute) override {
    return logPerformance(__func__, [&]() {
      return makeVecOfDynamic(
          m_map.getFileAttributeArgs(Path{*file.get()}, *attribute.get()));
    });
  }

  Array getAllTypes() override {
    return logPerformance(__func__, [&]() {
      return makeDictOfStringToString(m_map.getAllTypes());
    });
  }

  Array getAllFunctions() override {
    return logPerformance(__func__, [&]() {
      return makeDictOfStringToString(m_map.getAllFunctions());
    });
  }

  Array getAllConstants() override {
    return logPerformance(__func__, [&]() {
      return makeDictOfStringToString(m_map.getAllConstants());
    });
  }

  Array getAllModules() override {
    return logPerformance(__func__, [&]() {
      return makeDictOfStringToString(m_map.getAllModules());
    });
  }

  Array getAllTypeAliases() override {
    return logPerformance(__func__, [&]() {
      return makeDictOfStringToString(m_map.getAllTypeAliases());
    });
  }

  bool canHandleFailure() const override {
    return false;
  }

  AutoloadMap::Result handleFailure(
      KindOf UNUSED kind,
      const String& UNUSED className,
      const Variant& UNUSED err) const override {
    return AutoloadMap::Result::Failure;
  }

  /**
   * Update whenever a file in the filesystem changes.
   */
  void subscribe() {
    if (!m_watcher) {
      return;
    }
    auto clock = m_map.getClock();
    if (clock.isInitial()) {
      clock = m_map.dbClock();
    }
    m_watcher->subscribe(
        clock, [weakThis = weak_from_this()](Watcher::Results&& results) {
          XLOGF(
              INFO,
              "Subscription result: {} paths received.",
              results.m_files.size());
          auto sharedThis = weakThis.lock();
          if (!sharedThis) {
            return;
          }
          sharedThis->update();
        });
  }

  /**
   * Query our Watcher to see changed files, update our internal data
   * structures, and resolve the returned future once the map is at least as
   * up-to-date as the time update() was called.
   */
  folly::SemiFuture<folly::Unit> update() {
    if (!m_watcher) {
      return {};
    }
    auto updateFuture = m_updateFuture.wlock();
    *updateFuture = folly::splitFuture(
        updateFuture->getFuture()
            .via(&m_updateExec)
            .thenTry([this](const folly::Try<folly::Unit>&) {
              return m_watcher->getChanges(getClock());
            })
            .thenTry([this](folly::Try<Watcher::Results>&& results) {
              if (results.hasException()) {
                auto msg = folly::sformat(
                    "Exception while querying watcher: {}",
                    results.exception().what());
                XLOG(ERR) << msg;
                throw UpdateExc{msg};
              }
              return update(std::move(results.value()));
            }));
    return updateFuture->getFuture().semi();
  }

  folly::SemiFuture<folly::Unit> update(Watcher::Results&& results) {
    if (m_closing) {
      throw UpdateExc{"Shutting down"};
    }

    bool isFresh = results.m_fresh;
    Clock lastClock = *results.m_lastClock;
    Clock newClock = results.m_newClock;

    auto [alteredPathsAndHashes, deletedPaths] = isFresh
        ? getFreshDelta(std::move(results))
        : getIncrementalDelta(std::move(results));

    // We need to update the DB if Watchman has restarted or if
    // something's changed on the filesystem. Otherwise, there's no
    // need to update the DB.
    if (LIKELY(!isFresh) && LIKELY(alteredPathsAndHashes.empty()) &&
        LIKELY(deletedPaths.empty())) {
      XLOG(INFO) << "Finished: it's a a no-op incremental update.";
      return {};
    }

    XLOGF(
        INFO,
        "{} update: {} paths altered, {} paths deleted.",
        isFresh ? "Fresh" : "Incremental",
        alteredPathsAndHashes.size(),
        deletedPaths.size());

    auto alteredPathFacts = LIKELY(alteredPathsAndHashes.empty())
        ? std::vector<folly::Try<FileFacts>>{}
        : facts_from_paths(m_root, alteredPathsAndHashes);

    std::vector<FileFacts> facts;
    facts.reserve(alteredPathFacts.size());
    for (auto i = 0; i < alteredPathFacts.size(); i++) {
      try {
        facts.push_back(std::move(alteredPathFacts[i]).value());
      } catch (FactsExtractionExc& e) {
        XLOGF(
            CRITICAL,
            "Error extracting facts from {}: {}",
            alteredPathsAndHashes.at(i).m_path.c_str(),
            e.what());
        // Treat a parse error as if we had deleted the path
        deletedPaths.push_back(std::move(alteredPathsAndHashes.at(i).m_path));
        alteredPathsAndHashes.at(i) = {fs::path{}, {}};
      }
    }

    // Compact empty paths out of alteredPathsAndHashes
    if (facts.size() < alteredPathsAndHashes.size()) {
      alteredPathsAndHashes.erase(
          std::remove_if(
              alteredPathsAndHashes.begin(),
              alteredPathsAndHashes.end(),
              [](const PathAndOptionalHash& pathAndHash) {
                return pathAndHash.m_path.empty();
              }),
          alteredPathsAndHashes.end());
    }

    XLOGF(
        DBG0,
        "Facts size: {}. Altered paths size: {}",
        facts.size(),
        alteredPathsAndHashes.size());
    assertx(facts.size() == alteredPathsAndHashes.size());

    // Check for files with the SHA1 hash corresponding to an empty
    // file, but with nonempty facts
    for (auto i = 0; i < alteredPathsAndHashes.size(); ++i) {
      auto const& fileFacts = facts.at(i);
      auto const& pathAndHash = alteredPathsAndHashes[i];
      if (pathAndHash.m_hash == kEmptyFileSha1Hash && !fileFacts.isEmpty()) {
        throw FactsExtractionExc{folly::sformat(
            "{} has a SHA1 hash corresponding to an empty file ('{}'), "
            "but we are attempting to assign it non-empty facts: `{}`",
            pathAndHash.m_path.native(),
            kEmptyFileSha1Hash,
            json_from_facts(fileFacts))};
      }
    }

    XLOGF(
        DBG0,
        "SymbolMap.update(since={}, clock={}, "
        "alteredPathsAndHashes.size()={}, deletedPaths.size()={})",
        lastClock,
        newClock,
        alteredPathsAndHashes.size(),
        deletedPaths.size());
    m_map.update(
        lastClock,
        newClock,
        removeHashes(std::move(alteredPathsAndHashes)),
        std::move(deletedPaths),
        std::move(facts));

    XLOGF(INFO, "Thread {} has finished updating.", curthread());
    return {};
  }

  /**
   * Calculate the paths which have changed since the last Watchman
   * update when Watchman gives us the full state of the world.
   */

  std::tuple<std::vector<PathAndOptionalHash>, std::vector<fs::path>>
  getFreshDelta(Watcher::Results&& result) const {
    auto allPaths = m_map.getAllPathsWithHashes();
    XLOGF(
        INFO,
        "Received fresh query, checking against our list of {} paths.",
        allPaths.size());

    std::vector<PathAndOptionalHash> alteredPaths;
    std::vector<fs::path> deletedPaths;
    for (auto& pathData : std::move(result.m_files)) {
      assertx(pathData.m_exists);

      auto& path = pathData.m_path;
      auto pathStr = Path{path};

      auto sha1hex = pathData.m_hash;

      // Watchman is sending us all the files in the repo, regardless of
      // whether we've seen the same file before. Watchman is also
      // sending us SHA1 hashes, so compare these to determine which
      // files have actually changed.
      bool sha1HexMatches = [&]() {
        if (!pathData.m_hash.has_value()) {
          return false;
        }
        auto const it = allPaths.find(pathStr);
        if (it == allPaths.end()) {
          return false;
        }
        return SHA1{*sha1hex} == it->second;
      }();

      // Remove the path from the map to mark that we've seen
      // it. Watchman doesn't know about files which have been deleted
      // while it was down, so we have to figure out which files were
      // deleted ourselves.
      allPaths.erase(pathStr);

      if (sha1HexMatches) {
        continue;
      }

      alteredPaths.push_back({std::move(path), std::move(sha1hex)});
    }

    // All remaining paths are ones which Watchman didn't see. These
    // have been deleted.
    for (auto const& [pathStr, _] : allPaths) {
      deletedPaths.emplace_back(std::string{pathStr.slice()});
    }

    return {std::move(alteredPaths), std::move(deletedPaths)};
  }

  /**
   * Calculate the paths which have changed since the last Watchman
   * update when Watchman gives us an incremental update.
   */
  std::tuple<std::vector<PathAndOptionalHash>, std::vector<fs::path>>
  getIncrementalDelta(Watcher::Results&& result) const {
    std::vector<PathAndOptionalHash> alteredPaths;
    std::vector<fs::path> deletedPaths;
    for (auto& pathData : result.m_files) {
      if (!pathData.m_exists) {
        deletedPaths.push_back(std::move(pathData.m_path));
      } else {
        alteredPaths.push_back({std::move(pathData.m_path), pathData.m_hash});
      }
    }
    return {std::move(alteredPaths), std::move(deletedPaths)};
  }

  Array getBaseTypes(
      const String& derivedType,
      const InheritanceFilterData& filters) {
    std::vector<Symbol<SymKind::Type>> baseTypes;

    auto addBaseTypes = [&](DeriveKind kind) {
      auto newBaseTypes =
          m_map.getBaseTypes(Symbol<SymKind::Type>{*derivedType.get()}, kind);
      baseTypes.reserve(baseTypes.size() + newBaseTypes.size());
      std::move(
          std::begin(newBaseTypes),
          std::end(newBaseTypes),
          std::back_inserter(baseTypes));
    };

    if (!filters.m_deriveKindFilters.m_removeExtends) {
      addBaseTypes(DeriveKind::Extends);
    }
    if (!filters.m_deriveKindFilters.m_removeRequires) {
      addBaseTypes(DeriveKind::RequireClass);
      addBaseTypes(DeriveKind::RequireExtends);
      addBaseTypes(DeriveKind::RequireImplements);
    }

    return makeVecOfString(filterTypesByAttribute(
        filterByKind(std::move(baseTypes), filters.m_kindFilters),
        filters.m_attrFilters));
  }

  Array getDerivedTypes(
      const String& baseType,
      const InheritanceFilterData& filters) {
    std::vector<Symbol<SymKind::Type>> derivedTypes;

    auto addDerivedTypes = [&](DeriveKind kind) {
      auto newDerivedTypes =
          m_map.getDerivedTypes(Symbol<SymKind::Type>{*baseType.get()}, kind);
      derivedTypes.reserve(derivedTypes.size() + newDerivedTypes.size());
      std::move(
          std::begin(newDerivedTypes),
          std::end(newDerivedTypes),
          std::back_inserter(derivedTypes));
    };

    if (!filters.m_deriveKindFilters.m_removeExtends) {
      addDerivedTypes(DeriveKind::Extends);
    }
    if (!filters.m_deriveKindFilters.m_removeRequires) {
      addDerivedTypes(DeriveKind::RequireClass);
      addDerivedTypes(DeriveKind::RequireExtends);
      addDerivedTypes(DeriveKind::RequireImplements);
    }

    return makeVecOfString(filterTypesByAttribute(
        filterByKind(std::move(derivedTypes), filters.m_kindFilters),
        filters.m_attrFilters));
  }

  Array getTransitiveDerivedTypes(
      const String& baseType,
      const InheritanceFilterData& filters) {
    auto derivedTypeInfo = filterTypesByAttribute(
        m_map.getTransitiveDerivedTypes(
            *baseType.get(),
            filters.m_kindFilters.toMask(),
            filters.m_deriveKindFilters.toMask()),
        filters.m_attrFilters,
        [](auto const& tuple) { return std::get<0>(tuple); });
    VecInit derivedTypeVec{derivedTypeInfo.size()};
    for (auto const& [type, path, kind, flags] : derivedTypeInfo) {
      VecInit derivedTypeTuple{4};
      derivedTypeTuple.append(make_tv<KindOfPersistentString>(type.get()));
      derivedTypeTuple.append(make_tv<KindOfPersistentString>(path.get()));
      derivedTypeTuple.append(
          make_tv<KindOfPersistentString>(makeStaticString(toString(kind))));
      derivedTypeTuple.append(
          (flags & static_cast<TypeFlagMask>(TypeFlag::Abstract)) != 0);
      derivedTypeVec.append(std::move(derivedTypeTuple).toArray());
    }
    return derivedTypeVec.toArray();
  }

  folly::CPUThreadPoolExecutor m_updateExec;

  folly::Synchronized<folly::FutureSplitter<folly::Unit>> m_updateFuture{
      folly::splitFuture(folly::makeFuture())};
  std::atomic<bool> m_closing{false};
  fs::path m_root;
  SymbolMap m_map;
  std::shared_ptr<Watcher> m_watcher;

  template <SymKind k, typename TLambda>
  Array getFileSymbols(const String& path, TLambda lambda) {
    if (path.empty()) {
      return Array::CreateVec();
    }

    auto symbols = [&]() -> std::vector<Symbol<k>> {
      if (path.slice().at(0) != '/') {
        return lambda(m_map, Path{*path.get()});
      }
      auto resolvedPath =
          resolvePathRelativeToRoot(fs::path{path.toCppString()}, m_root);
      if (!resolvedPath) {
        return {};
      }
      return lambda(m_map, Path{*resolvedPath});
    }();
    VecInit ret{symbols.size()};
    for (auto s : std::move(symbols)) {
      ret.append(make_tv<KindOfPersistentString>(s.get()));
    }
    return ret.toArray();
  }

  template <SymKind K, class T>
  Optional<String> getSymbolFile(const String& symbol, T lambda) {
    auto path = getSymbolFile<K>(std::string_view{symbol.slice()}, lambda);
    if (UNLIKELY(!path)) {
      return {};
    }
    return String{path->native()};
  }

  template <SymKind K, class T>
  Optional<fs::path> getSymbolFile(std::string_view symbol, T lambda) {
    const StringData* fileStr = lambda(m_map, Symbol<K>{symbol}).get();
    if (UNLIKELY(!fileStr))
      return std::nullopt;
    fs::path p{fileStr->toCppString()};
    assertx(p.is_relative());
    return m_root / p;
  }

  /**
   * Filter the given `types` down to only those of the given `kinds`, such as
   * class, interface, enum, or trait.
   *
   * The Hack type of `kinds` is `keyset<HH\Facts\TypeKind>`.
   */
  std::vector<Symbol<SymKind::Type>> filterByKind(
      std::vector<Symbol<SymKind::Type>> types,
      const KindFilterData& kinds) {
    // Bail out if we aren't filtering on kind
    if (kinds.doesIncludeEverything()) {
      return types;
    }
    types.erase(
        std::remove_if(
            types.begin(),
            types.end(),
            [&](const Symbol<SymKind::Type>& type) {
              auto kind = m_map.getKind(type);
              switch (kind) {
                case Facts::TypeKind::Class:
                  return kinds.m_removeClasses;
                case Facts::TypeKind::Enum:
                  return kinds.m_removeEnums;
                case Facts::TypeKind::Interface:
                  return kinds.m_removeInterfaces;
                case Facts::TypeKind::Trait:
                  return kinds.m_removeTraits;
                default:
                  return false;
              }
            }),
        types.end());
    return types;
  }

  /**
   * Filter the given `types` down to only those with the given attributes.
   */
  template <typename T>
  std::vector<T> filterTypesByAttribute(
      std::vector<T> types,
      const AttributeFilterData& filter) {
    return filterTypesByAttribute(
        std::move(types), filter, [](T type) -> Symbol<SymKind::Type> {
          return type;
        });
  }

  template <typename T, typename TypeGetFn>
  std::vector<T> filterTypesByAttribute(
      std::vector<T> types,
      const AttributeFilterData& filter,
      TypeGetFn typeGetFn) {
    // Bail out if we aren't filtering on attributes
    if (filter.m_attrs.empty()) {
      return types;
    }

    // True iff the given attribute satisfies one of our AttributeFilterData
    // constraints.
    auto attrMatchesFilter = [&](Symbol<SymKind::Type> type,
                                 Symbol<SymKind::Type> attr) {
      // The given attribute isn't one of our constraints.
      auto it = filter.m_attrs.find(attr);
      if (it == filter.m_attrs.end()) {
        return false;
      }

      // The attribute is one of our constraints, and we don't care about
      // arguments.
      auto argFilters = it->second;
      if (argFilters.empty()) {
        return true;
      }

      // Check that each of our argument constraints is satisfied by the
      // attribute's argument.
      auto args = m_map.getTypeAttributeArgs(type, attr);
      for (auto const& [i, argFilter] : argFilters) {
        if (i >= args.size() || args[i] != argFilter) {
          return false;
        }
      }

      // This attribute satisfies all argument constraints.
      return true;
    };

    types.erase(
        std::remove_if(
            types.begin(),
            types.end(),
            [&](const T& typeInfo) {
              auto const& type = typeGetFn(typeInfo);
              size_t numAttrMatches = 0;
              for (auto attr : m_map.getAttributesOfType(type)) {
                if (attrMatchesFilter(type, attr)) {
                  ++numAttrMatches;
                }
              }
              return numAttrMatches < filter.m_attrs.size();
            }),
        types.end());
    return types;
  }
};

} // namespace

std::shared_ptr<FactsStore> make_watcher_facts(
    fs::path root,
    AutoloadDB::Handle dbHandle,
    std::shared_ptr<Watcher> watcher,
    bool shouldSubscribe,
    std::vector<std::string> indexedMethodAttrsVec) {
  hphp_hash_set<std::string> indexedMethodAttrs;
  indexedMethodAttrs.reserve(indexedMethodAttrsVec.size());
  for (auto& v : indexedMethodAttrsVec) {
    indexedMethodAttrs.insert(std::move(v));
  }
  auto map = std::make_shared<FactsStoreImpl>(
      std::move(root),
      std::move(dbHandle),
      std::move(watcher),
      std::move(indexedMethodAttrs));
  if (shouldSubscribe) {
    map->subscribe();
  }
  return map;
}

std::shared_ptr<FactsStore> make_trusted_facts(
    fs::path root,
    AutoloadDB::Handle dbHandle) {
  return std::make_shared<FactsStoreImpl>(std::move(root), std::move(dbHandle));
}

} // namespace Facts
} // namespace HPHP
