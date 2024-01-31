/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ConfigPreprocessor.h"

#include <memory>
#include <random>

#include <folly/Format.h>
#include <folly/IPAddress.h>
#include <folly/Optional.h>
#include <folly/Random.h>
#include <folly/String.h>
#include <folly/hash/Hash.h>
#include <folly/json.h>

#include "mcrouter/lib/config/ImportResolverIf.h"
#include "mcrouter/lib/config/RendezvousHash.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/lib/fbi/network.h"

using folly::dynamic;
using folly::StringPiece;
using folly::json::stripComments;
using std::string;

namespace facebook {
namespace memcache {

namespace {

class NestedLimitGuard {
 public:
  explicit NestedLimitGuard(size_t& nestedLimit) : nestedLimit_(nestedLimit) {
    checkLogic(--nestedLimit_ > 0, "Too many nested macros. Check for cycles.");
  }
  ~NestedLimitGuard() {
    ++nestedLimit_;
  }

 private:
  size_t& nestedLimit_;
};

StringPiece asStringPiece(const dynamic& obj, StringPiece objName) {
  checkLogic(
      obj.isString(), "{} is {}, string expected", objName, obj.typeName());
  return obj.stringPiece();
}

const dynamic&
tryGet(const dynamic& obj, const dynamic& key, StringPiece objName) {
  auto it = obj.find(key);
  checkLogic(it != obj.items().end(), "{}: '{}' not found", objName, key);
  return it->second;
}

dynamic moveGet(dynamic& obj, const dynamic& key, StringPiece objName) {
  auto it = obj.get_ptr(key);
  checkLogic(it, "{}: '{}' not found", objName, key);
  return std::move(*it);
}

template <class Value>
const Value& tryGet(
    const folly::F14NodeMap<std::string, Value>& map,
    StringPiece key,
    StringPiece objName) {
  auto it = map.find(key);
  checkLogic(it != map.end(), "{}: '{}' not found", objName, key);
  return it->second;
}

StringPiece trim(StringPiece sp) {
  sp = folly::ltrimWhitespace(sp);
  while (!sp.empty()) {
    if (!isspace(sp.back())) {
      break;
    }
    // the space should be unescaped, i.e. number of consecutive slashes
    // should be even
    if (sp.size() > 1) {
      auto it = sp.end() - 2;
      size_t cntSlashes = 0;
      while (it != sp.begin() && *it == '\\') {
        ++cntSlashes;
        --it;
      }
      if (*it == '\\') {
        ++cntSlashes;
      }
      if (cntSlashes > 0) {
        if (cntSlashes % 2 == 0) {
          sp.pop_back();
        }
        break;
      }
    }
    sp.pop_back();
  }
  return sp;
}

/**
 * Finds matching bracket to one at position i in string s
 */
size_t matchingUnescapedBracket(StringPiece s, size_t i) {
  int diffOpenedClosed = 0;
  for (; i < s.size(); ++i) {
    if (s[i] == '\\') {
      ++i;
      continue;
    }
    if (s[i] == '(') {
      ++diffOpenedClosed;
    } else if (s[i] == ')') {
      --diffOpenedClosed;
    }

    // we are on closing bracket corresponding to ours
    if (diffOpenedClosed == 0) {
      return i;
    }
  }
  return string::npos;
}

size_t findUnescaped(folly::StringPiece str, char c) {
  for (auto it = str.begin(); it != str.end(); ++it) {
    if (*it == '\\') {
      ++it;
      continue;
    }
    if (*it == c) {
      return it - str.begin();
    }
  }
  return string::npos;
}

size_t unescapeUntil(StringPiece from, string& to, char c) {
  for (auto it = from.begin(); it != from.end(); ++it) {
    if (*it == '\\') {
      if (++it == from.end()) {
        return string::npos;
      }
      to.push_back(*it);
      continue;
    }
    if (*it == c) {
      return it - from.begin();
    }
    to.push_back(*it);
  }
  return string::npos;
}

} // namespace

///////////////////////////////Macro////////////////////////////////////////////

class ConfigPreprocessor::Context {
 private:
  enum class VarState { DEAD, RAW, EXPAND, LAZY_EXPAND };

 public:
  enum ExtendedTypeT { Extended };

  // not copyable
  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  // movable
  Context(Context&&) = default;
  Context& operator=(Context&&) = default;

  explicit Context(const ConfigPreprocessor& prep)
      : prep_(prep), baseContext_(true) {}

  Context(ExtendedTypeT, const Context& outer, bool baseContext = false)
      : prep_(outer.prep_), outer_(&outer), baseContext_(baseContext) {}

  const ConfigPreprocessor& prep() const {
    return prep_;
  }

  const Context* outer() const {
    return outer_;
  }

  dynamic& addExpanded(StringPiece key, dynamic value) {
    return add(key, std::move(value), VarState::EXPAND);
  }

  dynamic& addRaw(StringPiece key, dynamic value) {
    return add(key, std::move(value), VarState::RAW);
  }

  dynamic& addLocal(StringPiece key, dynamic value) {
    return add(key, std::move(value), VarState::LAZY_EXPAND);
  }

  dynamic expandRawArg(StringPiece key) {
    auto& ret = locals_.find(key)->second;
    assert(ret.second == VarState::RAW);
    ret.second = VarState::DEAD;
    if (outer_) {
      return prep_.expandMacros(std::move(ret.first), *outer_);
    } else {
      return prep_.expandMacros(std::move(ret.first), Context(prep_));
    }
  }

  folly::Optional<dynamic> tryExpandRawArg(StringPiece key) {
    if (locals_.find(key) != locals_.end()) {
      return expandRawArg(key);
    }
    return folly::none;
  }

  const dynamic& at(StringPiece key) const {
    auto it = locals_.find(key);
    assert(it != locals_.end());
    assert(it->second.second == VarState::EXPAND);
    return it->second.first;
  }

  dynamic move(StringPiece key) {
    auto& ret = locals_.find(key)->second;
    assert(ret.second == VarState::EXPAND);
    ret.second = VarState::DEAD;
    return std::move(ret.first);
  }

  const dynamic* find(StringPiece key) const {
    auto it = locals_.find(key);
    if (it != locals_.end()) {
      if (it->second.second == VarState::LAZY_EXPAND) {
        const_cast<Context&>(*this).doLazyExpand(key);
      }
      assert(it->second.second == VarState::EXPAND);
      return &it->second.first;
    }
    if (outer_ && !baseContext_) {
      return outer_->find(key);
    }
    return nullptr;
  }

  bool contains(StringPiece key) const {
    auto it = locals_.find(key);
    if (it != locals_.end()) {
      return true;
    }
    if (outer_ && !baseContext_) {
      return outer_->find(key);
    }
    return false;
  }

  void doLazyExpand(StringPiece key) {
    auto& it = locals_.find(key)->second;
    assert(it.second == VarState::LAZY_EXPAND || it.second == VarState::EXPAND);
    if (it.second == VarState::LAZY_EXPAND) {
      try {
        it.first = prep_.expandMacros(std::move(it.first), *this);
        it.second = VarState::EXPAND;
      } catch (const std::exception& e) {
        throwLogic("Variable '{}':\n{}", key, e.what());
      }
    }
  }

 private:
  const ConfigPreprocessor& prep_;
  const Context* outer_{nullptr};
  folly::F14NodeMap<std::string, std::pair<dynamic, VarState>> locals_;
  bool baseContext_{false};

  dynamic& add(StringPiece key, dynamic&& value, VarState state) {
    auto it = locals_.emplace(key, std::make_pair(std::move(value), state));
    checkLogic(it.second, "'{}' already exists in local scope", key);
    return it.first->second.first;
  }
};

class ConfigPreprocessor::Macro {
 public:
  using Func = folly::Function<dynamic(Context&&) const>;

  Macro(
      const ConfigPreprocessor& prep,
      StringPiece name,
      const std::vector<dynamic>& params,
      Func f,
      bool autoExpand = true)
      : prep_(prep), f_(std::move(f)), autoExpand_(autoExpand), name_(name) {
    initParams(params);
  }

