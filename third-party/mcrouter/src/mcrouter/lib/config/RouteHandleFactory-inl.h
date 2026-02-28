/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/json/dynamic.h>

#include "mcrouter/lib/config/RouteHandleProviderIf.h"
#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {

template <class RouteHandleIf>
RouteHandleFactory<RouteHandleIf>::RouteHandleFactory(
    RouteHandleProviderIf<RouteHandleIf>& provider,
    size_t threadId)
    : provider_(provider), threadId_(threadId) {}

template <class RouteHandleIf>
void RouteHandleFactory<RouteHandleIf>::addNamed(
    const folly::StringPiece name,
    const folly::dynamic& json) {
  if (json.isObject()) {
    const auto jName = json.get_ptr("name");
    checkLogic(
        jName && jName->isString() && jName->stringPiece() == name,
        "Ambiguous RouteHandle name in object for {}",
        name);
  }
  checkLogic(
      registered_.emplace(name, &json).second,
      "Route handle '{}' was already registered",
      name);
}

template <class RouteHandleIf>
std::shared_ptr<RouteHandleIf> RouteHandleFactory<RouteHandleIf>::create(
    const folly::dynamic& json) {
  auto result = createList(json);

  checkLogic(
      result.size() == 1, "{} RouteHandles in list, expected 1", result.size());

  return std::move(result.back());
}

template <class RouteHandleIf>
const std::vector<std::shared_ptr<RouteHandleIf>>&
RouteHandleFactory<RouteHandleIf>::createNamed(
    folly::StringPiece name,
    const folly::dynamic& json) {
  auto seenIt = seen_.find(name);
  if (seenIt != seen_.end()) {
    // we had the same named handle already. Reuse it.
    return seenIt->second;
  }

  // check if this name was registered
  auto registeredIt = registered_.find(name);
  if (registeredIt != registered_.end()) {
    auto const tmp = registeredIt->second;
    registered_.erase(registeredIt);
    return createNamedImpl(name, *tmp);
  }

  return createNamedImpl(name, json);
}

template <class RouteHandleIf>
const std::vector<std::shared_ptr<RouteHandleIf>>&
RouteHandleFactory<RouteHandleIf>::createNamedImpl(
    folly::StringPiece name,
    const folly::dynamic& json) {
  if (json.isObject()) {
    auto jType = json.get_ptr("type");
    checkLogic(jType, "No type field in RouteHandle json object");
    checkLogic(jType->isString(), "Type field in RouteHandle is not a string");
    auto ret = provider_.create(*this, jType->stringPiece(), json);
    return seen_.emplace(name, std::move(ret)).first->second;
  }

  return seen_.emplace(name, createList(json)).first->second;
}

template <class RouteHandleIf>
std::vector<std::shared_ptr<RouteHandleIf>>
RouteHandleFactory<RouteHandleIf>::createList(const folly::dynamic& json) {
  if (json.isArray()) {
    std::vector<RouteHandlePtr> ret;
    // merge all inner lists into result
    for (const auto& it : json) {
      for (auto& listIt : createList(it)) {
        ret.push_back(std::move(listIt));
      }
    }
    ret.shrink_to_fit();
    return ret;
  } else if (json.isObject()) {
    auto jName = json.get_ptr("name");
    if (jName && jName->isString()) {
      return createNamed(jName->stringPiece(), json);
    } else {
      auto jType = json.get_ptr("type");
      checkLogic(jType, "No type field in RouteHandle json object");
      checkLogic(
          jType->isString(), "Type field in RouteHandle is not a string");
      return provider_.create(*this, jType->stringPiece(), json);
    }
  } else if (json.isString()) {
    if (json.empty()) {
      // useful for routes with optional children
      return {};
    }

    // check if we need to use a pre-built list of children.
    constexpr folly::StringPiece kChildrenListStr = "$children_list$";
    if (json.stringPiece() == kChildrenListStr) {
      checkLogic(
          !childrenLists_.empty(),
          "$children_list$ was found, but there were no pre-constructed "
          "children available. Did you forget to call pushChildrenList()?");
      return childrenLists_.top();
    }

    // check if we already parsed the same string. It can be named handle or
    // short form handle.
    auto handlePiece = json.stringPiece();
    auto seenIt = seen_.find(handlePiece);
    if (seenIt != seen_.end()) {
      return seenIt->second;
    }

    // check if this name was registered
    auto registeredIt = registered_.find(handlePiece);
    if (registeredIt != registered_.end()) {
      auto const tmp = registeredIt->second;
      registered_.erase(registeredIt);
      return createNamedImpl(handlePiece, *tmp);
    }

    std::vector<RouteHandlePtr> ret;
    auto pipeId = handlePiece.find("|");
    if (pipeId != std::string::npos) { // short form (e.g. HashRoute|ErrorRoute)
      auto type = handlePiece.subpiece(0, pipeId); // split by first '|'
      auto def = handlePiece.subpiece(pipeId + 1);
      ret = provider_.create(*this, type, def);
    } else {
      // assume it is a short form of route without children (e.g. ErrorRoute)
      ret = provider_.create(*this, handlePiece, nullptr);
    }

    seen_.emplace(handlePiece, ret);
    return ret;
  }
  throwLogic(
      "RouteHandle is {}, expected object/array/string", json.typeName());
}

template <class RouteHandleIf>
const folly::dynamic& RouteHandleFactory<RouteHandleIf>::parsePool(
    const folly::dynamic& json) {
  return provider_.parsePool(json);
}

template <class RouteHandleIf>
void RouteHandleFactory<RouteHandleIf>::pushChildrenList(
    std::vector<RouteHandlePtr> children) {
  childrenLists_.push(std::move(children));
}

template <class RouteHandleIf>
void RouteHandleFactory<RouteHandleIf>::popChildrenList() {
  assert(!childrenLists_.empty());
  childrenLists_.pop();
}

} // namespace memcache
} // namespace facebook
