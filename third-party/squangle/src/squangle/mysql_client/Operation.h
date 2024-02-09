/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// Operation objects are the handles users of AsyncMysqlClient use to
// interact with connections and queries.  Every action a user
// initiates returns an Operation to track the status of the action,
// report errors, etc.  Operations also offer a way to set callbacks
// for completion.
//
// In general, operations are held in shared_ptr's as ownership is
// unclear.  This allows the construction of Operations, callbacks to
// be set, and the Operation itself be cleaned up by AsyncMysqlClient.
// Conversely, if callbacks aren't being used, the Operation can
// simply be wait()'d upon for completion.
//
// See README for examples.
//
// Implementation detail; Operations straddle the caller's thread and
// the thread managed by the AsyncMysqlClient.  They also are
// responsible for execution of the actual libmysqlclient functions
// and most interactions with libevent.
//
// As mentioned above, an Operation's lifetime is determined by both
// AsyncMysqlClient and the calling point that created an Operation.
// It is permissible to immediately discard an Operation or to hold
// onto it (via a shared_ptr).  However, all calls to methods such as
// result() etc must occur either in the callback or after wait() has
// returned.

#ifndef COMMON_ASYNC_MYSQL_OPERATION_H
#define COMMON_ASYNC_MYSQL_OPERATION_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include <folly/Exception.h>
#include <folly/Function.h>
#include <folly/Memory.h>
#include <folly/String.h>
#include <folly/Unit.h>
#include <folly/dynamic.h>
#include <folly/futures/Future.h>
#include <folly/io/async/AsyncTimeout.h>
#include <folly/io/async/EventHandler.h>
#include <folly/io/async/Request.h>
#include <folly/io/async/SSLContext.h>
#include <folly/ssl/OpenSSLPtrTypes.h>
#include <folly/stop_watch.h>
#include "squangle/logger/DBEventLogger.h"
#include "squangle/mysql_client/Compression.h"
#include "squangle/mysql_client/DbResult.h"
#include "squangle/mysql_client/MysqlHandler.h"
#include "squangle/mysql_client/Query.h"
#include "squangle/mysql_client/Row.h"

DECLARE_int64(async_mysql_max_connect_timeout_micros);
namespace facebook {
namespace common {
namespace mysql_client {

class MysqlClientBase;
class QueryResult;
class ConnectOperation;
class FetchOperation;
class MultiQueryStreamOperation;
class QueryOperation;
class MultiQueryOperation;
class ResetOperation;
class SpecialOperation;
class ChangeUserOperation;
class Operation;
class Connection;
class ConnectionKey;
class ConnectionSocketHandler;
class ConnectionOptions;
class SSLOptionsProviderBase;
class SyncConnection;
class MultiQueryStreamHandler;

enum class QueryCallbackReason;

enum class StreamState;

// Simplify some std::chrono types.
typedef std::chrono::time_point<std::chrono::steady_clock> Timepoint;

// Simple name for mysql internal connect stage enum
typedef enum connect_stage MySqlConnectStage;

// Callbacks for connecting and querying, respectively.  A
// ConnectCallback is invoked when a connection succeeds or fails.  A
// QueryCallback is called for each row block (see Row.h) as well as
// when the query completes (either successfully or with an error).
using ConnectCallback = std::function<void(ConnectOperation&)>;
// Callback for observer. I will be called for a completed operation,
// after the callback for the specific operation is called, if one is defined.
using ObserverCallback = std::function<void(Operation&)>;
// Callback that is set on the ConnectOperation, and is then chained along all
// subsequent queries on that given connection.
using ChainedCallback = folly::Function<void(Operation&)>;
// Variant type allowing AsyncPostQueryCallback (below) to operate on either a
// Query or MultiQuery result.
using AsyncPostQueryResult = std::variant<DbQueryResult, DbMultiQueryResult>;
// Callbacks that are set on the ConnectOperation, and are then chained along
// all subsequent queries on that given connection. They execute asynchronously
// before/after query operations (if the async query APIs are used), and allow
// modification of the operation before or processing of results after a query.
using AsyncPreQueryCallback =
    std::function<folly::SemiFuture<folly::Unit>(FetchOperation&)>;
using AsyncPostQueryCallback =
    std::function<folly::SemiFuture<AsyncPostQueryResult>(
        AsyncPostQueryResult&&)>;
using QueryCallback =
    std::function<void(QueryOperation&, QueryResult*, QueryCallbackReason)>;
using MultiQueryCallback = std::function<
    void(MultiQueryOperation&, QueryResult*, QueryCallbackReason)>;
using CertValidatorCallback = std::function<
    bool(X509* server_cert, const void* context, folly::StringPiece& errMsg)>;
using SpecialOperationCallback =
    std::function<void(SpecialOperation&, OperationResult)>;

enum class SquangleErrno : uint16_t {
  SQ_ERRNO_CONN_TIMEOUT = 7000,
  SQ_ERRNO_CONN_TIMEOUT_LOOP_STALLED = 7001,
  SQ_ERRNO_QUERY_TIMEOUT = 7002,
  SQ_ERRNO_QUERY_TIMEOUT_LOOP_STALLED = 7003,
  SQ_ERRNO_POOL_CONN_TIMEOUT = 7004,
  SQ_ERRNO_FAILED_CONFIG_INIT = 7005,
  SQ_INVALID_API_USAGE = 7006,
};

// prefix to use in mysql errors generated by the client
constexpr auto kErrorPrefix = "MySQL Client";

// The state of the Operation.  In general, callers will see Unstarted
// (i.e., haven't yet called run()) or Completed (which may mean
// success or error; see OperationResult below).  Pending and
// Cancelling are not visible at times an outside caller might see
// them (since, once run() has been called, wait() must be called
// before inspecting other Operation attributes).
enum class OperationState {
  Unstarted,
  Pending,
  Cancelling,
  Completed,
};

// overload of operator<< for OperationState
std::ostream& operator<<(std::ostream& os, OperationState state);

// Once an operation is Completed, it has a result type, indicating
// what ultimately occurred.  These are self-explanatory.
enum class OperationResult {
  Unknown,
  Succeeded,
  Failed,
  Cancelled,
  TimedOut,
};

// overload of operator<< for OperationResult
std::ostream& operator<<(std::ostream& os, OperationResult result);

// For control flows in callbacks. This indicates the reason a callback was
// fired. When a pack of rows if fetched it is used RowsFetched to
// indicate that new rows are available. QueryBoundary means that the
// fetching for current query has completed successfully, and if any
// query failed (OperationResult is Failed) we use Failure. Success is for
// indicating that all queries have been successfully fetched.
enum class QueryCallbackReason { RowsFetched, QueryBoundary, Failure, Success };

// overload of operator<< for QueryCallbackReason
std::ostream& operator<<(std::ostream& os, QueryCallbackReason reason);

enum class StreamState { InitQuery, RowsReady, QueryEnded, Failure, Success };

// overload of operator<< for StreamState
std::ostream& operator<<(std::ostream& os, StreamState state);

class ConnectionOptions {
 public:
  ConnectionOptions();