  dynamic getResult(const std::vector<StringPiece>& params, const Context& ctx)
      const {
    checkLogic(
        minParamCnt_ <= params.size(),
        "Too few arguments for macro {}. Expected at least {} got {}",
        name_,
        minParamCnt_,
        params.size());

    checkLogic(
        params.size() <= maxParamCnt_,
        "Too many arguments for macro {}. Expected at most {} got {}",
        name_,
        maxParamCnt_,
        params.size());

    Context result(Context::Extended, ctx, /* base */ true);
    for (size_t i = 0; i < params.size(); i++) {
      if (autoExpand_) {
        result.addExpanded(
            std::get<0>(params_[i]), prep_.expandStringMacro(params[i], ctx));
      } else {
        result.addRaw(std::get<0>(params_[i]), params[i]);
      }
    }

    for (size_t i = params.size(); i < params_.size(); i++) {
      if (!std::get<1>(params_[i]).isNull()) {
        result.addExpanded(std::get<0>(params_[i]), std::get<1>(params_[i]));
      }
    }
    return f_(std::move(result));
  }

  dynamic getResult(dynamic&& obj, const Context& ctx) const {
    Context result(Context::Extended, ctx, /* base */ true);
    for (const auto& p : params_) {
      auto it = obj.get_ptr(std::get<0>(p));
      if (!it) {
        checkLogic(
            !std::get<2>(p),
            "Macro call {}: '{}' parameter not found",
            name_,
            std::get<0>(p));
        if (!std::get<1>(p).isNull()) {
          result.addExpanded(std::get<0>(p), std::get<1>(p));
        }
      } else {
        if (autoExpand_) {
          result.addExpanded(
              std::get<0>(p), prep_.expandMacros(std::move(*it), ctx));
        } else {
          result.addRaw(std::get<0>(p), std::move(*it));
        }
      }
    }
    return f_(std::move(result));
  }

 private:
  const ConfigPreprocessor& prep_;
  Func f_;
  bool autoExpand_{true};
  // name, default, required?
  std::vector<std::tuple<string, dynamic, bool>> params_;
  size_t maxParamCnt_{0};
  size_t minParamCnt_{0};
  StringPiece name_;

  void initParams(const std::vector<dynamic>& params) {
    maxParamCnt_ = minParamCnt_ = params.size();
    bool needOptional = false;
    for (const auto& param : params) {
      bool hasOptional = false;
      if (param.isString()) { // param name
        params_.emplace_back(param.getString(), nullptr, true);
      } else if (param.isObject()) { // object (name & default)
        auto name = asStringPiece(
            tryGet(param, "name", "Macro param object"),
            "Macro param object name");

        auto optionalPtr = param.get_ptr("optional");
        if (optionalPtr) {
          checkLogic(
              optionalPtr->isBool(),
              "Incorrect definition for parameter '{}' in macro '{}'. Expected "
              "optional parameter 'optional' to be boolean, but got {}!",
              name,
              name_,
              optionalPtr->typeName());
        }

        if (auto defaultPtr = param.get_ptr("default")) {
          if (optionalPtr && !optionalPtr->getBool()) {
            throwLogic(
                "Conflicting options provided for parameter '{}' in "
                "macro '{}'. Default was set, but the parameter is also "
                "explicitly marked as not optional.",
                name,
                name_);
          }
          hasOptional = true;
          --minParamCnt_;
          params_.emplace_back(name.str(), *defaultPtr, false);
        } else if (optionalPtr && optionalPtr->getBool()) {
          hasOptional = true;
          --minParamCnt_;
          params_.emplace_back(name.str(), nullptr, false);
        } else {
          params_.emplace_back(name.str(), nullptr, true);
        }
      } else {
        throwLogic(
            "Macro param is {}, expected string/object", param.typeName());
      }

      checkLogic(
          hasOptional || !needOptional,
          "Incorrect defaults/optionals in macro {}. "
          "All params after optional/default one should also be "
          "optional or have default value",
          name_);

      needOptional = needOptional || hasOptional;
    }
  }
};

////////////////////////////////////Const///////////////////////////////////////

class ConfigPreprocessor::Const {
 public:
  Const(const ConfigPreprocessor& prep, StringPiece name, dynamic result)
      : prep_(prep), name_(name), result_(std::move(result)) {}

  const dynamic& getResult() const {
    if (!expanded_) {
      try {
        result_ = prep_.expandMacros(std::move(result_), Context(prep_));
      } catch (const std::logic_error& e) {
        throwLogic("Const '{}':\n{}", name_, e.what());
      }
      expanded_ = true;
    }
    return result_;
  }

 private:
  mutable bool expanded_{false};
  const ConfigPreprocessor& prep_;
  StringPiece name_;
  mutable dynamic result_;
};

///////////////////////////////////BuiltIns/////////////////////////////////////

class ConfigPreprocessor::BuiltIns {
 public:
  /**
   * Loads JSONM from external source via importResolver.
   * Usage: @import(path)
   */
  static dynamic importMacro(
      ConfigPreprocessor& p,
      ImportResolverIf& importResolver,
      Context&& ctx) {
    auto path = ctx.expandRawArg("path");
    auto pathStr = asStringPiece(path, "import path");
    // cache each result by path, so we won't import same path twice
    auto it = p.importCache_.find(pathStr);
    if (it != p.importCache_.end()) {
      return it->second;
    }
    dynamic result = nullptr;
    try {
      auto jsonC = importResolver.import(pathStr);
      // result may contain comments, macros, etc.
      result =
          p.expandMacros(parseJsonString(stripComments(jsonC)), Context(p));
    } catch (const std::exception& e) {
      if (auto defaultVal = ctx.tryExpandRawArg("default")) {
        p.importCache_.emplace(pathStr, *defaultVal);
        return std::move(*defaultVal);
      }
      throwLogic("Import '{}':\n{}", pathStr, e.what());
    }
    p.importCache_.emplace(pathStr, result);
    return result;
  }

  /**
   * Computes hash of an int or string
   * Usage: @hash(prn1c05)
   * Usage: @hash(@int(52135))
   */
  static dynamic hashMacro(Context&& ctx) {
    const auto& val = ctx.at("value");
    if (val.isInt()) {
      return folly::Hash()(val.getInt());
    } else if (val.isString()) {
      return folly::Hash()(val.stringPiece());
    } else {
      // invalid
      throwLogic("Hash: can not cast {} to int or string", val.typeName());
    }
  }

  /**
   * Computes weighted hash given a dictionary of choices and a key
   *
   * Usage: {
   *  "type": "weightedHash",
   *  "dictionary": { <string>: 0.0..1.0 },
   *  "key": <string> or <int>
   * }
   * Example: {
   *  "type": "weightedHash",
   *  "dictionary": {
   *    "a": 0.0,
   *    "b": 1.0
   *  },
   *  "key": 5
   * }
   * => "b"
   */
  static dynamic weightedHashMacro(Context&& ctx) {
    const auto& dictionary = ctx.at("dictionary");
    checkLogic(
        dictionary.isObject(),
        "WeightedHash: dictionary is {}, expected object",
        dictionary.typeName());

    const auto& key = ctx.at("key");
    std::vector<std::pair<StringPiece, double>> weights;
    for (const auto& it : dictionary.items()) {
      auto name = it.first.stringPiece();
      checkLogic(
          it.second.isNumber(),
          "WeightedHash: {} weight is {}, expected number",
          name,
          it.second.typeName());
      auto weight = it.second.asDouble();
      checkLogic(
          -0.01 <= weight,
          "WeightedHash: {} weight {} is negative",
          name,
          weight);
      weights.emplace_back(name, std::max(weight, 0.0));
    }
    checkLogic(!weights.empty(), "WeightedHash: dictionary is empty");

    uint64_t keyHash;
    if (key.isInt()) {
      keyHash = key.getInt();
    } else if (key.isString()) {
      keyHash = folly::Hash()(key.stringPiece());
    } else {
      throwLogic(
          "WeightedHash: key is {}, expected string or int", key.typeName());
    }
    try {
      auto id = RendezvousHash(weights.begin(), weights.end()).get(keyHash);
      return weights[id].first;
    } catch (const std::exception& e) {
      throwLogic("WeightedHash: failed to compute hash: {}", e.what());
    }
  }

  /**
   * Casts its argument to int.
   * Usage: @int(5)
   */
  static dynamic intMacro(Context&& ctx) {
    try {
      return ctx.at("value").asInt();
    } catch (const std::exception& e) {
      throwLogic(
          "Can not cast {} to int:\n{}", ctx.at("value").typeName(), e.what());
    }
  }

