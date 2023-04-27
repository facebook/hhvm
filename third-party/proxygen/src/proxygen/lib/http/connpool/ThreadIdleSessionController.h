/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/connpool/SessionHolder.h>
#include <proxygen/lib/http/connpool/SessionPool.h>

/**
 * ThreadIdleSessionController keeps track of all idle sessions in a set of
 * session pools belonging to a **single thread** but potentially different
 * servers.
 *
 * The component will observe idle session pool changes and will proactively
 * purge idle sessions once a given limit is reached.
 *
 * Some benefits of using the ThreadIdleSessionController are the following:
 *    * Define a tighter bound on the total number of idle connections that are
 *    open in the system.
 *    * If you have an uneven number of connections to upstream destinations,
 *    reclaiming connections to lesser used destinations will allow for better
 *    usage of the resources in the system.
 */
namespace proxygen {

class ThreadIdleSessionController {
 public:
  /**
   * Construct an idle session controller.
   * @param totalIdleSessions The maximum number of idle sessions on a given
   * thread.
   */
  explicit ThreadIdleSessionController(uint32_t totalIdleSessions = 0);
  /*
   * Callback used to indicate a new session has been attached.
   */
  void onAttachIdle(SessionHolder*);

  /*
   * Callback used a session has been detached.
   */
  void onDetachIdle(SessionHolder*);

  /**
   * Purges the excess idle sessions in the system.
   */
  void purgeExcessIdleSessions();

  /**
   * Get the number of total idle sessions.
   */
  uint32_t getTotalIdleSessions() const;

 private:
  uint32_t totalIdleSessions_;
  SecondarySessionList idleSessionsLRU_;
};

} // namespace proxygen