  // Each attempt to acquire a connection will take at maximum this duration.
  // Use setTotalTimeout if you want to limit the timeout for all attempts.
  ConnectionOptions& setTimeout(Duration dur) {
    connection_timeout_ = dur;
    return *this;
  }

  Duration getTimeout() const {
    return connection_timeout_;
  }

  // This time out is used for each connect attempt and this is the maximum
  // time allowed for client<->server tcp handshake. This timeout allows client
  // to failfast in case the MYSQL server is not reachable, where we don't want
  // to wait for the entire connection_timeout_ which also includes time buffer
  // for ssl and MYSQL protocol exchange
  ConnectionOptions& setConnectTcpTimeout(Duration dur) {
    connection_tcp_timeout_ = dur;
    return *this;
  }

  const folly::Optional<Duration>& getConnectTcpTimeout() const {
    return connection_tcp_timeout_;
  }

  // The connection created by these options will apply this query timeout
  // to all statements executed
  ConnectionOptions& setQueryTimeout(Duration dur) {
    query_timeout_ = dur;
    return *this;
  }

  Duration getQueryTimeout() const {
    return query_timeout_;
  }

  // Used to provide an SSLContext and SSL_Session provider
  ConnectionOptions& setSSLOptionsProvider(
      std::shared_ptr<SSLOptionsProviderBase> ssl_options_provider) {
    ssl_options_provider_ = ssl_options_provider;
    return *this;
  }

  std::shared_ptr<SSLOptionsProviderBase> getSSLOptionsProvider() const {
    return ssl_options_provider_;
  }

  SSLOptionsProviderBase* getSSLOptionsProviderPtr() const {
    return ssl_options_provider_.get();
  }

  // Provide a Connection Attribute to be passed in the connection handshake
  ConnectionOptions& setAttribute(std::string_view attr, std::string value) {
    attributes_[attr] = std::move(value);
    return *this;
  }

  // MySQL 5.6 connection attributes.  Sent at time of connect.
  const AttributeMap& getAttributes() const {
    return attributes_;
  }

  ConnectionOptions& setAttributes(const AttributeMap& attributes) {
    for (auto& [key, value] : attributes) {
      attributes_[key] = value;
    }
    return *this;
  }

  // Sorry for the weird API, there is no enum for compression = None
  ConnectionOptions& setCompression(
      folly::Optional<CompressionAlgorithm> comp_lib) {
    compression_lib_ = std::move(comp_lib);
    return *this;
  }

  const folly::Optional<CompressionAlgorithm>& getCompression() const {
    return compression_lib_;
  }

  ConnectionOptions& setUseChecksum(bool useChecksum) noexcept {
    use_checksum_ = useChecksum;
    return *this;
  }

  FOLLY_NODISCARD bool getUseChecksum() const noexcept {
    return use_checksum_;
  }

  // Sets the amount of attempts that will be tried in order to acquire the
  // connection. Each attempt will take at maximum the given timeout. To set
  // a global timeout that the operation shouldn't take more than, use
  // setTotalTimeout.
  //
  // This is no longer recommended for use, due to higher level retries
  ConnectionOptions& setConnectAttempts(uint32_t max_attempts) {
    max_attempts_ = max_attempts;
    return *this;
  }

  uint32_t getConnectAttempts() const {
    return max_attempts_;
  }

  // Sets the differentiated service code point (DSCP) on the underlying
  // connection, which has the effect of embedding it into outgoing packet ip
  // headers. The value may be used to classify and prioritize said traffic.
  //
  // Note: A DSCP value is 6 bits and is packed into an 8 bit field. Users must
  // specify the unpacked (unshifted) 6-bit value.
  //
  // Note: This implementation only supports IPv6.
  //
  // Also known as "Quality of Service" (QoS), "Type of Service", "Class of
  // Service" (COS).
  //
  // See also RFC 2474 [0] and RFC 3542 6.5 (IPv6 sockopt) [1]
  // [0]: https://tools.ietf.org/html/rfc2474
  // [1]: https://tools.ietf.org/html/rfc3542#section-6.5
  ConnectionOptions& setDscp(uint8_t dscp) {
    CHECK_THROW((dscp & 0b11000000) == 0, std::invalid_argument);
    dscp_ = dscp;
    return *this;
  }

  folly::Optional<uint8_t> getDscp() const {
    return dscp_;
  }

  // If this is not set, but regular timeout was, the TotalTimeout for the
  // operation will be the number of attempts times the primary timeout.
  // Set this if you have strict timeout needs.
  //
  // This should generally not be set, as connectAttempts is 1
  ConnectionOptions& setTotalTimeout(Duration dur) {
    total_timeout_ = dur;
    return *this;
  }

  Duration getTotalTimeout() const {
    return total_timeout_;
  }

  std::string getDisplayString() const;

  ConnectionOptions& setSniServerName(const std::string& sniName) {
    sni_servername_ = sniName;
    return *this;
  }

  const folly::Optional<std::string>& getSniServerName() const {
    return sni_servername_;
  }

  ConnectionOptions& enableResetConnBeforeClose() {
    reset_conn_before_close_ = true;
    return *this;
  }

  bool isEnableResetConnBeforeClose() const {
    return reset_conn_before_close_;
  }

  ConnectionOptions& enableDelayedResetConn() {
    delayed_reset_conn_ = true;
    return *this;
  }

  bool isEnableDelayedResetConn() const {
    return delayed_reset_conn_;
  }

  ConnectionOptions& enableChangeUser() {
    change_user_ = true;
    return *this;
  }

  bool isEnableChangeUser() const {
    return change_user_;
  }

  ConnectionOptions& setCertValidationCallback(
      CertValidatorCallback callback,
      const void* context,
      bool opPtrAsContext) {
    certValidationCallback_ = std::move(callback);
    opPtrAsCertValidationContext_ = opPtrAsContext;
    certValidationContext_ = opPtrAsCertValidationContext_ ? nullptr : context;
    return *this;
  }

  const CertValidatorCallback& getCertValidationCallback() const {
    return certValidationCallback_;
  }