  /**
   * Casts its argument to double.
   * Usage: @double(5.1)
   */
  static dynamic doubleMacro(Context&& ctx) {
    try {
      return ctx.at("value").asDouble();
    } catch (const std::exception& e) {
      throwLogic(
          "Can not cast {} to double:\n{}",
          ctx.at("value").typeName(),
          e.what());
    }
  }

  /**
   * Casts its argument to string.
   * Usage: @str(@int(5))
   */
  static dynamic strMacro(Context&& ctx) {
    try {
      return ctx.at("value").asString();
    } catch (const std::exception& e) {
      throwLogic(
          "Can not cast {} to string:\n{}",
          ctx.at("value").typeName(),
          e.what());
    }
  }

  /**
   * Casts its argument to boolean.
   * Usage: @bool(false); @bool(true); @bool(@int(0)); @bool(@int(1))
   */
  static dynamic boolMacro(Context&& ctx) {
    try {
      return ctx.at("value").asBool();
    } catch (const std::exception& e) {
      throwLogic(
          "Can not cast {} to boolean:\n{}",
          ctx.at("value").typeName(),
          e.what());
    }
  }

  /**
   * Returns array of object keys
   * Usage: @keys(object)
   */
  static dynamic keysMacro(Context&& ctx) {
    const auto& dictionary = ctx.at("dictionary");
    checkLogic(dictionary.isObject(), "Keys: dictionary is not object");
    return dynamic(dictionary.keys().begin(), dictionary.keys().end());
  }

  /**
   * Returns array of object values
   * Usage: @values(object)
   */
  static dynamic valuesMacro(Context&& ctx) {
    const auto& dictionary = ctx.at("dictionary");
    checkLogic(dictionary.isObject(), "Values: dictionary is not object");
    return dynamic(dictionary.values().begin(), dictionary.values().end());
  }

  /**
   * Merges lists/objects/strings.
   * Usage:
   * "type": "merge",
   * "params": [ list1, list2, list3, ... ]
   * or
   * "type": "merge",
   * "params": [ obj1, obj2, obj3, ... ]
   * or
   * "type": "merge"
   * "params": [ str1, str2, str3, ... ]
   *
   * Returns single list/object which contains elements/properties of all
   * passed objects.
   * Note: properties of obj{N} will override properties of obj{N-1}.
   * In case params are strings, "merge" concatenates them.
   */
  static dynamic mergeMacro(Context&& ctx) {
    auto mergeParams = ctx.move("params");

    checkLogic(mergeParams.isArray(), "Merge: 'params' is not array");
    checkLogic(!mergeParams.empty(), "Merge: empty params");

    // get first param, it will determine the result type (array or object)
    auto res = std::move(mergeParams[0]);
    checkLogic(
        res.isArray() || res.isObject() || res.isString(),
        "Merge: first param is not array/object/string");
    for (size_t i = 1; i < mergeParams.size(); ++i) {
      auto& it = mergeParams[i];
      if (res.isArray()) {
        checkLogic(it.isArray(), "Merge: param {} is not an array", i);
        for (auto& inner : it) {
          res.push_back(std::move(inner));
        }
      } else if (res.isObject()) {
        checkLogic(it.isObject(), "Merge: param {} is not an object", i);
        // override properties
        for (auto& inner : it.items()) {
          auto& key = const_cast<dynamic&>(inner.first);
          auto& value = const_cast<dynamic&>(inner.second);
          res.insert(std::move(key), std::move(value));
        }
      } else { // string
        checkLogic(it.isString(), "Merge: param {} is not a string", i);
        res.getString() += it.getString();
      }
    }
    return res;
  }

  /**
   * Randomly shuffles list.
   * Usage: @shuffle(list) or @shuffle(list, seed)
   *
   * "dictionary": list
   * "seed": int (optional, must be non-negative)
   * Returns list with randomly shuffled items.
   */
  static dynamic shuffleMacro(Context&& ctx) {
    auto array = ctx.expandRawArg("dictionary");
    checkLogic(array.isArray(), "Shuffle: argument must be an array");

    static thread_local std::minstd_rand defaultEngine(
        folly::randomNumberSeed());

    if (auto seedArg = ctx.tryExpandRawArg("seed")) {
      checkLogic(
          seedArg->isInt() && seedArg->getInt() >= 0,
          "Shuffle: seed must be a non-negative integer");
      auto seed = static_cast<uint32_t>(seedArg->getInt());
      std::minstd_rand seededEngine(seed);
      std::shuffle(array.begin(), array.end(), seededEngine);
      return array;
    }

    std::shuffle(array.begin(), array.end(), defaultEngine);
    return array;
  }

  /**
   * Selects element from list/object.
   * Usage: @select(obj,string) or
   *        @select(list,int)
   *
   * Returns value of corresponding object property/list element
   */
  static dynamic selectMacro(Context&& ctx) {
    auto dictionary = ctx.expandRawArg("dictionary");
    const auto key = ctx.expandRawArg("key");

    checkLogic(
        dictionary.isObject() || dictionary.isArray(),
        "Select: dictionary is {}, expected array/object",
        dictionary.typeName());

    if (dictionary.isObject()) {
      checkLogic(
          key.isString(),
          "Select: dictionary is an object, key is not a string");
      if (auto jValue = dictionary.get_ptr(key)) {
        return std::move(*jValue);
      }
      if (auto defaultVal = ctx.tryExpandRawArg("default")) {
        return std::move(*defaultVal);
      }
      throwLogic(
          "Select: '{}' not found, default not specified", key.stringPiece());
    } else { // array
      checkLogic(
          key.isInt(), "Select: dictionary is an array, key is not an integer");
      auto id = key.getInt();
      if (id < 0) {
        id += dictionary.size();
      }
      if (id >= 0 && size_t(id) < dictionary.size()) {
        return std::move(dictionary[id]);
      }
      if (auto defaultVal = ctx.tryExpandRawArg("default")) {
        return std::move(*defaultVal);
      }
      throwLogic(
          "Select: index {} is out of range [0, {})", id, dictionary.size());
    }
  }

  /**
   * Returns range from list/object/string.
   * Usage:
   * "type": "slice",
   * "dictionary": obj,
   * "from": string,
   * "to": string
   * or
   * "type": "slice",
   * "dictionary": list/string,
   * "from": int,
   * "to": int
   *
   * Returns:
   * - in case of list range of elements from <= id <= to.
   * - in case of object range of properties with keys from <= key <= to.
   * - in case of string substring [from, to]
   * Note: from and to are inclusive
   */
  static dynamic sliceMacro(Context&& ctx) {
    const auto& from = ctx.at("from");
    const auto& to = ctx.at("to");
    auto dict = ctx.move("dictionary");

    checkLogic(
        dict.isObject() || dict.isArray() || dict.isString(),
        "Slice: dictionary is not array/object/string");

    if (dict.isObject()) {
      dynamic res = dynamic::object();
      checkLogic(from.isString(), "Slice: from is not a string");
      checkLogic(to.isString(), "Slice: to is not a string");
      auto fromKey = from.stringPiece();
      auto toKey = to.stringPiece();
      // since dictionary is unordered, we should iterate over it
      for (auto& it : dict.items()) {
        auto& key = const_cast<dynamic&>(it.first);
        auto& value = const_cast<dynamic&>(it.second);
        if (fromKey <= key.stringPiece() && key.stringPiece() <= toKey) {
          res.insert(std::move(key), std::move(value));
        }
      }
      return res;
    } else if (dict.isArray()) {
      dynamic res = dynamic::array;
      checkLogic(from.isInt() && to.isInt(), "Slice: from/to is not an int");
      auto fromId = std::max((int64_t)0, from.asInt());
      auto toId = std::min(to.asInt() + 1, (int64_t)dict.size());
      for (auto i = fromId; i < toId; ++i) {
        res.push_back(std::move(dict[i]));
      }
      return res;
    } else { // string
      string res;
      auto dictStr = dict.stringPiece();
      checkLogic(from.isInt() && to.isInt(), "Slice: from/to is not an int");
      auto fromId = std::max((int64_t)0, from.asInt());
      auto toId = std::min(to.asInt() + 1, (int64_t)dict.size());
      for (auto i = fromId; i < toId; ++i) {
        res += dictStr[i];
      }
      return res;
    }
  }

