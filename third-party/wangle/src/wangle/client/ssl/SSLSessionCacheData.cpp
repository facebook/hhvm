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

//
#include <wangle/client/ssl/SSLSessionCacheData.h>

using namespace std::chrono;

namespace folly {

template <>
folly::dynamic toDynamic(const wangle::SSLSessionCacheData& data) {
  folly::dynamic ret = folly::dynamic::object;
  ret["session_data"] = folly::dynamic(data.sessionData.toStdString());
  system_clock::duration::rep rep = data.addedTime.time_since_epoch().count();
  ret["added_time"] = folly::dynamic(static_cast<uint64_t>(rep));
  ret["service_identity"] = folly::dynamic(data.serviceIdentity.toStdString());
  ret["peer_identities"] = folly::dynamic(data.peerIdentities.toStdString());
  return ret;
}

template <>
wangle::SSLSessionCacheData convertTo(const dynamic& d) {
  wangle::SSLSessionCacheData data;
  data.sessionData = d["session_data"].asString();
  data.addedTime =
      system_clock::time_point(system_clock::duration(d["added_time"].asInt()));
  data.serviceIdentity = d.getDefault("service_identity", "").asString();
  data.peerIdentities = d.getDefault("peer_identities", "").asString();
  return data;
}

} // namespace folly