  const void* getCertValidationContext() const {
    return certValidationContext_;
  }

  bool isOpPtrAsValidationContext() const {
    return opPtrAsCertValidationContext_;
  }

 private:
  Duration connection_timeout_;
  folly::Optional<Duration> connection_tcp_timeout_;
  Duration total_timeout_;
  Duration query_timeout_;
  std::shared_ptr<SSLOptionsProviderBase> ssl_options_provider_;
  AttributeMap attributes_;
  folly::Optional<CompressionAlgorithm> compression_lib_;
  bool use_checksum_ = false;
  uint32_t max_attempts_ = 1;
  folly::Optional<uint8_t> dscp_;
  folly::Optional<std::string> sni_servername_;
  bool reset_conn_before_close_ = false;
  bool delayed_reset_conn_ = false;
  bool change_user_ = false;
  CertValidatorCallback certValidationCallback_{nullptr};
  const void* certValidationContext_{nullptr};
  bool opPtrAsCertValidationContext_{false};
};

// The abstract base for our available Operations.  Subclasses share
// intimate knowledge with the Operation class (most member variables
// are protected).
class Operation : public std::enable_shared_from_this<Operation> {
 public:
  // No public constructor.
  virtual ~Operation();

  Operation* run();

  // Set a timeout; otherwise FLAGS_async_mysql_timeout_micros is
  // used.
  Operation* setTimeout(Duration timeout) {
    CHECK_THROW(
        state_ == OperationState::Unstarted, db::OperationStateException);
    timeout_ = timeout;
    return this;
  }

  Duration getTimeout() {
    return timeout_;
  }

  Duration getMaxThreadBlockTime() {
    return max_thread_block_time_;
  }

  Duration getTotalThreadBlockTime() {
    return total_thread_block_time_;
  }

  void logThreadBlockTime(const folly::stop_watch<Duration> sw) {
    auto block_time = sw.elapsed();
    max_thread_block_time_ = std::max(max_thread_block_time_, block_time);
    total_thread_block_time_ += block_time;
  }

  // Did the operation succeed?
  bool ok() const {
    return done() && result_ == OperationResult::Succeeded;
  }

  // Is the operation complete (success or failure)?
  bool done() const {
    return state_ == OperationState::Completed;
  }

  // host and port we are connected to (or will be connected to).
  const std::string& host() const;
  int port() const;

  // Try to cancel a pending operation.  This is inherently racey with
  // callbacks; it is possible the callback is being invoked *during*
  // the cancel attempt, so a cancelled operation may still succeed.
  void cancel();

  // Wait for the Operation to complete.
  void wait();

  // Wait for an operation to complete.  Throw a
  // RequiredOperationFailedException if it fails. Mainly for testing.
  virtual void mustSucceed() = 0;

  // Information about why this operation failed.
  unsigned int mysql_errno() const {
    return mysql_errno_;
  }
  const std::string& mysql_error() const {
    return mysql_error_;
  }
  const std::string& mysql_normalize_error() const {
    return mysql_normalize_error_;
  }

  // Get the state and result, as well as readable string versions.
  OperationResult result() const {
    return result_;
  }

  folly::StringPiece resultString() const;

  OperationState state() const {
    return state_;
  }

  folly::StringPiece stateString() const;

  static folly::StringPiece toString(OperationState state);
  static folly::StringPiece toString(OperationResult result);
  static folly::StringPiece toString(QueryCallbackReason reason);
  static folly::StringPiece toString(StreamState state);

  static folly::fbstring connectStageString(connect_stage stage);

  // An Operation can have a folly::dynamic associated with it.  This
  // can represent anything the caller wants to track and is primarily
  // useful in the callback.  Typically this would be a string or
  // integer.  Note, also, such information can be stored inside the
  // callback itself (via a lambda).
  Operation* setUserData(folly::dynamic val) {
    user_data_.assign(std::move(val));
    return this;
  }

  const folly::dynamic& userData() const {
    return *user_data_;
  }
  folly::dynamic&& stealUserData() {
    return std::move(*user_data_);
  }

  Operation* setAttributes(const AttributeMap& attributes) {
    CHECK_THROW(
        state() == OperationState::Unstarted, db::OperationStateException);
    for (const auto& [key, value] : attributes) {
      attributes_[key] = value;
    }
    return this;
  }

  Operation* setAttributes(AttributeMap&& attributes) {
    CHECK_THROW(
        state() == OperationState::Unstarted, db::OperationStateException);
    for (auto& [key, value] : attributes) {
      attributes_[key] = std::move(value);
    }
    return this;
  }

  Operation* setAttribute(const std::string& key, const std::string& value) {
    CHECK_THROW(
        state() == OperationState::Unstarted, db::OperationStateException);
    attributes_[key] = value;
    return this;
  }

  const AttributeMap& getAttributes() const {
    return attributes_;
  }

  // Connections are transferred across operations.  At any one time,
  // there is one unique owner of the connection.
  std::unique_ptr<Connection>&& releaseConnection();
  const Connection* connection() const {
    return conn_proxy_.get();
  }
  Connection* connection() {
    return conn_proxy_.get();
  }

  // Various accessors for our Operation's start, end, and total elapsed time.
  Timepoint startTime() const {
    return start_time_;
  }
  Timepoint endTime() const {
    CHECK_THROW(
        state_ == OperationState::Completed, db::OperationStateException);
    return end_time_;
  }

  Duration elapsed() const {
    CHECK_THROW(
        state_ == OperationState::Completed, db::OperationStateException);
    return std::chrono::duration_cast<std::chrono::microseconds>(
        end_time_ - start_time_);
  }

  /**
   * Set various callbacks that are invoked during the operation's lifetime
   * The pre and post operations are chained, in that they are propagated to
   * operations that are scheduled on the connection following the current
   * operation. A couple notes:
   *
   * The PreOperationCallback will be invoked on a cancelled operation before
   * the cancellation takes effect
   *
   * The PostOperationCallback will be invoked on operations that have failed
   */
  void setObserverCallback(ObserverCallback obs_cb);
  void setPreOperationCallback(ChainedCallback obs_cb);
  void setPostOperationCallback(ChainedCallback obs_cb);

  /**
   * Set callbacks that are invoked asynchronously before/after query operations
   * (if using asynchronous query APIs). They do not execute on operations that
   * have failed, and if they fail the operation will fail too.
   *
   * PreQueryCallbacks execute after a query operation is initialized and before
   * it has been run. They are allowed to modify the operation by reference.
   *
   * PostQueryCallbacks execute after a query operation returns results. They
   * are allowed to do post-processing on the results received and returned via
   * rvalue reference.
   */
  void setPreQueryCallback(AsyncPreQueryCallback&& callback);
  void setPostQueryCallback(AsyncPostQueryCallback&& callback);