  /**
   * Splits a string by delimiter. Returns list of strings.
   * Usage:
   *  "type": "split",
   *  "dictionary": "a.b.c.",
   *  "delim": "."
   * => [ "a", "b", "c", "" ]
   */
  static dynamic splitMacro(Context ctx) {
    auto dict = asStringPiece(ctx.at("dictionary"), "Split: dictionary");
    auto delim = asStringPiece(ctx.at("delim"), "Split: delim");

    std::vector<StringPiece> result;
    folly::split(delim, dict, result);
    return folly::dynamic(result.begin(), result.end());
  }

  /**
   * Returns `dictionary` with `key` set to `value` i.e.
   *   dictionary[key] = value;
   *   return dictionary;
   *
   * Usage:
   * "type": "set",
   * "dictionary": array or object,
   * "key": string or int,
   * "value": jsonm
   */
  static dynamic setMacro(Context&& ctx) {
    const auto& key = ctx.at("key");
    auto value = ctx.move("value");
    auto dict = ctx.move("dictionary");

    checkLogic(
        dict.isObject() || dict.isArray(),
        "Set: dictionary is not array/object");

    if (dict.isObject()) {
      checkLogic(key.isString(), "Set: key is not a string");
      dict[key.getString()] = std::move(value);
      return dict;
    } else { // array
      checkLogic(key.isInt(), "Set: key is not an int");
      auto id = key.getInt();
      checkLogic(
          0 <= id && static_cast<size_t>(id) < dict.size(),
          "Set: key '{}' is out of range [0..{})",
          id,
          dict.size());
      dict[id] = std::move(value);
      return dict;
    }
  }

  /**
   * Returns list of integers [from, from+1, ... to]
   */
  static dynamic rangeMacro(Context&& ctx) {
    const auto& from = ctx.at("from");
    const auto& to = ctx.at("to");
    checkLogic(from.isInt(), "Range: from is not an integer");
    checkLogic(to.isInt(), "Range: to is not an integer");
    dynamic result = dynamic::array;
    for (auto i = from.asInt(); i <= to.asInt(); ++i) {
      result.push_back(i);
    }
    return result;
  }

  /**
   * Returns true if:
   * - dictionary is object which contains key == key (param)
   * - dictionary is array which contains item == key (param)
   * - dictionary is string which contains key (param) as a substring.
   * Usage: @contains(dictionary,value)
   */
  static dynamic containsMacro(Context&& ctx) {
    const auto& dictionary = ctx.at("dictionary");
    const auto& key = ctx.at("key");
    if (dictionary.isObject()) {
      checkLogic(
          key.isString(),
          "Contains: dictionary is an object, key is not a string");
      return dictionary.find(key) != dictionary.items().end();
    } else if (dictionary.isArray()) {
      for (const auto& it : dictionary) {
        if (it == key) {
          return true;
        }
      }
      return false;
    } else if (dictionary.isString()) {
      checkLogic(
          key.isString(),
          "Contains: dictionary is a string, key is not a string");
      return dictionary.stringPiece().find(key.stringPiece()) != string::npos;
    } else {
      throwLogic(
          "Contains: dictionary is {}, expected object/array/string",
          dictionary.typeName());
    }
  }

  /**
   * Returns true if dictionary (object/array/string) is empty
   * Usage: @empty(dictionary)
   */
  static dynamic emptyMacro(Context&& ctx) {
    const auto& dict = ctx.at("dictionary");
    checkLogic(
        dict.isObject() || dict.isArray() || dict.isString(),
        "empty: dictionary is not object/array/string");
    return dict.empty();
  }

  /**
   * Returns true if A == B
   * Usage: @equals(A,B)
   */
  static dynamic equalsMacro(Context&& ctx) {
    return ctx.at("A") == ctx.at("B");
  }

  /**
   * Returns true if value is an array
   * Usage: @isArray(value)
   */
  static dynamic isArrayMacro(Context&& ctx) {
    return ctx.at("value").isArray();
  }

  /**
   * Returns true if value is boolean
   * Usage: @isBool(value)
   */
  static dynamic isBoolMacro(Context&& ctx) {
    return ctx.at("value").isBool();
  }

  /**
   * Returns true if value is an integer
   * Usage: @isInt(value)
   */
  static dynamic isIntMacro(Context&& ctx) {
    return ctx.at("value").isInt();
  }

  /**
   * Returns true if value is a double
   * Usage: @isDouble(value)
   */
  static dynamic isDoubleMacro(Context&& ctx) {
    return ctx.at("value").isDouble();
  }

  /**
   * Returns true if value is an object
   * Usage: @isObject(value)
   */
  static dynamic isObjectMacro(Context&& ctx) {
    return ctx.at("value").isObject();
  }

  /**
   * Returns true if value is a string
   * Usage: @isString(value)
   */
  static dynamic isStringMacro(Context&& ctx) {
    return ctx.at("value").isString();
  }

  /**
   * Returns true if A < B
   * Usage: @less(A,B)
   */
  static dynamic lessMacro(Context&& ctx) {
    const auto& A = ctx.at("A");
    const auto& B = ctx.at("B");
    checkLogic(!A.isObject() && !B.isObject(), "Can not compare objects");
    return A < B;
  }

  /**
   * Return the result of A & B.
   * Usage: @bitwiseAnd(A, B)
   */
  static dynamic bitwiseAndMacro(Context&& ctx) {
    const auto& A = ctx.at("A");
    const auto& B = ctx.at("B");
    checkLogic(A.isInt(), "bitwiseAnd: A is not an integer");
    checkLogic(B.isInt(), "bitwiseAnd: B is not an integer");
    return A.getInt() & B.getInt();
  }

  /**
   * return the result of A | B
   * Usage: @bitwiseOr(A, B)
   */
  static dynamic bitwiseOrMacro(Context&& ctx) {
    const auto& A = ctx.at("A");
    const auto& B = ctx.at("B");
    checkLogic(A.isInt(), "bitwiseOr: A is not an integer");
    checkLogic(B.isInt(), "bitwiseOr: B is not an integer");
    return A.getInt() | B.getInt();
  }

  /**
   * Return the result of A ^ B
   * Usage: @bitwiseXor(A, B)
   */
  static dynamic bitwiseXorMacro(Context&& ctx) {
    const auto& A = ctx.at("A");
    const auto& B = ctx.at("B");
    checkLogic(A.isInt(), "bitwiseXor: A is not an integer");
    checkLogic(B.isInt(), "bitwiseXor: B is not an integer");
    return A.getInt() ^ B.getInt();
  }

  /**
   * Return the result of A << B
   * Usage: @bitwiseLeftShift(A, B)
   */
  static dynamic bitwiseLeftShiftMacro(Context&& ctx) {
    const auto& A = ctx.at("A");
    const auto& B = ctx.at("B");
    checkLogic(A.isInt(), "bitwiseLeftShift: A is not an integer");
    checkLogic(B.isInt(), "bitwiseLeftShift: B is not an integer");
    return static_cast<uint64_t>(A.getInt()) << B.getInt();
  }

  /**
   * Return the result of A >> B
   * Usage: @bitwiseRightShift(A, B)
   */
  static dynamic bitwiseRightShiftMacro(Context&& ctx) {
    const auto& A = ctx.at("A");
    const auto& B = ctx.at("B");
    checkLogic(A.isInt(), "bitwiseRightShift: A is not an integer");
    checkLogic(B.isInt(), "bitwiseRightShift: B is not an integer");
    return static_cast<uint64_t>(A.getInt()) >> B.getInt();
  }

  /**
   * Returns the complement of A.
   * Usage: @bitwiseNot(A)
   */
  static dynamic bitwiseNotMacro(Context&& ctx) {
    const auto& A = ctx.at("A");
    checkLogic(A.isInt(), "bitwiseNot: A is not an integer");
    return ~A.getInt();
  }

  /**
   * Returns true if A && B. B is evaluated only if A is true.
   * A and B should be booleans.
   * Usage: @and(A,B)
   */
  static dynamic andMacro(Context&& ctx) {
    auto A = ctx.expandRawArg("A");
    checkLogic(A.isBool(), "and: A is {}, expected bool", A.typeName());
    if (A.getBool()) {
      auto B = ctx.expandRawArg("B");
      checkLogic(B.isBool(), "and: B is {}, expected bool", B.typeName());
      return B.getBool();
    }
    return false;
  }

