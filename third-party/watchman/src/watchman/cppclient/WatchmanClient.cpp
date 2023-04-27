/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WatchmanClient.h"

#include <fmt/core.h>
#include <glog/logging.h>

namespace watchman {

using namespace folly;

WatchmanClient::~WatchmanClient() {
  // We need to explicitly close the connection so in the case that it outlives
  // us it doesn't accidentally call our callback.
  conn_->close();
}

WatchmanClient::WatchmanClient(
    EventBase* eventBase,
    std::optional<std::string>&& socketPath,
    folly::Executor* cpuExecutor,
    ErrorCallback errorCallback)
    : conn_(std::make_shared<WatchmanConnection>(
          eventBase,
          std::move(socketPath),
          std::optional<WatchmanConnection::Callback>(
              [this](Try<dynamic>&& data) {
                connectionCallback(std::move(data));
              }),
          cpuExecutor)),
      errorCallback_(errorCallback) {}

void WatchmanClient::connectionCallback(Try<dynamic>&& try_data) {
  // If an exception occurs notify all subscription callbacks. Other outstanding
  // one-shots etc. will get exceptions returned via their futures if needed.
  if (try_data.hasException()) {
    for (auto& subscription : subscriptionMap_) {
      subscription.second->executor_->add(
          [sub_ptr = subscription.second, try_data]() mutable {
            if (sub_ptr->active_) {
              sub_ptr->callback_(std::move(try_data));
            }
          });
    }
    if (errorCallback_) {
      errorCallback_(try_data.exception());
    }
    return;
  }

  auto& data = try_data.value();
  auto subscription_data = data.get_ptr("subscription");
  if (subscription_data) {
    auto subscription = [&]() -> SubscriptionPtr {
      std::lock_guard<std::mutex> guard(mutex_);
      auto subscriptionIt =
          subscriptionMap_.find(subscription_data->asString());
      if (subscriptionIt == subscriptionMap_.end()) {
        return nullptr;
      } else {
        return subscriptionIt->second;
      }
    }();
    if (!subscription) {
      LOG(ERROR) << "Unexpected subscription update: "
                 << subscription_data->asString();
    } else {
      auto exec = subscription->executor_;
      exec->add([sub_ptr = std::move(subscription),
                 data = std::move(data)]() mutable {
        if (sub_ptr->active_) {
          sub_ptr->callback_(Try<dynamic>(std::move(data)));
        }
      });
    }
  } else {
    LOG(ERROR) << "Unhandled unilateral data: " << data;
  }
}

SemiFuture<dynamic> WatchmanClient::connect(dynamic versionArgs) {
  return conn_->connect(versionArgs);
}

void WatchmanClient::close() {
  return conn_->close();
}

SemiFuture<dynamic> WatchmanClient::run(const dynamic& cmd) {
  return conn_->run(cmd);
}

Future<WatchPathPtr> WatchmanClient::watchImpl(std::string_view path) {
  return conn_->run(dynamic::array("watch-project", path))
      .thenValue([=](dynamic&& data) {
        auto relative_path = data["relative_path"];
        std::optional<std::string> relative_path_optional;
        if (relative_path != nullptr) {
          relative_path_optional = relative_path.asString();
        }
        return std::make_shared<WatchPath>(
            data["watch"].asString(), relative_path_optional);
      });
}

SemiFuture<WatchPathPtr> WatchmanClient::watch(std::string_view path) {
  return watchImpl(path).semi();
}

SemiFuture<std::string> WatchmanClient::getClock(WatchPathPtr path) {
  return conn_->run(dynamic::array("clock", path->root_))
      .thenValue([](dynamic data) { return data["clock"].asString(); });
}

SemiFuture<QueryResult> WatchmanClient::query(
    dynamic queryObj,
    WatchPathPtr path) {
  if (path->relativePath_) {
    queryObj["relative_root"] = *path->relativePath_;
  }
  return run(dynamic::array("query", path->root_, std::move(queryObj)))
      .deferValue(
          [](folly::dynamic&& res) { return QueryResult{std::move(res)}; });
}

SemiFuture<SubscriptionPtr> WatchmanClient::subscribe(
    dynamic query,
    WatchPathPtr path,
    Executor* executor,
    SubscriptionCallback&& callback,
    std::string subscriptionName) {
  auto name = subscriptionName.empty() ? fmt::format("sub{}", (int)++nextSubID_)
                                       : std::move(subscriptionName);
  auto subscription =
      std::make_shared<Subscription>(executor, std::move(callback), name, path);
  {
    std::lock_guard<std::mutex> guard(mutex_);
    subscriptionMap_[name] = subscription;
  }
  if (path->relativePath_) {
    query["relative_root"] = *(path->relativePath_);
  }
  return run(dynamic::array("subscribe", path->root_, name, query))
      .deferValue([subscription = std::move(subscription),
                   name = name](const dynamic& data) {
        CHECK(data["subscribe"] == name)
            << "Unexpected response to subscribe request " << data;
        return subscription;
      });
}

SemiFuture<SubscriptionPtr> WatchmanClient::subscribe(
    const dynamic& query,
    std::string_view path,
    Executor* executor,
    SubscriptionCallback&& callback,
    std::string subscriptionName) {
  return watchImpl(path).thenValue(
      [=,
       callback = std::move(callback),
       subscriptionName =
           std::move(subscriptionName)](WatchPathPtr watch_path) mutable {
        return subscribe(
            query,
            watch_path,
            executor,
            std::move(callback),
            std::move(subscriptionName));
      });
}

SemiFuture<dynamic> WatchmanClient::flushSubscription(
    SubscriptionPtr sub,
    std::chrono::milliseconds timeout) {
  CHECK(sub->active_) << "Not subscribed.";

  dynamic args = dynamic::object;
  args["sync_timeout"] = timeout.count();
  args["subscriptions"] = dynamic::array(sub->name_);
  return run(
      dynamic::array("flush-subscriptions", sub->watchPath_->root_, args));
}

SemiFuture<dynamic> WatchmanClient::unsubscribe(SubscriptionPtr sub) {
  CHECK(sub->active_) << "Already unsubscribed.";

  sub->active_ = false;
  return conn_
      ->run(dynamic::array("unsubscribe", sub->watchPath_->root_, sub->name_))
      .ensure([=] {
        std::lock_guard<std::mutex> guard(mutex_);
        subscriptionMap_.erase(sub->name_);
      })
      .semi();
}

Subscription::Subscription(
    Executor* executor,
    SubscriptionCallback&& callback,
    const std::string& name,
    WatchPathPtr watchPath)
    : executor_(executor),
      callback_(std::move(callback)),
      name_(name),
      watchPath_(watchPath) {}

WatchPath::WatchPath(
    const std::string& root,
    const std::optional<std::string>& relativePath)
    : root_(root), relativePath_(relativePath) {}

} // namespace watchman
