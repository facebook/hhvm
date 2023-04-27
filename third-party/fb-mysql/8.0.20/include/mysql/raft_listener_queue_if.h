// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <future>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

/* Type of callback that raft plugin wants to invoke in the server */
enum class RaftListenerCallbackType {
  SET_READ_ONLY = 1,
  TRIM_LOGGED_GTIDS = 2,
  ROTATE_BINLOG = 3,
  ROTATE_RELAYLOG = 4,
  RAFT_LISTENER_THREADS_EXIT = 5,
  RLI_RELAY_LOG_RESET = 6,
  RESET_SLAVE = 7,
  BINLOG_CHANGE_TO_APPLY = 8,
  BINLOG_CHANGE_TO_BINLOG = 9,
  STOP_SQL_THREAD = 10,
  START_SQL_THREAD = 11,
  STOP_IO_THREAD = 12,
  CHANGE_MASTER = 13,
  GET_COMMITTED_GTIDS = 14,
  GET_EXECUTED_GTIDS = 15,
  SET_BINLOG_DURABILITY = 16,
  RAFT_CONFIG_CHANGE = 17,
  HANDLE_DUMP_THREADS = 18,
  RAFT_UPDATE_FOLLOWER_INFO = 19,
  // Note: Please update CallbackTypeToString() below when adding/removing elems
  // here
};

/* Callback argument, each type would just populate the fields needed for its
 * callback */
class RaftListenerCallbackArg {
 public:
  explicit RaftListenerCallbackArg() {}
  std::vector<std::string> trim_gtids = {};
  std::pair<std::string, unsigned long long> log_file_pos = {};
  bool val_bool;
  uint32_t val_uint;
  std::pair<std::string, unsigned int> master_instance;
  std::string master_uuid;
  std::string val_str;
  std::map<std::string, unsigned int> val_sys_var_uint;
  std::pair<int64_t, int64_t> val_opid;
  std::unordered_map<std::string, std::string> val_str_map;
  // Used in RAFT_UPDATE_FOLLOWER_INFO to tell mysql that we're shutting down
  bool is_shutdown;
};

/* Result of the callback execution in the server. This will be set in the
 * future's promise (in the QueueElement) and the invoker can get()/wait() for
 * the result. Add more fields as needed */
class RaftListenerCallbackResult {
 public:
  explicit RaftListenerCallbackResult() {}

  // Indicates if the callback was able to execute successfully
  int error = 0;
  std::vector<std::string> gtids;
  std::string val_str;
};

class RaftListenerQueueIf {
 public:
  static const int RAFT_FLAGS_POSTAPPEND = 1;
  static const int RAFT_FLAGS_NOOP = 2;

  virtual ~RaftListenerQueueIf() {}

  /* Defines the element of the queue. It consists of the callback type to be
   * invoked and the argument (optional) for the callback */
  struct QueueElement {
    // Type of the callback to invoke in the server
    RaftListenerCallbackType type;

    // Argument to the callback
    RaftListenerCallbackArg arg;

    /* result of the callback will be fulfilled through this promise. If this
     * is set, then the invoker should ensure tht he eventually calls
     * get()/wait() to retrieve the result. Example:
     *
     * std::promise<RaftListenerCallbackResult> promise;
     * std::future<RaftListenerCallbackResult> fut = promise.get_future();
     *
     * QueueElement e;
     * e.type = RaftListenerCallbackType::SET_READ_ONLY;
     * e.result = &promise;
     * listener_queue.add(std::move(e));
     * ....
     * ....
     * ....
     * // Get the result when we want it. This wll block until the promise is
     * // fullfilled by the raft listener thread after executing the callback
     * RaftListenerCallbackResult result = fut.get();
     */
    std::promise<RaftListenerCallbackResult> *result = nullptr;
  };

  /* Add an element to the queue. This will signal any listening threads
   * after adding the element to the queue
   *
   * @param element QueueElement to add to queue
   *
   * @return 0 on success, 1 on error
   */
  virtual int add(QueueElement element) = 0;

  /* Get an element from the queue. This will block if there are no elements
   * in the queue to be processed
   *
   * @return QueueElement to be processed next
   */
  virtual QueueElement get() = 0;

  virtual int init() = 0;

  virtual void deinit() = 0;

  static std::string CallbackTypeToString(RaftListenerCallbackType type) {
    switch (type) {
      case RaftListenerCallbackType::SET_READ_ONLY:
        return "SET_READ_ONLY";
      case RaftListenerCallbackType::TRIM_LOGGED_GTIDS:
        return "TRIM_LOGGED_GTIDS";
      case RaftListenerCallbackType::ROTATE_BINLOG:
        return "ROTATE_BINLOG";
      case RaftListenerCallbackType::ROTATE_RELAYLOG:
        return "ROTATE_RELAYLOG";
      case RaftListenerCallbackType::RAFT_LISTENER_THREADS_EXIT:
        return "RAFT_LISTENER_THREADS_EXIT";
      case RaftListenerCallbackType::RLI_RELAY_LOG_RESET:
        return "RLI_RELAY_LOG_RESET";
      case RaftListenerCallbackType::RESET_SLAVE:
        return "RESET_SLAVE";
      case RaftListenerCallbackType::BINLOG_CHANGE_TO_APPLY:
        return "BINLOG_CHANGE_TO_APPLY";
      case RaftListenerCallbackType::BINLOG_CHANGE_TO_BINLOG:
        return "BINLOG_CHANGE_TO_BINLOG";
      case RaftListenerCallbackType::STOP_SQL_THREAD:
        return "STOP_SQL_THREAD";
      case RaftListenerCallbackType::START_SQL_THREAD:
        return "START_SQL_THREAD";
      case RaftListenerCallbackType::STOP_IO_THREAD:
        return "STOP_IO_THREAD";
      case RaftListenerCallbackType::CHANGE_MASTER:
        return "CHANGE_MASTER";
      case RaftListenerCallbackType::GET_COMMITTED_GTIDS:
        return "GET_COMMITTED_GTIDS";
      case RaftListenerCallbackType::GET_EXECUTED_GTIDS:
        return "GET_EXECUTED_GTIDS";
      case RaftListenerCallbackType::SET_BINLOG_DURABILITY:
        return "SET_BINLOG_DURABILITY";
      case RaftListenerCallbackType::RAFT_CONFIG_CHANGE:
        return "RAFT_CONFIG_CHANGE";
      case RaftListenerCallbackType::HANDLE_DUMP_THREADS:
        return "HANDLE_DUMP_THREADS";
      case RaftListenerCallbackType::RAFT_UPDATE_FOLLOWER_INFO:
        return "RAFT_UPDATE_FOLLOWER_INFO";
      default:
        return {};
    }
    return {};
  }
};