  /**
   * Returns true if A || B. B is evaluated only if A is true.
   * A and B should be booleans.
   * Usage: @or(A,B)
   */
  static dynamic orMacro(Context&& ctx) {
    auto A = ctx.expandRawArg("A");
    checkLogic(A.isBool(), "or: A is {}, expected bool", A.typeName());
    if (!A.getBool()) {
      auto B = ctx.expandRawArg("B");
      checkLogic(B.isBool(), "or: B is {}, expected bool", B.typeName());
      return B.getBool();
    }
    return true;
  }

  /**
   * Returns true if !A. A should be boolean.
   * Usage: @not(A)
   */
  static dynamic notMacro(Context&& ctx) {
    const auto& A = ctx.at("A");
    checkLogic(A.isBool(), "not: A is not bool");
    return !A.getBool();
  }

  /**
   * Returns size of object/array/string.
   * Usage: @size(dictionary)
   */
  static dynamic sizeMacro(Context&& ctx) {
    const auto& dict = ctx.at("dictionary");
    checkLogic(
        dict.isObject() || dict.isArray() || dict.isString(),
        "size: dictionary is not object/array/string");
    return dict.size();
  }

  /**
   * Adds two integers.
   * Usage: @add(A,B)
   */
  static dynamic addMacro(Context&& ctx) {
    const auto& A = ctx.at("A");
    const auto& B = ctx.at("B");
    checkLogic(A.isInt(), "add: A is not an integer");
    checkLogic(B.isInt(), "add: B is not an integer");
    return A.getInt() + B.getInt();
  }

  /**
   * Subtracts two integers.
   * Usage: @sub(A,B)
   */
  static dynamic subMacro(Context&& ctx) {
    const auto& A = ctx.at("A");
    const auto& B = ctx.at("B");
    checkLogic(A.isInt(), "sub: A is not an integer");
    checkLogic(B.isInt(), "sub: B is not an integer");
    return A.getInt() - B.getInt();
  }

  /**
   * Multiplies two integers.
   * Usage: @mul(A,B)
   */
  static dynamic mulMacro(Context&& ctx) {
    const auto& A = ctx.at("A");
    const auto& B = ctx.at("B");
    checkLogic(A.isInt(), "mul: A is not an integer");
    checkLogic(B.isInt(), "mul: B is not an integer");
    return A.getInt() * B.getInt();
  }

  /**
   * Divides two integers.
   * Usage: @div(A,B)
   */
  static dynamic divMacro(Context&& ctx) {
    const auto& A = ctx.at("A");
    const auto& B = ctx.at("B");
    checkLogic(A.isInt(), "div: A is not an integer");
    checkLogic(B.isInt(), "div: B is not an integer");
    checkLogic(B.getInt() != 0, "div: B == 0");
    return A.getInt() / B.getInt();
  }

  /**
   * A % B.
   * Usage: @mod(A,B)
   */
  static dynamic modMacro(Context&& ctx) {
    const auto& A = ctx.at("A");
    const auto& B = ctx.at("B");
    checkLogic(A.isInt(), "mod: A is not an integer");
    checkLogic(B.isInt(), "mod: B is not an integer");
    checkLogic(B.getInt() != 0, "mod: B == 0");
    return A.getInt() % B.getInt();
  }

  /**
   * Throw an exception with custom message
   * Usage: @fail(Your message here)
   */
  static dynamic failMacro(Context&& ctx) {
    const auto& msg = ctx.at("msg");
    if (msg.isString()) {
      throw std::logic_error(msg.getString());
    } else {
      throw std::logic_error(folly::toPrettyJson(msg));
    }
  }

  /**
   * Sort an array of strings/integers
   * Usage: @sort(array)
   */
  static dynamic sortMacro(Context&& ctx) {
    auto dict = ctx.move("dictionary");
    checkLogic(dict.isArray(), "sort: dictionary is not an array");
    std::vector<dynamic> v;
    v.reserve(dict.size());
    for (size_t i = 0; i < dict.size(); ++i) {
      v.push_back(std::move(dict[i]));
    }
    std::sort(v.begin(), v.end());
    for (size_t i = 0; i < v.size(); ++i) {
      dict[i] = std::move(v[i]);
    }
    return dict;
  }

  /**
   * If condition is true, returns "is_true", otherwise "is_false"
   * Usage:
   * "type": "if",
   * "condition": bool,
   * "is_true": any object
   * "is_false": any object
   */
  static dynamic ifMacro(Context&& ctx) {
    auto condition = ctx.expandRawArg("condition");
    checkLogic(condition.isBool(), "If: condition is not bool");
    if (condition.getBool()) {
      try {
        return ctx.expandRawArg("is_true");
      } catch (const std::logic_error& e) {
        throwLogic("If 'is_true':\n{}", e.what());
      }
    } else {
      try {
        return ctx.expandRawArg("is_false");
      } catch (const std::logic_error& e) {
        throwLogic("If 'is_false':\n{}", e.what());
      }
    }
  }

  /**
   * Returns true if name is defined in local context or in consts
   * Usage: @defined(default-route)
   * => true for mcrouter config
   */
  static dynamic definedMacro(Context&& ctx) {
    auto name = asStringPiece(ctx.at("name"), "Defined: name");
    auto& consts = ctx.prep().consts_;
    return (ctx.outer() && ctx.outer()->contains(name)) ||
        consts.find(name) != consts.end();
  }

  /**
   * Return "result" field. Useful with vars, e.g.
   * Usage:
   *  "type": "define",
   *  "vars": { "A": "B" },
   *  "result": "%A%"
   * => "B"
   */
  static dynamic defineMacro(Context&& ctx) {
    return ctx.move("result");
  }

  /**
   * Return true if "ip" is a local address
   * Usage:
   *  @isLocalIp(::1) => true
   *  @isLocalIp(blah) => false
   */
  static dynamic isLocalIpMacro(Context&& ctx) {
    auto compareIp = [](const struct sockaddr* addr, void* res) {
      auto* result = reinterpret_cast<std::pair<folly::IPAddress, bool>*>(res);
      try {
        folly::IPAddress ip(addr);
        if (ip == result->first) {
          result->second = true;
          // found matching ip, no need to continue
          return false;
        }
      } catch (const std::exception&) {
      }
      return true;
    };

    auto ipStr = asStringPiece(ctx.at("ip"), "isLocalIp: ip");
    std::pair<folly::IPAddress, bool> result;
    try {
      result.second = false;
      result.first = folly::IPAddress(ipStr);
    } catch (const std::exception&) {
      // not an ip
      return false;
    }
    if (!for_each_localaddr(compareIp, &result)) {
      throwLogic("Can not enumerate local ips: {}", folly::errnoStr(errno));
    }
    return result.second;
  }

  /**
   * Special built-in that prevents expanding 'macroDef' and 'constDef' objects
   * unless we parse them. For internal use only, nobody should call it
   * explicitly.
   */
  static dynamic noop(dynamic&& json, const Context&) {
    return std::move(json);
  }

