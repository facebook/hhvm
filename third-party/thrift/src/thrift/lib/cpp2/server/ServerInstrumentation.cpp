/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/server/ServerInstrumentation.h>

#include <map>
#include <set>

#include <folly/MapUtil.h>
#include <folly/Singleton.h>
#include <folly/Synchronized.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>

namespace apache::thrift::instrumentation {
namespace {

class TrackerCollection {
 private:
  using Map = std::map<std::string, std::set<ServerTracker*>, std::less<>>;
  folly::Synchronized<Map> servers_;

 public:
  TrackerCollection() = default;
  TrackerCollection(const TrackerCollection&) = delete;
  void operator=(const TrackerCollection&) = delete;

  void forTrackersWithKey(
      std::string_view key,
      folly::FunctionRef<void(const std::set<ServerTracker*>&)> f) const {
    servers_.withRLock([&](const Map& map) {
      if (auto* trackers = folly::get_ptr(map, key)) {
        f(*trackers);
      }
    });
  }

  void addTracker(std::string_view key, ServerTracker& tracker) {
    servers_.withWLock(
        [&](Map& map) { map[std::string(key)].insert(&tracker); });
  }

  void removeTracker(std::string_view key, ServerTracker& tracker) {
    servers_.withWLock(
        [&](Map& map) { map[std::string(key)].erase(&tracker); });
  }
};

folly::LeakySingleton<TrackerCollection> trackerCollection;

TrackerCollection& getTrackerCollection() {
  return trackerCollection.get();
}

} // namespace

ServerTracker::ServerTracker(std::string_view key, ThriftServer& server)
    : key_(key),
      server_(server),
      cb_(std::make_unique<ServerTrackerRef::ControlBlock>(key_, server_)) {
  getTrackerCollection().addTracker(key_, *this);
  getLoggingEventRegistry().getServerTrackerHandler(key_).log(*this);
}

ServerTracker::~ServerTracker() {
  cb_.join();
  getTrackerCollection().removeTracker(key_, *this);
}

size_t getServerCount(std::string_view key) {
  size_t ret = 0;
  getTrackerCollection().forTrackersWithKey(
      key,
      [&](const std::set<ServerTracker*>& trackers) { ret = trackers.size(); });
  return ret;
}

void forEachServer(
    std::string_view key, folly::FunctionRef<void(ThriftServer&)> f) {
  getTrackerCollection().forTrackersWithKey(
      key, [&](const std::set<ServerTracker*>& trackers) {
        for (auto* tracker : trackers) {
          f(tracker->getServer());
        }
      });
}

} // namespace apache::thrift::instrumentation
