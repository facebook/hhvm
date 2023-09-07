/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <eden/common/utils/ProcessInfoCache.h>
#include <fmt/core.h>

#include <chrono>
#include <deque>
#include <unordered_map>

#include "watchman/Clock.h"
#include "watchman/CommandRegistry.h"
#include "watchman/Logging.h"
#include "watchman/PDU.h"
#include "watchman/PerfSample.h"
#include "watchman/watchman_stream.h"

namespace watchman {

class ClientStateAssertion;
class Command;
class Root;
struct Query;
struct QueryResult;

class Client : public std::enable_shared_from_this<Client> {
 public:
  Client();
  explicit Client(std::unique_ptr<watchman_stream> stm);
  virtual ~Client();

  bool dispatchCommand(const Command& command, CommandFlags mode);

  void enqueueResponse(json_ref resp);
  void enqueueResponse(UntypedResponse resp);

  const uint64_t unique_id;
  std::unique_ptr<watchman_stream> stm;
  std::unique_ptr<watchman_event> ping;
  PduBuffer reader;
  PduBuffer writer;
  bool client_mode = false;
  bool client_is_owner = false;
  PduFormat format;

  // The command currently being processed by dispatchCommand. Only set by the
  // client thread.
  const Command* current_command = nullptr;
  // The PerfSample wrapping the current command's execution. Only set by the
  // client thread.
  PerfSample* perf_sample = nullptr;

  // Queue of things to send to the client.
  std::deque<json_ref> responses;

  // Logging Subscriptions
  std::shared_ptr<Publisher::Subscriber> debugSub;
  std::shared_ptr<Publisher::Subscriber> errorSub;

 protected:
  void sendErrorResponse(std::string_view formatted);

  template <typename T, typename... Rest>
  void sendErrorResponse(
      fmt::format_string<T, Rest...> fmt,
      T&& arg,
      Rest&&... rest) {
    return sendErrorResponse(fmt::format(
        std::move(fmt), std::forward<T>(arg), std::forward<Rest>(rest)...));
  }
};

enum class OnStateTransition { QueryAnyway, DontAdvance };

class UserClient;

class ClientSubscription
    : public std::enable_shared_from_this<ClientSubscription> {
 public:
  explicit ClientSubscription(
      const std::shared_ptr<Root>& root,
      std::weak_ptr<Client> client);
  ~ClientSubscription();

  void processSubscription();

  std::shared_ptr<UserClient> lockClient();
  std::optional<UntypedResponse> buildSubscriptionResults(
      const std::shared_ptr<Root>& root,
      ClockSpec& position,
      OnStateTransition onStateTransition);

 public:
  struct LoggedResponse {
    // TODO: also track the time when the response was enqueued
    std::chrono::system_clock::time_point written;
    json_ref response;
  };

  std::shared_ptr<Root> root;
  w_string name;
  /* whether this subscription is paused */
  bool debug_paused = false;

  std::shared_ptr<Query> query;
  bool vcs_defer;
  uint32_t last_sub_tick{0};
  // map of statename => bool.  If true, policy is drop, else defer
  std::unordered_map<w_string, bool> drop_or_defer;
  std::weak_ptr<Client> weakClient;

  std::deque<LoggedResponse> lastResponses;

 private:
  ClockSpec runSubscriptionRules(
      UserClient* client,
      const std::shared_ptr<Root>& root);
  void updateSubscriptionTicks(QueryResult* res);
  void processSubscriptionImpl();
};

class ClientStatus {
 public:
  enum State {
    /// UserClient is allocated, but its thread is not started.
    THREAD_STARTING,
    /// The client thread has begun.
    THREAD_STARTED,
    /// The client thread is waiting for a request.
    WAITING_FOR_REQUEST,
    /// The client thread is decoding request data.
    DECODING_REQUEST,
    /// The client thread is executing a request.
    DISPATCHING_COMMAND,
    /// The client thread is reading subscription events and processing them.
    PROCESSING_SUBSCRIPTION,
    /// The client thread is sending responses.
    SENDING_SUBSCRIPTION_RESPONSES,
    /// The client thread is shutting down.
    THREAD_STOPPING,
  };

  void transitionTo(State state) {
    state_.store(state, std::memory_order_release);
  }

  std::string getName() const;

 private:
  // No locking or CAS required, as the tag is only written by UserClient's
  // constructor and the client thread. There will never be simultaneous state
  // transitions.
  std::atomic<State> state_{THREAD_STARTING};
};

struct PeerInfo : serde::Object {
  int32_t pid;
  std::string name;

  template <typename X>
  void map(X& x) {
    x("pid", pid);
    x("name", name);
  }
};

struct ClientDebugStatus : serde::Object {
  std::string state;
  std::optional<PeerInfo> peer;
  std::optional<int64_t> since;

  template <typename X>
  void map(X& x) {
    x("state", state);
    x("peer", peer);
    x("since", since);
  }
};

/**
 * Represents the server side session maintained for a client of
 * the watchman per-user process.
 *
 * Each UserClient has a corresponding thread that reads and decodes json
 * packets and dispatches the commands that it finds.
 */
class UserClient final : public Client {
 public:
  static void create(std::unique_ptr<watchman_stream> stm);
  ~UserClient() override;

  static std::vector<std::shared_ptr<UserClient>> getAllClients();

  static std::vector<ClientDebugStatus> getStatusForAllClients();

  /* map of subscription name => struct watchman_client_subscription */
  std::unordered_map<w_string, std::shared_ptr<ClientSubscription>>
      subscriptions;

  /* map of state-name => ClientStateAssertion
   * The values are owned by root::assertedStates */
  std::unordered_map<w_string, std::weak_ptr<ClientStateAssertion>> states;

  // Subscriber to root::unilateralResponses
  std::unordered_map<
      std::shared_ptr<ClientSubscription>,
      std::shared_ptr<Publisher::Subscriber>>
      unilateralSub;

  bool unsubByName(const w_string& name);

 private:
  UserClient() = delete;
  UserClient(UserClient&&) = delete;
  UserClient& operator=(UserClient&&) = delete;

  // To allow make_shared to construct UserClient.
  struct PrivateBadge {};

 public: // Public for std::make_shared
  explicit UserClient(PrivateBadge, std::unique_ptr<watchman_stream> stm);

 private:
  ClientDebugStatus getDebugStatus() const;

  // Abandon any states that haven't been explicit vacated.
  void vacateStates();

  void clientThread() noexcept;

  const std::chrono::system_clock::time_point since_;
  const pid_t peerPid_;
  const facebook::eden::ProcessInfoHandle peerInfo_;

  ClientStatus status_;
};

} // namespace watchman

void w_leave_state(
    watchman::UserClient* client,
    std::shared_ptr<watchman::ClientStateAssertion> assertion,
    bool abandoned,
    const std::optional<json_ref>& metadata);
