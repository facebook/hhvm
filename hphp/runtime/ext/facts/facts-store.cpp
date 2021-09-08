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

#include <memory>
#include <sstream>
#include <thread>

#include <folly/Likely.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/futures/Future.h>
#include <folly/futures/FutureSplitter.h>

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
#include "hphp/util/trace.h"

TRACE_SET_MOD(facts);

namespace HPHP {
namespace Facts {
namespace {

/**
 * Return a path relative to `root` with the same canonical location as `p`. If
 * `p` does not exist within `root`, return `std::nullopt`.
 */
Optional<folly::fs::path> resolvePathRelativeToRoot(
    const folly::fs::path& path, const folly::fs::path& root) {
  if (path.is_relative()) {
    return path;
  }

  if (folly::fs::starts_with(path, root)) {
    return folly::fs::remove_prefix(path, root);
  }

  if (!folly::fs::exists(path)) {
    return {};
  }

  auto canonicalPath = folly::fs::canonical(path);
  if (folly::fs::starts_with(canonicalPath, root)) {
    return folly::fs::remove_prefix(canonicalPath, root);
  }

  return std::nullopt;
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

  hphp_hash_map<Symbol<StringData, SymKind::Type>, ArgMap> m_attrs;

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
  static std::pair<StringPtr<StringData>, ArgMap>
  createAttrFilterFromShape(const ArrayData* attrShape) {
    assertx(attrShape);
    StringPtr<StringData> name{nullptr};
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

  static DeriveKindFilterData
  createFromKeyset(const ArrayData* deriveKindFilter) {
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

/**
 * Ensure the given `queryExpr` is requesting the fields we need Watchman to
 * return.
 */
folly::dynamic addFieldsToQuery(folly::dynamic queryExpr) {
  assertx(queryExpr.isObject());
  queryExpr["fields"] =
      folly::dynamic::array("name", "exists", "content.sha1hex");
  return queryExpr;
}

/**
 * Augment `query` with a field telling Watchman to give us changes since the
 * point in time given by `clock`.
 */
folly::dynamic addWatchmanSince(folly::dynamic query, Clock clock) {
  if (clock.isInitial()) {
    return query;
  }

  if (clock.m_mergebase.empty() && !clock.m_clock.empty()) {
    // Filesystem changes since a machine-local time
    query["since"] = std::move(clock.m_clock);
  } else if (!clock.m_mergebase.empty() && clock.m_clock.empty()) {
    // Repo changes since a global commit
    query["since"] = folly::dynamic::object(
        "scm",
        folly::dynamic::object("mergebase-with", std::move(clock.m_mergebase)));
  } else {
    // Changes since a machine-local time and global commit
    query["since"] = folly::dynamic::object("clock", std::move(clock.m_clock))(
        "scm",
        folly::dynamic::object("mergebase-with", std::move(clock.m_mergebase)));
  }
  // We're using the "since" generator, so clear all other generators
  query.erase("suffix");
  query.erase("glob");
  query.erase("path");
  return query;
}

/**
 * Return the SHA1 hash of the given file.
 *
 * If the "content.sha1hex" field of a Watchman response is a
 * string, then it is a hash of the file. If Watchman couldn't
 * find the SHA1 hash of the file, the "content.sha1hex" field
 * will instead be an object describing the error. If we didn't
 * get a hash from Watchman, it's not a huge deal. We'll just
 * reparse the file, hash it ourselves, and move on with our
 * lives.
 */
Optional<std::string> getSha1Hash(const folly::dynamic& pathData) {
  auto const& sha1hex = pathData["content.sha1hex"];
  if (!sha1hex.isString()) {
    return {};
  }
  return {sha1hex.asString()};
}

std::vector<folly::fs::path>
removeHashes(std::vector<PathAndHash>&& pathsWithHashes) {
  std::vector<folly::fs::path> paths;
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
Array makeVecOfStringString(const std::vector<MethodDecl<StringData>>& vector) {
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

/**
 * The actual AutoloadMap. Stores one SymbolMap for each kind of symbol.
 */
struct FactsStoreImpl final
    : public FactsStore,
      public std::enable_shared_from_this<FactsStoreImpl> {

  FactsStoreImpl(
      folly::fs::path root,
      DBData dbData,
      folly::dynamic queryExpr,
      Watchman& watchmanClient,
      hphp_hash_set<std::string> indexedMethodAttributes)
      : m_updateExec{1, make_thread_factory("Autoload update")}
      , m_root{std::move(root)}
      , m_map{m_root, std::move(dbData), RuntimeOption::AutoloadEnforceOneDefinitionRule, std::move(indexedMethodAttributes)}
      , m_watchmanData{
            {.m_queryExpr = addFieldsToQuery(std::move(queryExpr)),
             .m_watchmanClient = watchmanClient}} {
  }

  FactsStoreImpl(folly::fs::path root, DBData dbData)
      : m_updateExec{1, make_thread_factory("Autoload update")}
      , m_root{std::move(root)}
      , m_map{
            m_root,
            std::move(dbData),
            RuntimeOption::AutoloadEnforceOneDefinitionRule} {
  }

  ~FactsStoreImpl() override {
    m_closing = true;
    if (m_watchmanData) {
      m_watchmanData->m_updateFuture.getFuture().wait();
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

  /**
   * Guarantee the map is at least as up-to-date as the codebase was
   * when update() was called.
   *
   * throws SQLiteExc or FactsStoreExc.
   */
  void ensureUpdated() override {
    TimeoutSuspender suspendTimeoutsDuringUpdate;

    folly::Future<folly::Unit> updateFuture = update();
    updateFuture.wait();
    auto const& res = std::move(updateFuture).result();
    if (res.hasException()) {
      res.throwUnlessValue();
    }
  }

  Array getAllFiles() const override {
    auto allPaths = m_map.getAllPaths();
    auto ret = VecInit{allPaths.size()};
    for (auto&& path : std::move(allPaths)) {
      folly::fs::path p{path.get()->slice()};
      assertx(p.is_relative());
      ret.append(VarNR{String{(m_root / p).native()}}.tv());
    }
    return ret.toArray();
  }

  Variant getTypeName(const String& type) override {
    auto name = m_map.getTypeName(*type.get());
    if (!name) {
      return Variant{Variant::NullInit{}};
    } else {
      return Variant{name->get(), Variant::PersistentStrInit{}};
    }
  }

  Variant getKind(const String& type) override {
    auto const* kindStr = makeStaticString(toString(
        m_map.getKind(Symbol<StringData, SymKind::Type>{*type.get()})));

    if (kindStr == nullptr || kindStr->empty()) {
      return Variant{Variant::NullInit{}};
    } else {
      return Variant{kindStr, Variant::PersistentStrInit{}};
    }
  }

  bool isTypeAbstract(const String& type) override {
    return m_map.isTypeAbstract(*type.get());
  }

  bool isTypeFinal(const String& type) override {
    return m_map.isTypeFinal(*type.get());
  }

  Optional<String> getTypeFile(const String& type) override {
    return getSymbolFile<SymKind::Type>(
        type,
        [](SymbolMap<StringData>& m, Symbol<StringData, SymKind::Type> s) {
          return m.getTypeFile(s);
        });
  }

  Optional<String> getFunctionFile(const String& function) override {
    return getSymbolFile<SymKind::Function>(
        function,
        [](SymbolMap<StringData>& m, Symbol<StringData, SymKind::Function> s) {
          return m.getFunctionFile(s);
        });
  }

  Optional<String> getConstantFile(const String& constant) override {
    return getSymbolFile<SymKind::Constant>(
        constant,
        [](SymbolMap<StringData>& m, Symbol<StringData, SymKind::Constant> s) {
          return m.getConstantFile(s);
        });
  }

  Optional<String> getTypeAliasFile(const String& typeAlias) override {
    return getSymbolFile<SymKind::Type>(
        typeAlias,
        [](SymbolMap<StringData>& m, Symbol<StringData, SymKind::Type> s) {
          return m.getTypeAliasFile(s);
        });
  }

  Array getFileTypes(const String& path) override {
    return getFileSymbols<SymKind::Type>(
        path, [](SymbolMap<StringData>& m, Path<StringData> s) {
          return m.getFileTypes(s);
        });
  }

  Array getFileFunctions(const String& path) override {
    return getFileSymbols<SymKind::Function>(
        path, [](SymbolMap<StringData>& m, Path<StringData> s) {
          return m.getFileFunctions(s);
        });
  }

  Array getFileConstants(const String& path) override {
    return getFileSymbols<SymKind::Constant>(
        path, [](SymbolMap<StringData>& m, Path<StringData> s) {
          return m.getFileConstants(s);
        });
  }

  Array getFileTypeAliases(const String& path) override {
    return getFileSymbols<SymKind::Type>(
        path, [](SymbolMap<StringData>& m, Path<StringData> s) {
          return m.getFileTypeAliases(s);
        });
  }

  Array
  getBaseTypes(const String& derivedType, const Variant& filters) override {
    return getBaseTypes(
        derivedType,
        InheritanceFilterData::createFromShape(
            filters.isArray() ? filters.getArrayData() : nullptr));
  }

  Array
  getDerivedTypes(const String& baseType, const Variant& filters) override {
    return getDerivedTypes(
        baseType,
        InheritanceFilterData::createFromShape(
            filters.isArray() ? filters.getArrayData() : nullptr));
  }

  Array getTransitiveDerivedTypes(
      const String& baseType, const Variant& filters) override {
    return getTransitiveDerivedTypes(
        baseType,
        InheritanceFilterData::createFromShape(
            filters.isArray() ? filters.getArrayData() : nullptr));
  }

  Array getTypesWithAttribute(const String& attr) override {
    return makeVecOfString(m_map.getTypesWithAttribute(*attr.get()));
  }

  Array getTypeAliasesWithAttribute(const String& attr) override {
    return makeVecOfString(m_map.getTypeAliasesWithAttribute(*attr.get()));
  }

  Array getMethodsWithAttribute(const String& attr) override {
    return makeVecOfStringString(m_map.getMethodsWithAttribute(*attr.get()));
  }

  Array getFilesWithAttribute(const String& attr) override {
    return makeVecOfString(m_map.getFilesWithAttribute(*attr.get()));
  }

  Array getTypeAttributes(const String& type) override {
    return makeVecOfString(m_map.getAttributesOfType(*type.get()));
  }

  Array getTypeAliasAttributes(const String& typeAlias) override {
    return makeVecOfString(m_map.getAttributesOfType(*typeAlias.get()));
  }

  Array getMethodAttributes(const String& type, const String& method) override {
    return makeVecOfString(
        m_map.getAttributesOfMethod(*type.get(), *method.get()));
  }

  Array getFileAttributes(const String& file) override {
    return makeVecOfString(m_map.getAttributesOfFile(Path{*file.get()}));
  }

  Array getTypeAttrArgs(const String& type, const String& attribute) override {
    return makeVecOfDynamic(
        m_map.getTypeAttributeArgs(*type.get(), *attribute.get()));
  }

  Array getTypeAliasAttrArgs(const String& typeAlias, const String& attribute) override {
    return makeVecOfDynamic(
        m_map.getTypeAliasAttributeArgs(*typeAlias.get(), *attribute.get()));
  }

  Array getMethodAttrArgs(
      const String& type,
      const String& method,
      const String& attribute) override {
    return makeVecOfDynamic(m_map.getMethodAttributeArgs(
        *type.get(), *method.get(), *attribute.get()));
  }

  Array getFileAttrArgs(const String& file, const String& attribute) override {
    return makeVecOfDynamic(
        m_map.getFileAttributeArgs(Path{*file.get()}, *attribute.get()));
  }

  Array getAllTypes() override {
    return makeDictOfStringToString(m_map.getAllTypes());
  }

  Array getAllFunctions() override {
    return makeDictOfStringToString(m_map.getAllFunctions());
  }

  Array getAllConstants() override {
    return makeDictOfStringToString(m_map.getAllConstants());
  }

  Array getAllTypeAliases() override {
    return makeDictOfStringToString(m_map.getAllTypeAliases());
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
    if (!m_watchmanData) {
      return;
    }
    auto queryExpr = m_watchmanData->m_queryExpr;
    auto since = m_map.getClock();
    if (since.isInitial()) {
      since = m_map.dbClock();
    }
    if (!since.isInitial()) {
      queryExpr = addWatchmanSince(std::move(queryExpr), std::move(since));
    }
    m_watchmanData->m_watchmanClient.subscribe(
        queryExpr,
        [weakThis = weak_from_this()](folly::Try<folly::dynamic>&& results) {
          if (results.hasValue()) {
            auto const* files = results->get_ptr("files");
            if (files && files->isArray()) {
              FTRACE(
                  3, "Subscription result. {} paths received\n", files->size());
            } else {
              FTRACE(3, "Subscription result: {}\n", folly::toJson(*results));
            }
            auto sharedThis = weakThis.lock();
            if (!sharedThis) {
              return;
            }
            sharedThis->update();
          } else if (results.hasException()) {
            FTRACE(
                2,
                "Watchman subscription Error: {}\n",
                results.exception().what());
          }
        });
  }

private:
  /**
   * Query Watchman to see changed files, update our internal data structures,
   * and resolve the returned future once the map is at least as up-to-date as
   * the time update() was called.
   */
  folly::Future<folly::Unit> update() {
    if (!m_watchmanData) {
      return folly::makeFuture();
    }
    std::unique_lock g{m_mutex};
    m_watchmanData->m_updateFuture =
        folly::splitFuture(std::move(m_watchmanData->m_updateFuture)
                               .getFuture()
                               .via(&m_updateExec)
                               .thenTry([this](const folly::Try<folly::Unit>&) {
                                 return updateImpl();
                               }));
    return m_watchmanData->m_updateFuture.getFuture();
  }

  folly::Future<folly::Unit> updateImpl() {
    if (m_closing) {
      throw UpdateExc{"Shutting down"};
    }
    if (!m_watchmanData) {
      return folly::makeFuture();
    }

    auto since = m_map.getClock();
    if (since.isInitial()) {
      since = m_map.dbClock();
    }
    auto query = addWatchmanSince(m_watchmanData->m_queryExpr, since);

    FTRACE(3, "Querying watchman ({})\n", folly::toJson(query));
    return m_watchmanData->m_watchmanClient.query(std::move(query))
        .via(&m_updateExec)
        .thenValue([this, since](
                       const watchman::QueryResult& wrappedResults) mutable {
          auto const& result = wrappedResults.raw_;
          if (result.count("error")) {
            throw UpdateExc{folly::sformat(
                "Got a watchman error: {}\n", folly::toJson(result))};
          }

          // isFresh means we either didn't pass Watchman a "since" token,
          // or it means that Watchman has restarted after the point in time
          // that our "since" token represents.
          bool isIncremental = [&]() {
            if (!since.m_mergebase.empty()) {
              // If we passed a mergebase to Watchman, our query must be
              // incremental. Watchman's response will still contain
              // `{"is_fresh_instance": true}`, but we should ignore it in this
              // situation.
              return true;
            }
            auto const& fresh = result["is_fresh_instance"];
            return !fresh.isBool() || !fresh.asBool();
          }();

          auto [alteredPathsAndHashes, deletedPaths] =
              isIncremental ? getIncrementalDelta(result)
                            : getFreshDelta(result);

          // We need to update the DB if Watchman has restarted or if
          // something's changed on the filesystem. Otherwise, there's no
          // need to update the DB.
          if (LIKELY(isIncremental) && LIKELY(alteredPathsAndHashes.empty()) &&
              LIKELY(deletedPaths.empty())) {
            return;
          }

          FTRACE(
              3,
              "{} altered, {} deleted\n",
              alteredPathsAndHashes.size(),
              deletedPaths.size());

          auto alteredPathFacts =
              LIKELY(alteredPathsAndHashes.empty())
                  ? std::vector<folly::Try<FileFacts>>{}
                  : facts_from_paths(m_root, alteredPathsAndHashes);

          std::vector<FileFacts> facts;
          facts.reserve(alteredPathFacts.size());
          for (auto i = 0; i < alteredPathFacts.size(); i++) {
            try {
              facts.push_back(std::move(alteredPathFacts[i]).value());
            } catch (FactsExtractionExc& e) {
              Logger::Warning(
                  "Error extracting facts from %s: %s\n",
                  alteredPathsAndHashes.at(i).m_path.c_str(),
                  e.what());
              // Treat a parse error as if we had deleted the path
              deletedPaths.push_back(
                  std::move(alteredPathsAndHashes.at(i).m_path));
              alteredPathsAndHashes.at(i) = {folly::fs::path{}, {}};
            }
          }

          // Compact empty paths out of alteredPathsAndHashes
          if (facts.size() < alteredPathsAndHashes.size()) {
            alteredPathsAndHashes.erase(
                std::remove_if(
                    alteredPathsAndHashes.begin(),
                    alteredPathsAndHashes.end(),
                    [](const PathAndHash& pathAndHash) {
                      return pathAndHash.m_path.empty();
                    }),
                alteredPathsAndHashes.end());
          }

          FTRACE(
              3,
              "Facts size: {}. Altered paths size: {}\n",
              facts.size(),
              alteredPathsAndHashes.size());
          assertx(facts.size() == alteredPathsAndHashes.size());

          // Check for files with the SHA1 hash corresponding to an empty
          // file, but with nonempty facts
          for (auto i = 0; i < alteredPathsAndHashes.size(); ++i) {
            auto const& fileFacts = facts.at(i);
            auto const& pathAndHash = alteredPathsAndHashes[i];
            if (pathAndHash.m_hash == kEmptyFileSha1Hash &&
                !fileFacts.isEmpty()) {
              throw FactsExtractionExc{folly::sformat(
                  "{} has a SHA1 hash corresponding to an empty file ('{}'), "
                  "but we are attempting to assign it non-empty facts: `{}`",
                  pathAndHash.m_path.native(),
                  kEmptyFileSha1Hash,
                  json_from_facts(fileFacts))};
            }
          }

          auto clock = [&result]() -> Clock {
            auto const* fullClock = result.get_ptr("clock");
            always_assert(fullClock);
            if (fullClock->isString()) {
              return {.m_clock = fullClock->asString()};
            } else {
              auto const* localClock = fullClock->get_ptr("clock");
              always_assert(localClock);
              return {.m_clock = localClock->asString()};
            }
          }();

          FTRACE(
              3,
              "SymbolMap.update("
              "since={}, clock={}, alteredPathsAndHashes.size()={}, "
              "deletedPaths.size()={})\n",
              since,
              clock,
              alteredPathsAndHashes.size(),
              deletedPaths.size());
          m_map.update(
              since,
              clock,
              removeHashes(std::move(alteredPathsAndHashes)),
              std::move(deletedPaths),
              std::move(facts));

          FTRACE(3, "{} has finished updating\n", curthread());
        });
  }

  /**
   * Calculate the paths which have changed since the last Watchman
   * update when Watchman gives us the full state of the world.
   */

  std::tuple<std::vector<PathAndHash>, std::vector<folly::fs::path>>
  getFreshDelta(const folly::dynamic& result) const {

    auto allPaths = m_map.getAllPathsWithHashes();
    FTRACE(
        3,
        "Received fresh query, checking against our list of {} paths\n",
        allPaths.size());

    std::vector<PathAndHash> alteredPaths;
    std::vector<folly::fs::path> deletedPaths;
    for (auto const& pathData : result["files"]) {
      folly::fs::path path{pathData["name"].asString()};
      assertx(path.is_relative());
      assertx(pathData["exists"].asBool());

      auto pathStr = Path<StringData>{path};

      auto sha1hex = getSha1Hash(pathData);

      // Watchman is sending us all the files in the repo, regardless of
      // whether we've seen the same file before. Watchman is also
      // sending us SHA1 hashes, so compare these to determine which
      // files have actually changed.
      bool sha1HexMatches = [&]() {
        if (!sha1hex.has_value()) {
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
  std::tuple<std::vector<PathAndHash>, std::vector<folly::fs::path>>
  getIncrementalDelta(const folly::dynamic& result) const {

    std::vector<PathAndHash> alteredPaths;
    std::vector<folly::fs::path> deletedPaths;
    for (auto const& pathData : result["files"]) {
      folly::fs::path path{pathData["name"].asString()};
      assertx(path.is_relative());
      if (!pathData["exists"].asBool()) {
        deletedPaths.push_back(std::move(path));
        continue;
      }

      alteredPaths.push_back({std::move(path), getSha1Hash(pathData)});
    }
    return {std::move(alteredPaths), std::move(deletedPaths)};
  }

  Array getBaseTypes(
      const String& derivedType, const InheritanceFilterData& filters) {

    std::vector<Symbol<StringData, SymKind::Type>> baseTypes;

    auto addBaseTypes = [&](DeriveKind kind) {
      auto newBaseTypes = m_map.getBaseTypes(
          Symbol<StringData, SymKind::Type>{*derivedType.get()}, kind);
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
      addBaseTypes(DeriveKind::RequireExtends);
      addBaseTypes(DeriveKind::RequireImplements);
    }

    return makeVecOfString(filterTypesByAttribute(
        filterByKind(std::move(baseTypes), filters.m_kindFilters),
        filters.m_attrFilters));
  }

  Array getDerivedTypes(
      const String& baseType, const InheritanceFilterData& filters) {
    std::vector<Symbol<StringData, SymKind::Type>> derivedTypes;

    auto addDerivedTypes = [&](DeriveKind kind) {
      auto newDerivedTypes = m_map.getDerivedTypes(
          Symbol<StringData, SymKind::Type>{*baseType.get()}, kind);
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
      addDerivedTypes(DeriveKind::RequireExtends);
      addDerivedTypes(DeriveKind::RequireImplements);
    }

    return makeVecOfString(filterTypesByAttribute(
        filterByKind(std::move(derivedTypes), filters.m_kindFilters),
        filters.m_attrFilters));
  }

  Array getTransitiveDerivedTypes(
      const String& baseType, const InheritanceFilterData& filters) {
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

  std::mutex m_mutex;
  folly::CPUThreadPoolExecutor m_updateExec;

  std::atomic<bool> m_closing{false};
  folly::fs::path m_root;
  SymbolMap<StringData> m_map;

  /**
   * Updates this AutoloadMap using Watchman to track changed files.
   *
   * If this is `std::nullopt`, then we will treat the AutoloadMap as static.
   */
  struct WatchmanData {
    folly::dynamic m_queryExpr;
    Watchman& m_watchmanClient;
    folly::Future<watchman::SubscriptionPtr> m_subscribeFuture{nullptr};
    folly::FutureSplitter<folly::Unit> m_updateFuture{folly::makeFuture()};
  };
  Optional<WatchmanData> m_watchmanData;

  template <SymKind k, typename TLambda>
  Array getFileSymbols(const String& path, TLambda lambda) {
    if (path.empty()) {
      return Array::CreateVec();
    }

    auto symbols = [&]() -> std::vector<Symbol<StringData, k>> {
      if (path.slice().at(0) != '/') {
        return lambda(m_map, Path<StringData>{*path.get()});
      }
      auto resolvedPath =
          resolvePathRelativeToRoot(folly::fs::path{path.slice()}, m_root);
      if (!resolvedPath) {
        return {};
      }
      return lambda(m_map, Path<StringData>{*resolvedPath});
    }();
    VecInit ret{symbols.size()};
    for (auto s : std::move(symbols)) {
      ret.append(make_tv<KindOfPersistentString>(s.get()));
    }
    return ret.toArray();
  }

  template <SymKind k, class T>
  Optional<String> getSymbolFile(const String& symbol, T lambda) {
    const StringData* fileStr =
        lambda(m_map, Symbol<StringData, k>{*symbol.get()}).get();
    if (UNLIKELY(fileStr == nullptr)) {
      return {};
    }

    folly::fs::path p{folly::fs::path{fileStr->slice()}};
    assertx(p.is_relative());
    return String{(m_root / p).native()};
  }

  /**
   * Filter the given `types` down to only those of the given `kinds`, such as
   * class, interface, enum, or trait.
   *
   * The Hack type of `kinds` is `keyset<HH\Facts\TypeKind>`.
   */
  std::vector<Symbol<StringData, SymKind::Type>> filterByKind(
      std::vector<Symbol<StringData, SymKind::Type>> types,
      const KindFilterData& kinds) {
    // Bail out if we aren't filtering on kind
    if (kinds.doesIncludeEverything()) {
      return types;
    }
    types.erase(
        std::remove_if(
            types.begin(),
            types.end(),
            [&](const Symbol<StringData, SymKind::Type>& type) {
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
      std::vector<T> types, const AttributeFilterData& filter) {
    return filterTypesByAttribute(
        std::move(types),
        filter,
        [](T type) -> Symbol<StringData, SymKind::Type> { return type; });
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
    auto attrMatchesFilter = [&](Symbol<StringData, SymKind::Type> type,
                                 Symbol<StringData, SymKind::Type> attr) {
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

std::shared_ptr<FactsStore> make_watchman_facts(
    folly::fs::path root,
    DBData dbData,
    folly::dynamic queryExpr,
    Watchman& watchmanClient,
    bool shouldSubscribe,
    std::vector<std::string> indexedMethodAttrsVec) {
  hphp_hash_set<std::string> indexedMethodAttrs;
  indexedMethodAttrs.reserve(indexedMethodAttrsVec.size());
  for (auto& v : indexedMethodAttrsVec) {
    indexedMethodAttrs.insert(std::move(v));
  }
  auto map = std::make_shared<FactsStoreImpl>(
      std::move(root),
      std::move(dbData),
      std::move(queryExpr),
      watchmanClient,
      std::move(indexedMethodAttrs));
  if (shouldSubscribe) {
    map->subscribe();
  }
  return map;
}

std::shared_ptr<FactsStore>
make_trusted_facts(folly::fs::path root, DBData dbData) {
  return std::make_shared<FactsStoreImpl>(std::move(root), std::move(dbData));
}

} // namespace Facts
} // namespace HPHP