  // Retrieve the shared pointer that holds this instance.
  std::shared_ptr<Operation> getSharedPointer();

  MysqlClientBase* client() const;

  // Flag internal async client errors; this always becomes a MySQL
  // error 2000 (CR_UNKNOWN_ERROR) with a suitable descriptive message.
  void setAsyncClientError(
      folly::StringPiece msg,
      folly::StringPiece normalizeMsg = "");

  // Same as above, but specify the error code.
  void setAsyncClientError(
      unsigned int mysql_errno,
      folly::StringPiece msg,
      folly::StringPiece normalizeMsg = "");

  virtual db::OperationType getOperationType() const = 0;

 protected:
  // Helper function to set chained callbacks
  ChainedCallback setCallback(
      ChainedCallback orgCallback,
      ChainedCallback newCallback);

  // Helper functions to set query callbacks
  static AsyncPreQueryCallback appendCallback(
      AsyncPreQueryCallback&& callback1,
      AsyncPreQueryCallback&& callback2);

  static AsyncPostQueryCallback appendCallback(
      AsyncPostQueryCallback&& callback1,
      AsyncPostQueryCallback&& callback2);

  static constexpr double kCallbackDelayStallThresholdUs = 50 * 1000;

  class ConnectionProxy;
  explicit Operation(ConnectionProxy&& conn);

  ConnectionProxy& conn() {
    return conn_proxy_;
  }
  const ConnectionProxy& conn() const {
    return conn_proxy_;
  }

  // Save any mysql errors that occurred (since we may hand off the
  // Connection before the user wants this information).
  void snapshotMysqlErrors();

  // Called when an Operation needs to wait for the socket to become
  // readable or writable (aka actionable).
  void waitForSocketActionable();

  // Overridden in child classes and invoked when the socket is
  // actionable.  This function should either completeOperation or
  // waitForSocketActionable.
  virtual void socketActionable() = 0;

  // Called by ConnectionSocketHandler when the operation timed out
  void timeoutTriggered();

  // Our operation has completed.  During completeOperation,
  // specializedCompleteOperation is invoked for subclasses to perform
  // their own finalization (typically annotating errors and handling
  // timeouts).
  void completeOperation(OperationResult result);
  void completeOperationInner(OperationResult result);
  virtual Operation* specializedRun() = 0;
  virtual void specializedTimeoutTriggered() = 0;
  virtual void specializedCompleteOperation() = 0;

  class OwnedConnection {
   public:
    OwnedConnection();
    explicit OwnedConnection(std::unique_ptr<Connection>&& conn);
    Connection* get();
    std::unique_ptr<Connection>&& releaseConnection();

   private:
    std::unique_ptr<Connection> conn_;
  };

  class ReferencedConnection {
   public:
    ReferencedConnection() : conn_(nullptr) {}
    explicit ReferencedConnection(Connection* conn) : conn_(conn) {}
    Connection* get() {
      return conn_;
    }

   private:
    Connection* conn_;
  };

  // Base class for a wrapper around the 2 types of connection
  // pointers we accept in the Operation:
  // - OwnedConnection: will hold an unique_ptr to the Connection
  //   for the async calls of the API, so the ownership is clear;
  // - ReferencedConnection: allows synchronous calls without moving unique_ptrs
  //   to the Operation;
  class ConnectionProxy {
   public:
    explicit ConnectionProxy(OwnedConnection&& conn);
    explicit ConnectionProxy(ReferencedConnection&& conn);

    Connection* get();

    std::unique_ptr<Connection>&& releaseConnection();

    const Connection* get() const {
      return const_cast<ConnectionProxy*>(this)->get();
    }

    Connection* operator->() {
      return get();
    }
    const Connection* operator->() const {
      return get();
    }

    ConnectionProxy(ConnectionProxy&&) = default;
    ConnectionProxy& operator=(ConnectionProxy&&) = default;

    ConnectionProxy(ConnectionProxy const&) = delete;
    ConnectionProxy& operator=(ConnectionProxy const&) = delete;

   private:
    OwnedConnection ownedConn_;
    ReferencedConnection referencedConn_;
  };

  bool isInEventBaseThread() const;
  bool isEventBaseSet() const;

  bool isCancelledOnRun() const {
    return cancel_on_run_;
  }

  std::string threadOverloadMessage(double cbDelayUs) const;
  std::string timeoutMessage(std::chrono::milliseconds delta) const;

  // Data members; subclasses freely interact with these.
  OperationState state_;
  OperationResult result_;

  // Our client is not owned by us. It must outlive all active Operations.
  Duration timeout_;
  Timepoint start_time_;
  Timepoint end_time_;

  // This will contain the max block time of the thread
  Duration max_thread_block_time_ = Duration(0);
  Duration total_thread_block_time_ = Duration(0);

  // Our Connection object.  Created by ConnectOperation and moved
  // into QueryOperations.
  ConnectionProxy conn_proxy_;

  // Errors that may have occurred.
  unsigned int mysql_errno_;
  std::string mysql_error_;
  std::string mysql_normalize_error_;

  // Connection or query attributes (depending on the Operation type)
  AttributeMap attributes_;

  // This mutex protects the operation cancel process when the state
  // is being checked in `run` and the operation is being cancelled in other
  // thread.
  std::mutex run_state_mutex_;

  struct Callbacks {
    Callbacks()
        : pre_operation_callback_(nullptr),
          post_operation_callback_(nullptr),
          pre_query_callback_(nullptr),
          post_query_callback_(nullptr) {}

    ChainedCallback pre_operation_callback_;
    ChainedCallback post_operation_callback_;

    AsyncPreQueryCallback pre_query_callback_;
    AsyncPostQueryCallback post_query_callback_;
  };

  Callbacks callbacks_;

  // Friends because they need to access the query callbacks on this class
  template <typename Operation>
  friend folly::SemiFuture<folly::Unit> handlePreQueryCallback(Operation& op);
  template <typename ReturnType, typename Operation, typename QueryResult>
  friend void handleQueryCompletion(
      Operation& op,
      QueryResult query_result,
      QueryCallbackReason reason,
      folly::Promise<std::pair<ReturnType, AsyncPostQueryCallback>>& promise);

 private:
  // Restore folly::RequestContext and also invoke socketActionable()
  void invokeSocketActionable();

  std::shared_ptr<folly::RequestContext> request_context_;

