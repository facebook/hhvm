/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/network/TypedMsg.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"

namespace facebook {
namespace memcache {

/* List of operations */
using RequestReplyPairs = List<
    Pair<McGetRequest, McGetReply>,
    Pair<McSetRequest, McSetReply>,
    Pair<McDeleteRequest, McDeleteReply>,
    Pair<McLeaseGetRequest, McLeaseGetReply>,
    Pair<McLeaseSetRequest, McLeaseSetReply>,
    Pair<McAddRequest, McAddReply>,
    Pair<McReplaceRequest, McReplaceReply>,
    Pair<McGetsRequest, McGetsReply>,
    Pair<McCasRequest, McCasReply>,
    Pair<McIncrRequest, McIncrReply>,
    Pair<McDecrRequest, McDecrReply>,
    Pair<McMetagetRequest, McMetagetReply>,
    Pair<McVersionRequest, McVersionReply>,
    Pair<McAppendRequest, McAppendReply>,
    Pair<McPrependRequest, McPrependReply>,
    Pair<McTouchRequest, McTouchReply>,
    Pair<McShutdownRequest, McShutdownReply>,
    Pair<McQuitRequest, McQuitReply>,
    Pair<McStatsRequest, McStatsReply>,
    Pair<McExecRequest, McExecReply>,
    Pair<McFlushReRequest, McFlushReReply>,
    Pair<McFlushAllRequest, McFlushAllReply>,
    Pair<McGatRequest, McGatReply>,
    Pair<McGatsRequest, McGatsReply>>;

using McRequestList = PairListFirstT<RequestReplyPairs>;

using RequestsNotRateLimited = List<McStatsRequest, McVersionRequest>;

struct ListChecker {
  StaticChecker<McRequestList> checker;
};

using RequestOpMapping = List<
    KV<mc_op_get, McGetRequest>,
    KV<mc_op_set, McSetRequest>,
    KV<mc_op_delete, McDeleteRequest>,
    KV<mc_op_lease_get, McLeaseGetRequest>,
    KV<mc_op_lease_set, McLeaseSetRequest>,
    KV<mc_op_add, McAddRequest>,
    KV<mc_op_replace, McReplaceRequest>,
    KV<mc_op_gets, McGetsRequest>,
    KV<mc_op_cas, McCasRequest>,
    KV<mc_op_incr, McIncrRequest>,
    KV<mc_op_decr, McDecrRequest>,
    KV<mc_op_metaget, McMetagetRequest>,
    KV<mc_op_version, McVersionRequest>,
    KV<mc_op_append, McAppendRequest>,
    KV<mc_op_prepend, McPrependRequest>,
    KV<mc_op_touch, McTouchRequest>,
    KV<mc_op_shutdown, McShutdownRequest>,
    KV<mc_op_quit, McQuitRequest>,
    KV<mc_op_stats, McStatsRequest>,
    KV<mc_op_exec, McExecRequest>,
    KV<mc_op_flushre, McFlushReRequest>,
    KV<mc_op_flushall, McFlushAllRequest>,
    KV<mc_op_gat, McGatRequest>,
    KV<mc_op_gats, McGatsRequest>>;

using ReplyOpMapping = List<
    KV<mc_op_get, McGetReply>,
    KV<mc_op_set, McSetReply>,
    KV<mc_op_delete, McDeleteReply>,
    KV<mc_op_lease_get, McLeaseGetReply>,
    KV<mc_op_lease_set, McLeaseSetReply>,
    KV<mc_op_add, McAddReply>,
    KV<mc_op_replace, McReplaceReply>,
    KV<mc_op_gets, McGetsReply>,
    KV<mc_op_cas, McCasReply>,
    KV<mc_op_incr, McIncrReply>,
    KV<mc_op_decr, McDecrReply>,
    KV<mc_op_metaget, McMetagetReply>,
    KV<mc_op_version, McVersionReply>,
    KV<mc_op_append, McAppendReply>,
    KV<mc_op_prepend, McPrependReply>,
    KV<mc_op_touch, McTouchReply>,
    KV<mc_op_shutdown, McShutdownReply>,
    KV<mc_op_quit, McQuitReply>,
    KV<mc_op_stats, McStatsReply>,
    KV<mc_op_exec, McExecReply>,
    KV<mc_op_flushre, McFlushReReply>,
    KV<mc_op_flushall, McFlushAllReply>,
    KV<mc_op_gat, McGatReply>,
    KV<mc_op_gats, McGatsReply>>;

/**
 * Given a Request Type T and a Mapping of mc_op_t to Request Type,
 * gives, the mc_op_t corresponding to the Type T
 */
template <class T, class Mapping>
struct OpFromType;

template <class T>
struct OpFromType<T, List<>> {
  static constexpr mc_op_t value = mc_op_unknown;
};

template <class T, class KV1, class... KVs>
struct OpFromType<T, List<KV1, KVs...>> {
  static constexpr mc_op_t value = std::is_same<T, typename KV1::Value>::value
      ? static_cast<mc_op_t>(KV1::Key)
      : OpFromType<T, List<KVs...>>::value;
};

template <int op, class Mapping>
struct TypeFromOp;

template <int op>
struct TypeFromOp<op, List<>> {
  using type = void;
};

template <int op, class KV1, class... KVs>
struct TypeFromOp<op, List<KV1, KVs...>> {
  using type = typename std::conditional<
      op == KV1::Key,
      typename KV1::Value,
      typename TypeFromOp<op, List<KVs...>>::type>::type;
};

template <class T>
using TNotRateLimited = ListContains<RequestsNotRateLimited, T>;
} // namespace memcache
} // namespace facebook