  /**
   * Tranforms keys and items of object or elements of list.
   * Usage:
   * "type": "transform",
   * "dictionary": obj,
   * "itemTransform": macro with extended context (optional)
   * "keyTranform": macro with extended context (optional)
   * "itemName": string (optional, default: item)
   * "keyName": string (optional, default: key)
   * or
   * "type": "transform",
   * "dictionary": list,
   * "itemTranform": macro with extended context (required)
   * "keyName": string (optional, default: key)
   * "itemName": string (optional, default: item)
   *
   * Extended context is current context with two additional parameters:
   * %keyName% (key of current entry) and %itemName% (value of current entry)
   */
  static dynamic
  transform(ConfigPreprocessor& p, dynamic&& json, const Context& ctx) {
    auto dictionary =
        p.expandMacros(moveGet(json, "dictionary", "Transform"), ctx);

    checkLogic(
        dictionary.isObject() || dictionary.isArray(),
        "Transform: dictionary is not array/object");

    if (dictionary.empty()) {
      return dictionary;
    }

    auto itemTransform = json.get_ptr("itemTransform");
    StringPiece itemNameStr = "item";
    if (auto itemName = json.get_ptr("itemName")) {
      itemNameStr = asStringPiece(*itemName, "Transform: itemName");
    }
    StringPiece keyNameStr = "key";
    if (auto keyName = json.get_ptr("keyName")) {
      keyNameStr = asStringPiece(*keyName, "Transform: keyName");
    }

    Context extContext(Context::Extended, ctx);
    auto& keyRef = extContext.addExpanded(keyNameStr, nullptr);
    auto& itemRef = extContext.addExpanded(itemNameStr, nullptr);
    if (dictionary.isObject()) {
      auto keyTransform = json.get_ptr("keyTransform");
      dynamic res = dynamic::object();
      for (const auto& it : dictionary.items()) {
        auto& key = const_cast<dynamic&>(it.first);
        auto& value = const_cast<dynamic&>(it.second);
        // add %key% and %item% to current context.
        keyRef = std::move(key);
        itemRef = std::move(value);
        auto nKey =
            keyTransform ? p.expandMacros(*keyTransform, extContext) : keyRef;
        checkLogic(
            nKey.isArray() || nKey.isString(),
            "Transformed key is not array/string");
        if (nKey.isArray() && nKey.empty()) {
          continue;
        }
        auto nItem = itemTransform ? p.expandMacros(*itemTransform, extContext)
                                   : std::move(itemRef);
        if (nKey.isString()) {
          res.insert(std::move(nKey), std::move(nItem));
        } else { // array
          for (auto& keyIt : nKey) {
            checkLogic(
                keyIt.isString(), "Transformed key list item is not a string");
            res.insert(std::move(keyIt), nItem);
          }
        }
      }
      return res;
    } else { // array
      checkLogic(
          itemTransform, "Transform: itemTransform is required for array");
      for (size_t index = 0; index < dictionary.size(); ++index) {
        auto& item = dictionary[index];
        // add %key% and %item% to current context.
        keyRef = index;
        itemRef = std::move(item);
        item = p.expandMacros(*itemTransform, extContext);
      }
      return dictionary;
    }
  }

  /**
   * Iterates over object or array and transforms "value". Literally:
   *
   *   value = initialValue
   *   for (auto& it : dictionary) {
   *     value = transform(it.first, it.second, value)
   *   }
   *   return value
   *
   * Usage:
   * "type": "process",
   * "initialValue": any value,
   * "transform": macro with extended context
   * "keyName": string (optional, default: key)
   * "itemName": string (optional, default: item)
   * "valueName": string (optional, default: value)
   *
   * Extended context is current context with three additional parameters:
   * %keyName% (key of current entry), %itemName% (value of current entry)
   * and %valueName% - current value.
   */
  static dynamic
  process(ConfigPreprocessor& p, dynamic&& json, const Context& ctx) {
    auto dictionary =
        p.expandMacros(moveGet(json, "dictionary", "Process"), ctx);
    auto value = p.expandMacros(moveGet(json, "initialValue", "Process"), ctx);
    const auto& transform = tryGet(json, "transform", "Process");

    checkLogic(
        dictionary.isObject() || dictionary.isArray(),
        "Process: dictionary is not array/object");

    StringPiece itemNameStr = "item";
    if (auto itemName = json.get_ptr("itemName")) {
      itemNameStr = asStringPiece(*itemName, "Process: itemName");
    }
    StringPiece keyNameStr = "key";
    if (auto keyName = json.get_ptr("keyName")) {
      keyNameStr = asStringPiece(*keyName, "Process: keyName");
    }
    StringPiece valueNameStr = "value";
    if (auto valueName = json.get_ptr("valueName")) {
      valueNameStr = asStringPiece(*valueName, "Process: valueName");
    }

    Context extContext(Context::Extended, ctx);
    auto& keyRef = extContext.addExpanded(keyNameStr, nullptr);
    auto& itemRef = extContext.addExpanded(itemNameStr, nullptr);
    auto& valueRef = extContext.addExpanded(valueNameStr, nullptr);
    if (dictionary.isObject()) {
      for (auto& item : dictionary.items()) {
        auto& key = const_cast<dynamic&>(item.first);
        auto& val = const_cast<dynamic&>(item.second);
        // add %key%, %item% and %value% to current context.
        keyRef = std::move(key);
        itemRef = std::move(val);
        valueRef = std::move(value);
        value = p.expandMacros(transform, extContext);
      }
      return value;
    } else { // array
      for (size_t index = 0, e = dictionary.size(); index < e; ++index) {
        auto& item = dictionary[index];
        // add %key%, %item% and %value% to current context.
        keyRef = index;
        itemRef = std::move(item);
        valueRef = std::move(value);
        value = p.expandMacros(transform, extContext);
      }
      return value;
    }
  }