  folly::Optional<folly::dynamic> user_data_;
  ObserverCallback observer_callback_;
  std::shared_ptr<db::ConnectionContextBase> connection_context_;

  MysqlClientBase* mysql_client_;

  bool cancel_on_run_ = false;

  Operation() = delete;
  Operation(const Operation&) = delete;
  Operation& operator=(const Operation&) = delete;

  friend class Connection;
  friend class SyncConnection;
  friend class SyncConnectionPool;
  friend class ConnectionSocketHandler;
};

// Timeout used for controlling early timeout of just the tcp handshake phase
// before doing heavy lifting like ssl and other mysql protocol for connection
// establishment
class ConnectTcpTimeoutHandler : public folly::AsyncTimeout {
 public:
  ConnectTcpTimeoutHandler(
      folly::EventBase* base,
      ConnectOperation* connect_operation)
      : folly::AsyncTimeout(base), op_(connect_operation) {}

  void timeoutExpired() noexcept override;

 private:
  ConnectOperation* op_;

  ConnectTcpTimeoutHandler() = delete;
  ConnectTcpTimeoutHandler(const ConnectTcpTimeoutHandler&) = delete;
  ConnectTcpTimeoutHandler& operator=(const ConnectTcpTimeoutHandler&) = delete;
};

// An operation representing a pending connection.  Constructed via
// AsyncMysqlClient::beginConnection.
class ConnectOperation : public Operation {
 public:
  ~ConnectOperation() override;

  void setCallback(ConnectCallback cb) {
    connect_callback_ = std::move(cb);
  }

  const std::string& database() const {
    return conn_key_.db_name();
  }
  const std::string& user() const {
    return conn_key_.user();
  }

  const ConnectionKey& getConnectionKey() const {
    return conn_key_;
  }
  const ConnectionOptions& getConnectionOptions() const;
  const ConnectionKey* getKey() const {
    return &conn_key_;
  }

  ConnectOperation* setSSLOptionsProviderBase(
      std::unique_ptr<SSLOptionsProviderBase> ssl_options_provider);
  ConnectOperation* setSSLOptionsProvider(
      std::shared_ptr<SSLOptionsProviderBase> ssl_options_provider);

  // Default timeout for queries created by the connection this
  // operation will create.
  ConnectOperation* setDefaultQueryTimeout(Duration t);
  ConnectOperation* setConnectionContext(
      std::shared_ptr<db::ConnectionContextBase> e) {
    CHECK_THROW(
        state_ == OperationState::Unstarted, db::OperationStateException);
    connection_context_ = std::move(e);
    return this;
  }
  ConnectOperation* setSniServerName(const std::string& sni_servername);
  ConnectOperation* enableResetConnBeforeClose();
  ConnectOperation* enableDelayedResetConn();
  ConnectOperation* enableChangeUser();
  ConnectOperation* setCertValidationCallback(
      CertValidatorCallback callback,
      const void* context = nullptr,
      bool opPtrAsContext = false);
  const CertValidatorCallback& getCertValidationCallback() const {
    return conn_options_.getCertValidationCallback();
  }

  db::ConnectionContextBase* getConnectionContext() {
    CHECK_THROW(
        state_ == OperationState::Unstarted, db::OperationStateException);
    return connection_context_.get();
  }

  const db::ConnectionContextBase* getConnectionContext() const {
    CHECK_THROW(
        state_ == OperationState::Unstarted ||
            state_ == OperationState::Completed,
        db::OperationStateException);
    return connection_context_.get();
  }

  void reportServerCertContent(
      const std::string& sslCertCn,
      const std::vector<std::string>& sslCertSan,
      const std::vector<std::string>& sslCertIdentities,
      bool isValidated) {
    if (connection_context_) {
      if (!sslCertCn.empty()) {
        connection_context_->sslCertCn = sslCertCn;
      }
      if (!sslCertSan.empty()) {
        connection_context_->sslCertSan = sslCertSan;
      }
      if (!sslCertIdentities.empty()) {
        connection_context_->sslCertIdentities = sslCertIdentities;
      }
      connection_context_->isServerCertValidated = isValidated;
    }
  }

  // Don't call this; it's public strictly for AsyncMysqlClient to be
  // able to call make_shared.
  ConnectOperation(MysqlClientBase* mysql_client, ConnectionKey conn_key);

  void mustSucceed() override;

  // Overriding to narrow the return type
  // Each connect attempt will take at most this timeout to retry to acquire
  // the connection.
  ConnectOperation* setTimeout(Duration timeout);

  // This timeout allows for clients to fail fast when tcp handshake
  // latency is high . This method allows to override the tcp timeout
  // connection options. These timeouts can either be set directly or by
  // passing in connection options.
  ConnectOperation* setTcpTimeout(Duration timeout);

  const folly::Optional<Duration>& getTcpTimeout() const {
    return conn_options_.getConnectTcpTimeout();
  }

  ConnectOperation* setUserData(folly::dynamic val) {
    Operation::setUserData(std::move(val));
    return this;
  }

  // Sets the total timeout that the connect operation will use.
  // Each attempt will take at most `setTimeout`. Use this in case
  // you have strong timeout restrictions but still want the connection to
  // retry.
  ConnectOperation* setTotalTimeout(Duration total_timeout);

  // Sets the number of attempts this operation will try to acquire a mysql
  // connection.
  ConnectOperation* setConnectAttempts(uint32_t max_attempts);

  // Sets the DSCP (QoS) value associated with this connection
  //
  // See Also ConnectionOptions::setDscp
  ConnectOperation* setDscp(uint8_t dscp);

  folly::Optional<uint8_t> getDscp() const {
    return conn_options_.getDscp();
  }

  uint32_t attemptsMade() const {
    return attempts_made_;
  }

  Duration getAttemptTimeout() const {
    return conn_options_.getTimeout();
  }

  // Set if we should open a new connection to kill a timed out query
  // Should not be used when connecting through a proxy
  ConnectOperation* setKillOnQueryTimeout(bool killOnQueryTimeout);

  bool getKillOnQueryTimeout() const {
    return killOnQueryTimeout_;
  }

  ConnectOperation* setCompression(
      folly::Optional<CompressionAlgorithm> compression_lib) {
    conn_options_.setCompression(std::move(compression_lib));
    return this;
  }

  const folly::Optional<CompressionAlgorithm>& getCompression() const {
    return conn_options_.getCompression();
  }

  ConnectOperation* setConnectionOptions(const ConnectionOptions& conn_options);

  static constexpr Duration kMinimumViableConnectTimeout =
      std::chrono::microseconds(50);

