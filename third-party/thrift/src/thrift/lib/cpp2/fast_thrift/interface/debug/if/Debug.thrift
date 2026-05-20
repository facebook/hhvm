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

include "thrift/annotation/cpp.thrift"
include "thrift/lib/thrift/metadata.thrift"

package "facebook.com/thrift/fast_thrift"

namespace cpp2 apache.thrift.fast_thrift

/**
 * fast_thrift counterpart of `service Debug` in
 * `common/thrift/thrift/debug.thrift`. Field IDs and method signatures are
 * intentionally kept in lockstep with that file — the wire format must
 * match so thriftdbg (whose client codegens off the legacy IDL) can talk
 * to either stack interchangeably.
 *
 * v1 ships the three methods needed for thriftdbg's `sendRequest`,
 * `getServerDbgInfo`, and `info` TUI baseline. Other legacy methods
 * (`dumpRequests`, `dumpThriftServiceSchema`, `dumpThriftServiceMetadataV1`,
 * `getThriftServiceConfiguration`) are not declared here yet and will land
 * as the corresponding fast_thrift subsystems materialize.
 */

// === Identity ===

struct ServiceIdentityRequest {}

struct ServiceIdentityResponse {
  1: string identity;
}

// === Metadata (wire-compatible alternative to ThriftMetadataService) ===

union ServerEndpoint {
  1: i32 port;
  2: string smc_tier;
}

struct ServerMetadataInfo {
  // Field IDs match legacy. Field 1 / 3 are reserved (legacy added then
  // deprecated DEPRECATED_endpoint at id 3); we skip them to leave room
  // for wire compat if those ever surface.
  2: metadata.ThriftServiceMetadataResponse metadata;
  4: list<ServerEndpoint> endpoints;
}

struct DumpMetadataRequest {}

struct DumpMetadataResponse {
  1: list<ServerMetadataInfo> serversMetadataInfo;
}

// === Server debug info (partial substructs only) ===
//
// Substructs absent from this v1 (resourcePools, cpuCc, dls,
// runtimeServerActions, genericConfig, thriftServerConfig) require fast_thrift
// subsystems we haven't built. thriftdbg's info-tab JSON renderer treats
// missing optional fields as empty objects, so this is graceful.

struct HostDbgInfo {
  1: string hostname;
  2: string ip;
  3: i32 port;
  4: optional string tupperwareJob;
  5: optional i32 tupperwareTaskId;
  6: optional string serviceId;
}

struct ProcessDbgInfo {
  1: i64 threadsCount;
}

struct GflagsDbgInfo {
  1: map<string, string> flags;
}

struct ThriftFlagsDbgInfo {
  1: map<string, string> flags;
}

struct StreamRequestInfo {
  1: i64 count;
}

struct StateDbgInfo {
  1: bool isGLobalServer;
  2: bool isPrimaryServer;
}

struct ThriftServerMetadataDbgInfo {
  1: map<string, string> info;
  2: list<string> modules;
  3: list<string> processorEventHandlers;
  4: list<string> serverModules;
  5: list<string> serviceInterceptors;
}

struct UncategorizedDbgInfo {
  1: map<string, string> info;
}

struct ServerDbgInfo {
  // Field IDs aligned with legacy ServerDbgInfo so an opaque deserializer
  // (thriftdbg generic JSON) renders the same shape as for legacy servers.
  1: HostDbgInfo hostDbgInfo;
  2: ProcessDbgInfo processDbgInfo;
  // 3-5 reserved for resourcePoolsDbgInfo / cpuCcDbgInfo / dlsDbgInfo
  //     once those subsystems land in fast_thrift.
  // 6 reserved for genericConfigDbgInfo
  7: GflagsDbgInfo gflagsDbgInfo;
  8: ThriftFlagsDbgInfo thriftFlagsDbgInfo;
  // 9 reserved for runtimeServerActionsDbgInfo
  10: UncategorizedDbgInfo uncategorizedDbgInfo;
  // 11 reserved for thriftServerConfigDbgInfo
  12: StreamRequestInfo streamRequestInfo;
  13: StateDbgInfo stateDbgInfo;
  14: ThriftServerMetadataDbgInfo thriftServerMetadataDbgInfo;
}

struct ServerDbgInfoRequest {
  // Used by callers (e.g. thriftdbg) to pick a specific FastThriftServer
  // when multiple are registered in the process. Zero matches any.
  1: i32 thriftServerLookupPort;
}

@cpp.FastServer
service Debug {
  ServiceIdentityResponse getServiceIdentity(1: ServiceIdentityRequest req);

  DumpMetadataResponse dumpThriftServiceMetadata(1: DumpMetadataRequest req);

  ServerDbgInfo getServerDbgInfo(1: ServerDbgInfoRequest req);
}