  /**
   * foreach (key, item) from <from> where <where> use <use> top <int>
   * for top <top> items from dictionary "from" which satisfy "where"
   * condition merge <use> expansions into one dictionary.
   *
   * Usage:
   *  "type": "foreach",
   *  "key": string (optional, default: key)
   *  "item": string (optional, default: item)
   *  "from": object or list
   *  "where": macro with extended context (optional, %key% and %item%)
   *  "use": macro with extended context (optional, %key% and %item%)
   *  "top": int (optional)
   *  "noMatchResult": any value (optional)
   *
   * Example:
   * filter dictionary:
   *  "type": "foreach",
   *  "from": <dictionary>,
   *  "where": <condition>
   *
   * Convert object to list:
   *  "type": "foreach",
   *  "from": <object>,
   *  "use": [ <list item> ]
   *  "noMatchResult": []
   *
   * Grab at most 2 items from <dictionary> that satisfy <condition>:
   *  "type": "foreach",
   *  "from": <dictionary>
   *  "where": <condition>
   *  "top": 2
   */
  static dynamic
  foreach(ConfigPreprocessor& p, dynamic&& json, const Context& ctx) {
    auto from = p.expandMacros(moveGet(json, "from", "Foreach"), ctx);
    checkLogic(
        from.isObject() || from.isArray(), "Foreach: from is not object/array");
    StringPiece itemStr = "item";
    if (auto jItem = json.get_ptr("item")) {
      itemStr = asStringPiece(*jItem, "Foreach: item");
    }
    StringPiece keyStr = "key";
    if (auto jKey = json.get_ptr("key")) {
      keyStr = asStringPiece(*jKey, "Foreach: key");
    }
    size_t top = from.size();
    if (auto jTopField = json.get_ptr("top")) {
      auto jtop = p.expandMacros(std::move(*jTopField), ctx);
      checkLogic(
          jtop.isInt() && jtop.getInt() >= 0,
          "Foreach: top should be a non-negative integer");
      top = jtop.getInt();
    }
    auto useIt = json.find("use");
    auto whereIt = json.find("where");

    dynamic result = nullptr;
    Context extContext(Context::Extended, ctx);
    auto& keyRef = extContext.addExpanded(keyStr, nullptr);
    auto& itemRef = extContext.addExpanded(itemStr, nullptr);

    auto appendUseToResult = [&]() {
      auto use = p.expandMacros(useIt->second, extContext);
      if (result.isNull()) {
        checkLogic(
            use.isObject() || use.isArray(),
            "Foreach: expanded item is not object/array");
        result = std::move(use);
      } else {
        if (result.isObject()) {
          checkLogic(use.isObject(), "Foreach: expanded item is not an object");
          for (auto& it : use.items()) {
            auto& key = const_cast<dynamic&>(it.first);
            auto& value = const_cast<dynamic&>(it.second);
            result.insert(std::move(key), std::move(value));
          }
        } else if (result.isArray()) {
          checkLogic(use.isArray(), "Foreach: expanded item is not an array");
          for (size_t i = 0; i < use.size(); ++i) {
            result.push_back(std::move(use[i]));
          }
        }
      }
    };

    auto satisfiesWhere = [&]() {
      if (whereIt == json.items().end()) {
        return true;
      }
      auto where = p.expandMacros(whereIt->second, extContext);
      checkLogic(where.isBool(), "Foreach: expanded 'where' is not boolean");
      return where.getBool();
    };

    if (from.isArray()) {
      for (size_t i = 0; i < from.size() && top > 0; ++i) {
        keyRef = i;
        itemRef = std::move(from[i]);
        if (!satisfiesWhere()) {
          continue;
        }

        if (useIt == json.items().end()) {
          if (result.isNull()) {
            result = dynamic::array(std::move(itemRef));
          } else {
            result.push_back(std::move(itemRef));
          }
        } else {
          appendUseToResult();
        }
        --top;
      }
    } else { // object
      for (auto& curIt : from.items()) {
        if (top == 0) {
          break;
        }
        auto& curKey = const_cast<dynamic&>(curIt.first);
        auto& curItem = const_cast<dynamic&>(curIt.second);

        keyRef = std::move(curKey);
        itemRef = std::move(curItem);
        if (!satisfiesWhere()) {
          continue;
        }

        if (useIt == json.items().end()) {
          if (result.isNull()) {
            result = dynamic::object(std::move(keyRef), std::move(itemRef));
          } else {
            result.insert(std::move(keyRef), std::move(itemRef));
          }
        } else {
          appendUseToResult();
        }
        --top;
      }
    }
    if (result.isNull()) {
      if (auto jnoMatchResult = json.get_ptr("noMatchResult")) {
        return p.expandMacros(std::move(*jnoMatchResult), ctx);
      }
      return from.isObject() ? dynamic::object() : dynamic::array();
    }
    return result;
  }
};

///////////////////////////////ConfigPreprocessor///////////////////////////////

ConfigPreprocessor::ConfigPreprocessor(
    ImportResolverIf& importResolver,
    folly::F14NodeMap<std::string, dynamic> globals,
    folly::json::metadata_map& configMetadataMap,
    size_t nestedLimit)
    : configMetadataMap_(configMetadataMap), nestedLimit_(nestedLimit) {
  for (auto& it : globals) {
    addConst(it.first, std::move(it.second));
  }

  addMacro(
      "import",
      {"path", dynamic::object("name", "default")("optional", true)},
      [this, &importResolver](Context&& ctx) {
        return BuiltIns::importMacro(*this, importResolver, std::move(ctx));
      },
      false);

  addMacro("hash", {"value"}, &BuiltIns::hashMacro);

  addMacro("weightedHash", {"dictionary", "key"}, &BuiltIns::weightedHashMacro);

  addMacro("int", {"value"}, &BuiltIns::intMacro);

  addMacro("double", {"value"}, &BuiltIns::doubleMacro);

  addMacro("str", {"value"}, &BuiltIns::strMacro);

  addMacro("bool", {"value"}, &BuiltIns::boolMacro);

  addMacro("keys", {"dictionary"}, &BuiltIns::keysMacro);

  addMacro("values", {"dictionary"}, &BuiltIns::valuesMacro);

  addMacro("merge", {"params"}, &BuiltIns::mergeMacro);

  addMacro(
      "select",
      {"dictionary",
       "key",
       dynamic::object("name", "default")("optional", true)},
      &BuiltIns::selectMacro,
      false);

  addMacro(
      "shuffle",
      {"dictionary", dynamic::object("name", "seed")("optional", true)},
      &BuiltIns::shuffleMacro,
      false);

  addMacro("slice", {"dictionary", "from", "to"}, &BuiltIns::sliceMacro);

  addMacro("split", {"dictionary", "delim"}, &BuiltIns::splitMacro);

  addMacro("range", {"from", "to"}, &BuiltIns::rangeMacro);

  addMacro("contains", {"dictionary", "key"}, &BuiltIns::containsMacro);

  addMacro("empty", {"dictionary"}, &BuiltIns::emptyMacro);

  addMacro("equals", {"A", "B"}, &BuiltIns::equalsMacro);

  addMacro("isArray", {"value"}, &BuiltIns::isArrayMacro);

  addMacro("isBool", {"value"}, &BuiltIns::isBoolMacro);

  addMacro("isInt", {"value"}, &BuiltIns::isIntMacro);

  addMacro("isDouble", {"value"}, &BuiltIns::isDoubleMacro);

  addMacro("isObject", {"value"}, &BuiltIns::isObjectMacro);

  addMacro("isString", {"value"}, &BuiltIns::isStringMacro);

  addMacro("less", {"A", "B"}, &BuiltIns::lessMacro);

  addMacro("bitwiseAnd", {"A", "B"}, &BuiltIns::bitwiseAndMacro);

  addMacro("bitwiseOr", {"A", "B"}, &BuiltIns::bitwiseOrMacro);

  addMacro("bitwiseXor", {"A", "B"}, &BuiltIns::bitwiseXorMacro);

  addMacro("bitwiseNot", {"A"}, &BuiltIns::bitwiseNotMacro);

  addMacro("bitwiseLeftShift", {"A", "B"}, &BuiltIns::bitwiseLeftShiftMacro);

  addMacro("bitwiseRightShift", {"A", "B"}, &BuiltIns::bitwiseRightShiftMacro);

  addMacro("and", {"A", "B"}, &BuiltIns::andMacro, false);

  addMacro("or", {"A", "B"}, &BuiltIns::orMacro, false);

  addMacro("not", {"A"}, &BuiltIns::notMacro);

  addMacro("size", {"dictionary"}, &BuiltIns::sizeMacro);

  addMacro("add", {"A", "B"}, &BuiltIns::addMacro);

  addMacro("sub", {"A", "B"}, &BuiltIns::subMacro);

  addMacro("mul", {"A", "B"}, &BuiltIns::mulMacro);

  addMacro("div", {"A", "B"}, &BuiltIns::divMacro);

  addMacro("mod", {"A", "B"}, &BuiltIns::modMacro);

  addMacro("fail", {"msg"}, &BuiltIns::failMacro);

  addMacro("sort", {"dictionary"}, &BuiltIns::sortMacro);

  addMacro("set", {"dictionary", "key", "value"}, &BuiltIns::setMacro);

  addMacro(
      "if", {"condition", "is_true", "is_false"}, &BuiltIns::ifMacro, false);

  addMacro("define", {"result"}, &BuiltIns::defineMacro);

  addMacro("defined", {"name"}, &BuiltIns::definedMacro);

  addMacro("isLocalIp", {"ip"}, &BuiltIns::isLocalIpMacro);

  builtInCalls_.emplace("macroDef", &BuiltIns::noop);

  builtInCalls_.emplace("constDef", &BuiltIns::noop);

  builtInCalls_.emplace(
      "transform", [this](dynamic&& json, const Context& ctx) {
        return BuiltIns::transform(*this, std::move(json), ctx);
      });

  builtInCalls_.emplace("process", [this](dynamic&& json, const Context& ctx) {
    return BuiltIns::process(*this, std::move(json), ctx);
  });

  builtInCalls_.emplace("foreach", [this](dynamic&& json, const Context& ctx) {
    return BuiltIns::foreach(*this, std::move(json), ctx);
  });
}

void ConfigPreprocessor::addConst(StringPiece name, folly::dynamic result) {
  auto it = consts_.emplace(name, nullptr).first;
  it->second = std::make_unique<Const>(*this, it->first, std::move(result));
}

void ConfigPreprocessor::addMacro(
    StringPiece name,
    const std::vector<dynamic>& params,
    Macro::Func func,
    bool autoExpand) {
  auto it = macros_.emplace(name, nullptr).first;
  it->second = std::make_unique<Macro>(
      *this, it->first, params, std::move(func), autoExpand);
}

dynamic ConfigPreprocessor::replaceParams(
    StringPiece str,
    const Context& context) const {
  string buf;
  while (!str.empty()) {
    auto pos = unescapeUntil(str, buf, '%');
    if (pos == string::npos) {
      break;
    }
    str = str.subpiece(pos + 1);

    string paramName;
    auto nextPos = unescapeUntil(str, paramName, '%');
    checkLogic(
        nextPos != string::npos,
        "Odd number of percent signs in string around '{}'",
        str);

    // first check current context, then global context
    auto paramPtr = context.find(paramName);
    const auto& substitution = paramPtr
        ? *paramPtr
        : tryGet(consts_, paramName, "Param in string")->getResult();

    if (buf.empty() && nextPos == str.size() - 1) {
      // whole string is a parameter. May be substituted to any value.
      return substitution;
    } else {
      // param inside string e.g. a%param%b
      checkLogic(
          substitution.isString(),
          "Param in string '{}' is not a string",
          paramName);
      auto sp = substitution.stringPiece();
      buf.append(sp.begin(), sp.end());
    }
    str = str.subpiece(nextPos + 1);
  }
  return buf;
}

std::vector<StringPiece> ConfigPreprocessor::getCallParams(
    StringPiece str) const {
  // all params are substituted. But inner macro calls are possible.
  std::vector<StringPiece> result;
  while (true) {
    if (str.empty()) {
      // it is one parameter - empty string e.g. @a() is call with one parameter
      result.emplace_back("");
      break;
    }

    auto commaPos = findUnescaped(str, ',');
    if (commaPos == string::npos) {
      // no commas - only one param;
      result.push_back(trim(str));
      break;
    }

    auto firstOpened = findUnescaped(str.subpiece(0, commaPos), '(');
    // macro call with params
    if (firstOpened != string::npos) {
      // first param is inner macro with params
      auto closing = matchingUnescapedBracket(str, firstOpened);
      checkLogic(closing != string::npos, "Brackets do not match: {}", str);

      if (closing == str.size() - 1) {
        // no more params, only single inner macro
        result.push_back(trim(str));
        break;
      }

      commaPos = closing + 1;
      while (commaPos < str.size() && str[commaPos] != ',') {
        ++commaPos;
      }
      checkLogic(commaPos != str.size(), "No comma after closing bracket");
    }

    // add first param
    auto firstParam = str.subpiece(0, commaPos);
    result.push_back(trim(firstParam));
    // and process next params
    str = str.subpiece(commaPos + 1);
  }

  return result;
}

dynamic ConfigPreprocessor::expandStringMacro(
    StringPiece str,
    const Context& context) const {
  NestedLimitGuard nestedGuard(nestedLimit_);

  if (str.empty()) {
    return "";
  }

  if (str[0] != '@') {
    return replaceParams(str, context);
  }

  // macro in string
  StringPiece name;
  std::vector<StringPiece> innerParams;

  auto paramStart = findUnescaped(str, '(');
  if (paramStart != string::npos) {
    // macro in string with params e.g. @a()
    checkLogic(
        str[str.size() - 1] == ')', "No closing bracket for call '{}'", str);

    name = str.subpiece(1, paramStart - 1);

    // get parameters of this macro
    auto innerStr = str.subpiece(paramStart + 1, str.size() - paramStart - 2);
    innerParams = getCallParams(innerStr);
  } else {
    // macro in string without params e.g. @a
    name = str.subpiece(1);
  }

  auto substName = replaceParams(name, context);
  auto nameStr = asStringPiece(substName, "Macro name");

  const auto& inner = tryGet(macros_, nameStr, "Macro");
  try {
    return inner->getResult(innerParams, context);
  } catch (const std::logic_error& e) {
    throwLogic("Macro in string '{}':\n{}", nameStr, e.what());
  }
}

dynamic ConfigPreprocessor::expandMacros(dynamic json, const Context& context)
    const {
  NestedLimitGuard nestedGuard(nestedLimit_);

  if (json.isString()) {
    // look for macros in string
    return expandStringMacro(json.stringPiece(), context);
  } else if (json.isObject()) {
    folly::Optional<Context> extContext;
    if (auto jVars = json.get_ptr("vars")) {
      checkLogic(
          jVars->isObject(), "vars is {}, expected object", jVars->typeName());

      extContext.emplace(Context::Extended, context);
      // since vars may use other vars from local context, we
      // do the initialization in two steps: first add them to context,
      // then lazily expand
      for (auto& it : jVars->items()) {
        auto var = const_cast<dynamic&>(it.second);
        extContext->addLocal(it.first.stringPiece(), std::move(var));
      }
      for (const auto& varName : jVars->keys()) {
        extContext->doLazyExpand(varName.stringPiece());
      }
      json.erase("vars");
    }
    const auto& localContext = extContext ? *extContext : context;

    // check for built-in calls and long-form macros
    auto typeIt = json.find("type");
    if (typeIt != json.items().end()) {
      auto type = expandMacros(typeIt->second, localContext);
      if (type.isString()) {
        auto typeStr = type.stringPiece();
        // built-in call
        auto builtInIt = builtInCalls_.find(typeStr);
        if (builtInIt != builtInCalls_.end()) {
          try {
            return builtInIt->second(std::move(json), localContext);
          } catch (const std::logic_error& e) {
            throwLogic("Built-in '{}':\n{}", typeStr, e.what());
          }
        }
        // long form macro substitution
        auto macroIt = macros_.find(typeStr);
        if (macroIt != macros_.end()) {
          const auto& inner = macroIt->second;
          try {
            return inner->getResult(std::move(json), localContext);
          } catch (const std::logic_error& e) {
            throwLogic("Macro '{}':\n{}", typeStr, e.what());
          }
        }
      }
    }

    // raw object
    dynamic result = dynamic::object();
    for (const auto& it : json.items()) {
      auto& value = const_cast<dynamic&>(it.second);
      try {
        auto nKey = expandMacros(it.first, localContext);
        checkLogic(nKey.isString(), "Expanded key is not a string");
        result.insert(
            std::move(nKey), expandMacros(std::move(value), localContext));
        // Since new json is being created with expanded macros we need
        // to re-populate the config metadata map with new dynamic objects
        // created in the process.
        const auto nKeyPtr = result.get_ptr(it.first);
        const auto nKeyJsonPtr = json.get_ptr(it.first);
        if (nKeyPtr && nKeyJsonPtr) {
          const auto resMetadataPtr = configMetadataMap_.find(nKeyJsonPtr);
          const auto jsonMetadataPtr = configMetadataMap_.find(nKeyPtr);
          if (resMetadataPtr != configMetadataMap_.end()) {
            // If it already exists in the map, replace it
            // Otherwise, create an entry in the map
            if (jsonMetadataPtr == configMetadataMap_.end()) {
              configMetadataMap_.emplace(nKeyPtr, resMetadataPtr->second);
            } else {
              jsonMetadataPtr->second = resMetadataPtr->second;
            }
          }
        }
      } catch (const std::logic_error& e) {
        throwLogic(
            "Raw object property '{}':\n{}", it.first.stringPiece(), e.what());
      }
    }
    return result;
  } else if (json.isArray()) {
    for (size_t i = 0, e = json.size(); i < e; ++i) {
      auto& value = json[i];
      try {
        value = expandMacros(std::move(value), context);
      } catch (const std::logic_error& e) {
        throwLogic("Array element #{}:\n{}", i, e.what());
      }
    }
    return json;
  } else {
    // some number or other type of json. Return 'as is'.
    return json;
  }
}

void ConfigPreprocessor::parseMacroDef(
    const dynamic& jkey,
    const dynamic& obj) {
  auto key = asStringPiece(jkey, "macro definition key");
  checkLogic(obj.isObject(), "'{}' macro definition is not an object", key);
  auto objType = asStringPiece(
      tryGet(obj, "type", "macro definition"), "macro definition type");

  if (objType == "macroDef") {
    const auto& res = tryGet(obj, "result", "Macro definition");
    std::vector<dynamic> params;
    auto paramsIt = obj.find("params");
    if (paramsIt != obj.items().end()) {
      checkLogic(
          paramsIt->second.isArray(),
          "'{}' macroDef params is not an array",
          key);
      for (auto& paramObj : paramsIt->second) {
        params.push_back(paramObj);
      }
    }
    auto f = [res, this](Context&& ctx) {
      return expandMacros(res, std::move(ctx));
    };
    addMacro(key, params, std::move(f));
  } else if (objType == "constDef") {
    checkLogic(obj.isObject(), "constDef is not an object");
    addConst(key, tryGet(obj, "result", "constDef"));
  } else {
    throwLogic("Unknown macro definition type: {}", objType);
  }
}

void ConfigPreprocessor::parseMacroDefs(dynamic jmacros) {
  auto macros = expandMacros(std::move(jmacros), Context(*this));
  checkLogic(
      macros.isObject() || macros.isArray(),
      "config macros is not an array/object");

  if (macros.isObject()) {
    for (const auto& it : macros.items()) {
      parseMacroDef(it.first, it.second);
    }
  } else { // array
    for (const auto& it : macros) {
      parseMacroDefs(it);
    }
  }
}

dynamic ConfigPreprocessor::getConfigWithoutMacros(
    StringPiece jsonC,
    ImportResolverIf& importResolver,
    folly::F14NodeMap<std::string, dynamic> globalParams,
    folly::json::metadata_map* configMetadataMap,
    size_t nestedLimit) {
  auto config = parseJsonString(stripComments(jsonC), configMetadataMap);
  checkLogic(config.isObject(), "config is not an object");

  ConfigPreprocessor prep(
      importResolver, std::move(globalParams), *configMetadataMap, nestedLimit);

  // parse and add macros
  auto jmacros = config.get_ptr("macros");
  if (jmacros) {
    prep.parseMacroDefs(std::move(*jmacros));
    config.erase("macros");
  }

  return prep.expandMacros(std::move(config), Context(prep));
}

} // namespace memcache
} // namespace facebook