  db::OperationType getOperationType() const override {
    return db::OperationType::Connect;
  }

  bool isActive() const {
    return active_in_client_;
  }

 protected:
  virtual void attemptFailed(OperationResult result);
  virtual void attemptSucceeded(OperationResult result);

  ConnectOperation* specializedRun() override;
  void socketActionable() override;
  void specializedTimeoutTriggered() override;
  void specializedCompleteOperation() override;

  // Called when tcp timeout is triggered
  void tcpConnectTimeoutTriggered();

  // Removes the Client ref, it can be called by child classes without needing
  // to add them as friend classes of AsyncMysqlClient
  virtual void removeClientReference();

  bool shouldCompleteOperation(OperationResult result);

  folly::ssl::SSLSessionUniquePtr getSSLSession();

  // Implementation of timeout handling for tcpTimeout and overall connect
  // timeout
  void timeoutHandler(bool isTcpTimeout, bool isPool = false);

  uint32_t attempts_made_ = 0;
  bool killOnQueryTimeout_ = false;
  ConnectionOptions conn_options_;

  // Context information for logging purposes.
  std::shared_ptr<db::ConnectionContextBase> connection_context_;

 private:
  void specializedRunImpl();

  void logConnectCompleted(OperationResult result);

  void maybeStoreSSLSession();

  bool isDoneWithTcpHandShake();

  static int mysqlCertValidator(
      X509* server_cert,
      const void* context,
      const char** errptr);

  const ConnectionKey conn_key_;

  int flags_;

  ConnectCallback connect_callback_;
  bool active_in_client_;
  ConnectTcpTimeoutHandler tcp_timeout_handler_;

  // Mysql internal connect stage which handles the async tcp handshake
  // completion between client and server
  static constexpr MySqlConnectStage tcpCompletionStage_ =
      MySqlConnectStage::CONNECT_STAGE_NET_COMPLETE_CONNECT;

  friend class AsyncMysqlClient;
  friend class MysqlClientBase;
  friend class ConnectTcpTimeoutHandler;
};

// A fetching operation (query or multiple queries) use the same primary
// actions. This is an abstract base for this kind of operation.
// FetchOperation controls the flow of fetching a result:
//  - When there are rows to be read, it will identify it and call the
//  subclasses
// for them to consume the state;
//  - When there are no rows to be read or an error happened, proper
//  notifications
// will be made as well.
// This is the only Operation that can be paused, and the pause should only be
// called from within `notify` calls. That will allow another thread to read
// the state.
class FetchOperation : public Operation {
 public:
  using RespAttrs = AttributeMap;
  ~FetchOperation() override = default;
  void mustSucceed() override;

  // Return the query as it was sent to MySQL (i.e., for a single
  // query, the query itself, but for multiquery, all queries
  // combined).
  folly::fbstring getExecutedQuery() const {
    CHECK_THROW(
        state_ != OperationState::Unstarted, db::OperationStateException);
    return rendered_query_.to<folly::fbstring>();
  }

  // Number of queries that succeed to execute
  int numQueriesExecuted() {
    CHECK_THROW(state_ != OperationState::Pending, db::OperationStateException);
    return num_queries_executed_;
  }

  uint64_t resultSize() const {
    CHECK_THROW(
        state_ != OperationState::Unstarted, db::OperationStateException);
    return total_result_size_;
  }

  FetchOperation* setUseChecksum(bool useChecksum) noexcept;

  // This class encapsulates the operations and access to the MySQL ResultSet.
  // When the consumer receives a notification for RowsFetched, it should
  // consume `rowStream`:
  //   while (rowStream->hasNext()) {
  //     EphemeralRow row = consumeRow();
  //   }
  // The state within RowStream is also used for FetchOperation to know
  // whether or not to go to next query.
  class RowStream {
   public:
    RowStream(MYSQL_RES* mysql_query_result, MysqlHandler* handler);

    EphemeralRow consumeRow();

    bool hasNext();

    EphemeralRowFields* getEphemeralRowFields() {
      return &row_fields_;
    }

    ~RowStream() = default;
    RowStream(RowStream&&) = default;
    RowStream& operator=(RowStream&&) = default;

   private:
    friend class FetchOperation;
    bool slurp();
    // user shouldn't take information from this
    bool hasQueryFinished() {
      return query_finished_;
    }
    uint64_t numRowsSeen() const {
      return num_rows_seen_;
    }

    bool query_finished_ = false;
    uint64_t num_rows_seen_ = 0;
    uint64_t query_result_size_ = 0;

    using MysqlResultDeleter =
        folly::static_function_deleter<MYSQL_RES, mysql_free_result>;
    using MysqlResultUniquePtr = std::unique_ptr<MYSQL_RES, MysqlResultDeleter>;

    // All memory lifetime is guaranteed by FetchOperation.
    MysqlResultUniquePtr mysql_query_result_ = nullptr;
    folly::Optional<EphemeralRow> current_row_;
    EphemeralRowFields row_fields_;
    MysqlHandler* handler_ = nullptr;
  };

  // Streaming calls. Should only be called when using the StreamCallback.
  // TODO#10716355: We shouldn't let these functions visible for non-stream
  // mode. Leaking for tests.
  uint64_t currentLastInsertId() const;
  uint64_t currentAffectedRows() const;
  const std::string& currentRecvGtid() const;
  const RespAttrs& currentRespAttrs() const;

  bool noIndexUsed() const {
    return no_index_used_;
  }

  bool wasSlow() const {
    return was_slow_;
  }

  int numCurrentQuery() const {
    return num_current_query_;
  }

  RowStream* rowStream();

  // Stalls the FetchOperation until `resume` is called.
  // This is used to allow another thread to access the socket functions.
  void pauseForConsumer();

  // Resumes the operation to the action it was before `pause` was called.
  // Should only be called after pause.
  void resume();

  int rows_received_ = 0;

 protected:
  MultiQuery queries_;

  FetchOperation* specializedRun() override;

  FetchOperation(ConnectionProxy&& conn, std::vector<Query>&& queries);
  FetchOperation(ConnectionProxy&& conn, MultiQuery&& multi_query);

  enum class FetchAction {
    StartQuery,
    InitFetch,
    Fetch,
    WaitForConsumer,
    CompleteQuery,
    CompleteOperation
  };

  void setFetchAction(FetchAction action);
  static folly::StringPiece toString(FetchAction action);

  // In socket actionable it is analyzed the action that is required to
  // continue the operation. For example, if the fetch action is StartQuery,
  // it runs query or requests more results depending if it had already ran or
  // not the query. The same process happens for the other FetchActions. The
  // action member can be changed in other member functions called in
  // socketActionable to keep the fetching flow running.
  void socketActionable() override;
  void specializedTimeoutTriggered() override;
  void specializedCompleteOperation() override;

  // Overridden in child classes and invoked when the Query fetching
  // has done specific actions that might be needed for report (callbacks,
  // store fetched data, initialize data).
  virtual void notifyInitQuery() = 0;
  virtual void notifyRowsReady() = 0;
  virtual void notifyQuerySuccess(bool more_results) = 0;
  virtual void notifyFailure(OperationResult result) = 0;
  virtual void notifyOperationCompleted(OperationResult result) = 0;

  bool cancel_ = false;

 private:
  friend class MultiQueryStreamHandler;
  void specializedRunImpl();

  int setQueryAttribute(const std::string& key, const std::string& value);

  void resumeImpl();
  // Checks if the current thread has access to stream, or result data.
  bool isStreamAccessAllowed() const;
  bool isPaused() const;

  // Read the response attributes
  RespAttrs readResponseAttributes();

  // Asynchronously kill a currently running query, returns
  // before the query is killed
  void killRunningQuery();

  // Current query data
  folly::Optional<RowStream> current_row_stream_;
  bool query_executed_ = false;
  bool no_index_used_ = false;
  bool use_checksum_ = false;
  bool was_slow_ = false;
  // TODO: Rename `executed` to `succeeded`
  int num_queries_executed_ = 0;
  // During a `notify` call, the consumer might want to know the index of the
  // current query, that's what `num_current_query_` is counting.
  int num_current_query_ = 0;
  // Best effort attempt to calculate the size of the result set in bytes.
  // Only counts the actual data in rows, not bytes sent over the wire, and
  // doesn't include column/table metadata or mysql packet overhead
  uint64_t total_result_size_ = 0;

  uint64_t current_affected_rows_ = 0;
  uint64_t current_last_insert_id_ = 0;
  std::string current_recv_gtid_;
  RespAttrs current_resp_attrs_;

  // When the Fetch gets paused, active fetch action moves to
  // `WaitForConsumer` and the action that got paused gets saved so tat
  // `resume` can set it properly afterwards.
  FetchAction active_fetch_action_ = FetchAction::StartQuery;
  FetchAction paused_action_ = FetchAction::StartQuery;

  folly::StringPiece rendered_query_;
};

// This operation only supports one mode: streaming callback. This is a
// simple layer on top of FetchOperation to adapt from `notify` to
// StreamCallback.
// This is an experimental class. Please don't use directly.
class MultiQueryStreamOperation : public FetchOperation {
 public:
  ~MultiQueryStreamOperation() override = default;

  typedef std::function<void(FetchOperation*, StreamState)> Callback;

  using StreamCallback = boost::variant<MultiQueryStreamHandler*, Callback>;

  void notifyInitQuery() override;
  void notifyRowsReady() override;
  void notifyQuerySuccess(bool more_results) override;
  void notifyFailure(OperationResult result) override;
  void notifyOperationCompleted(OperationResult result) override;

  // Overriding to narrow the return type
  MultiQueryStreamOperation* setTimeout(Duration timeout) {
    Operation::setTimeout(timeout);
    return this;
  }

  MultiQueryStreamOperation(
      ConnectionProxy&& connection,
      MultiQuery&& multi_query);

  MultiQueryStreamOperation(
      ConnectionProxy&& connection,
      std::vector<Query>&& queries);

  db::OperationType getOperationType() const override {
    return db::OperationType::MultiQueryStream;
  }

  template <typename C>
  void setCallback(C cb) {
    stream_callback_ = std::move(cb);
  }

 private:
  // wrapper to construct CallbackVistor and invoke the
  // right callback
  void invokeCallback(StreamState state);

  // Vistor to invoke the right callback depending on the type stored
  // in the variant 'stream_callback_'
  struct CallbackVisitor : public boost::static_visitor<> {
    CallbackVisitor(MultiQueryStreamOperation* op, StreamState state)
        : op_(op), state_(state) {}

    void operator()(MultiQueryStreamHandler* handler) const {
      DCHECK(op_ != nullptr);
      if (handler != nullptr) {
        handler->streamCallback(op_, state_);
      }
    }

    void operator()(Callback cb) const {
      DCHECK(op_ != nullptr);
      if (cb != nullptr) {
        cb(op_, state_);
      }
    }

   private:
    MultiQueryStreamOperation* op_;
    StreamState state_;
  };

  StreamCallback stream_callback_;
};

// An operation representing a query.  If a callback is set, it
// invokes the callback as rows arrive.  If there is no callback, it
// buffers all results into memory and makes them available as a
// RowBlock.  This is inefficient for large results.
//
// Constructed via Connection::beginQuery.
class QueryOperation : public FetchOperation {
 public:
  ~QueryOperation() override = default;

  void setCallback(QueryCallback cb) {
    buffered_query_callback_ = std::move(cb);
  }
  void chainCallback(QueryCallback cb) {
    auto origCb = std::move(buffered_query_callback_);
    if (origCb) {
      cb = [origCb = std::move(origCb), cb = std::move(cb)](
               QueryOperation& op,
               QueryResult* result,
               QueryCallbackReason reason) {
        origCb(op, result, reason);
        cb(op, result, reason);
      };
    }
    setCallback(cb);
  }

  // Steal all rows.  Only valid if there is no callback.  Inefficient
  // for large result sets.
  QueryResult&& stealQueryResult() {
    CHECK_THROW(ok(), db::OperationStateException);
    return std::move(*query_result_);
  }

  const QueryResult& queryResult() const {
    CHECK_THROW(ok(), db::OperationStateException);
    return *query_result_;
  }

  // Returns the Query of this operation
  const Query& getQuery() const {
    return queries_.getQuery(0);
  }

  // Steal all rows.  Only valid if there is no callback.  Inefficient
  // for large result sets.
  std::vector<RowBlock>&& stealRows() {
    return query_result_->stealRows();
  }

  const std::vector<RowBlock>& rows() const {
    return query_result_->rows();
  }

  // Last insert id (aka mysql_insert_id).
  uint64_t lastInsertId() const {
    return query_result_->lastInsertId();
  }

  // Number of rows affected (aka mysql_affected_rows).
  uint64_t numRowsAffected() const {
    return query_result_->numRowsAffected();
  }

  // Received gtid.
  const std::string& recvGtid() const {
    return query_result_->recvGtid();
  }

  void setQueryResult(QueryResult query_result) {
    query_result_ = std::make_unique<QueryResult>(std::move(query_result));
  }

  // Don't call this; it's public strictly for Connection to be able
  // to call make_shared.
  QueryOperation(ConnectionProxy&& connection, Query&& query);

  // Overriding to narrow the return type
  QueryOperation* setTimeout(Duration timeout) {
    Operation::setTimeout(timeout);
    return this;
  }

  QueryOperation* setUserData(folly::dynamic val) {
    Operation::setUserData(std::move(val));
    return this;
  }

  db::OperationType getOperationType() const override {
    return db::OperationType::Query;
  }

 protected:
  void notifyInitQuery() override;
  void notifyRowsReady() override;
  void notifyQuerySuccess(bool more_results) override;
  void notifyFailure(OperationResult result) override;
  void notifyOperationCompleted(OperationResult result) override;

 private:
  QueryCallback buffered_query_callback_;
  std::unique_ptr<QueryResult> query_result_;
  friend class Connection;
};

// An operation representing a query with multiple statements.
// If a callback is set, it invokes the callback as rows arrive.
// If there is no callback, it buffers all results into memory
// and makes them available as a RowBlock.
// This is inefficient for large results.
//
// Constructed via Connection::beginMultiQuery.
class MultiQueryOperation : public FetchOperation {
 public:
  ~MultiQueryOperation() override;

  // Set our callback.  This is invoked multiple times -- once for
  // every RowBatch and once, with nullptr for the RowBatch,
  // indicating the query is complete.
  void setCallback(MultiQueryCallback cb) {
    buffered_query_callback_ = std::move(cb);
  }
  void chainCallback(MultiQueryCallback cb) {
    auto origCb = std::move(buffered_query_callback_);
    if (origCb) {
      cb = [origCb = std::move(origCb), cb = std::move(cb)](
               MultiQueryOperation& op,
               QueryResult* result,
               QueryCallbackReason reason) {
        origCb(op, result, reason);
        cb(op, result, reason);
      };
    }
    setCallback(cb);
  }

  // Steal all rows. Only valid if there is no callback. Inefficient
  // for large result sets.
  // Only call after the query has finished, don't use it inside callbacks
  std::vector<QueryResult>&& stealQueryResults() {
    CHECK_THROW(done(), db::OperationStateException);
    return std::move(query_results_);
  }

  // Only call this after the query has finished and don't use it inside
  // callbacks
  const std::vector<QueryResult>& queryResults() const {
    CHECK_THROW(done(), db::OperationStateException);
    return query_results_;
  }

  // Returns the Query for a query index.
  const Query& getQuery(int index) const {
    return queries_.getQuery(index);
  }

  // Returns the list of Queries
  const std::vector<Query>& getQueries() const {
    return queries_.getQueries();
  }

  void setQueryResults(std::vector<QueryResult> query_results) {
    query_results_ = std::move(query_results);
  }

  // Don't call this; it's public strictly for Connection to be able
  // to call make_shared.
  MultiQueryOperation(
      ConnectionProxy&& connection,
      std::vector<Query>&& queries);

  // Overriding to narrow the return type
  MultiQueryOperation* setTimeout(Duration timeout) {
    Operation::setTimeout(timeout);
    return this;
  }

  MultiQueryOperation* setUserData(folly::dynamic val) {
    Operation::setUserData(std::move(val));
    return this;
  }

  db::OperationType getOperationType() const override {
    return db::OperationType::MultiQuery;
  }

 protected:
  void notifyInitQuery() override;
  void notifyRowsReady() override;
  void notifyQuerySuccess(bool more_results) override;
  void notifyFailure(OperationResult result) override;
  void notifyOperationCompleted(OperationResult result) override;

  // Calls the FetchOperation specializedCompleteOperation and then does
  // callbacks if needed

 private:
  MultiQueryCallback buffered_query_callback_;

  // Storage fields for every statement in the query
  // Only to be used if there is no callback set.
  std::vector<QueryResult> query_results_;
  // Buffer to trans to `query_results_` and for buffered callback.
  std::unique_ptr<QueryResult> current_query_result_;

  int num_current_query_ = 0;

  friend class Connection;
};

// SpecialOperation means operations like COM_RESET_CONNECTION,
// COM_CHANGE_USER, etc.
class SpecialOperation : public Operation {
 public:
  explicit SpecialOperation(ConnectionProxy&& conn)
      : Operation(std::move(conn)) {}
  void setCallback(SpecialOperationCallback callback) {
    callback_ = std::move(callback);
  }

 protected:
  void socketActionable() override;
  void specializedCompleteOperation() override;
  void specializedTimeoutTriggered() override;
  SpecialOperation* specializedRun() override;
  void mustSucceed() override;
  SpecialOperationCallback callback_{nullptr};
  friend class Connection;

 private:
  virtual MysqlHandler::Status callMysqlHandler() = 0;
  virtual const char* getErrorMsg() const = 0;
};

// This is for sending COM_RESET_CONNECTION command before returning an idle
// connection back to connection pool
class ResetOperation : public SpecialOperation {
 public:
  explicit ResetOperation(ConnectionProxy&& conn)
      : SpecialOperation(std::move(conn)) {}

 private:
  MysqlHandler::Status callMysqlHandler() override;
  db::OperationType getOperationType() const override {
    return db::OperationType::Reset;
  }
  const char* getErrorMsg() const override {
    return errorMsg;
  }
  static constexpr const char* errorMsg = "Reset connection failed: ";
};

class ChangeUserOperation : public SpecialOperation {
 public:
  explicit ChangeUserOperation(
      ConnectionProxy&& conn,
      const std::string& user,
      const std::string& password,
      const std::string& database)
      : SpecialOperation(std::move(conn)),
        user_(user),
        password_(password),
        database_(database) {}

 private:
  MysqlHandler::Status callMysqlHandler() override;
  db::OperationType getOperationType() const override {
    return db::OperationType::ChangeUser;
  }
  const char* getErrorMsg() const override {
    return errorMsg;
  }
  const std::string user_;
  const std::string password_;
  const std::string database_;
  static constexpr const char* errorMsg = "Change user failed: ";
};

// Helper function to build the result for a ConnectOperation in the sync
// mode. It will block the thread and return the acquired connection, in case
// of error, it will throw MysqlException as expected in the sync mode.
std::unique_ptr<Connection> blockingConnectHelper(
    std::shared_ptr<ConnectOperation> conn_op);
} // namespace mysql_client
} // namespace common
} // namespace facebook

#endif // COMMON_ASYNC_MYSQL_OPERATION_H
