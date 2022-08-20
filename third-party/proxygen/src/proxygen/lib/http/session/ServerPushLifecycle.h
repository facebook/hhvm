/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Optional.h>
#include <proxygen/lib/http/codec/HQFramer.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>

namespace proxygen {

class HTTPMessage;
class HTTPTransaction;

//  A callback class for observing pushed transactions.
//  It is built for HTTP3 state transitions, but is backward
//  compatible with HTTP2
//
//  The state transition diagram (states unique to HTTP3
//  are marked with "h3")
//
//  The sole purpose of this callback is tesing the correct
//  behavior of the server push.
//
//
//                        .-----------------.
//                      (       Start       )
//                       `-----------------'
//                                |
//                +---------------+--------------+
//                v                              v
//     +---------------------+       +---------------------h3
// +---| onPushPromiseBegin  |       |    onNascentBegin    |---+
// |   +---------------------+       +----------------------+   |
// |              |                              |              |
// |              v                              v              |
// |   +---------------------+         +------------------h3    |
// |   |    onPushPromise    |-------+ |  onNascentStream  |----+
// |   +---------------------+       | +-------------------+    |
// |              |                  |           |              |
// |              +---+              |           |              |
// |                  v              |           |              |
// |     +------------------------h3 |           |              |
// +--+--| onHalfPushedTransaction | |           +---+          |
// |  |  +-------------------------+ |               |          |
// |  |               |              |               |          |
// |  |               +--------------+               |          |
// |  |                              v               |          |
// |  +----------+      +-------------------------+  |          |
// |             +------| onFullPushedTransaction |<-+          |
// |             v      +-------------------------+             |
// |    .----------------h3          |                          |
// |   (onPushedTxnTimeout )         |  .-----------------h3    |
// |    `-----------------'          | ( onOrphanedNascent  )<--+
// |                              +--+  `------------------'    |
// |   .-----------------h3       |           .------------h3   |
// +->( onOrphanedHfOpnTxn )      |          ( onNascentEof  )<-+
//     `------------------'       |           `-------------'
//                                v
//                          .-----------.
//                         (  onTxnEof   )
//                          `-----------'
//
//
class ServerPushLifecycleCallback {
 public:
  virtual ~ServerPushLifecycleCallback() {
  }

  /**
   * A push promise has arrived, but has not been fully parsed yet
   */
  virtual void onPushPromiseBegin(HTTPCodec::StreamID /* parent streamID */,
                                  hq::PushId /* pushID */) = 0;

  /**
   * A push promise has arrived. Headers have been parsed.
   */
  virtual void onPushPromise(HTTPCodec::StreamID /* parent streamID */,
                             hq::PushId /* pushID */,
                             HTTPMessage* /* msg */) = 0;

  /**
   * A push stream has arrived, but the pushId has not been received yet
   */
  virtual void onNascentPushStreamBegin(
      HTTPCodec::StreamID /* push stream ID */, bool /* eom */) = 0;

  /**
   * A push stream has arrived, and the pushId has been received.
   */
  virtual void onNascentPushStream(HTTPCodec::StreamID /* push stream ID */,
                                   hq::PushId /* pushId */,
                                   bool /* eom */) = 0;

  /**
   * A push stream has ended before being converted to to full
   * ingress stream
   */
  virtual void onNascentEof(HTTPCodec::StreamID /* push stream ID */,
                            folly::Optional<hq::PushId> /* pushId */) = 0;

  /**
   * A nascent push stream has timed out. For more details, see
   * "The Personal History, Adventures, Experience and Observation
   * of David Copperfield the Younger of Blunderstone Rookery
   * (Which He Never Meant to Publish on Any Account).", Dickens C.
   */
  virtual void onOrphanedNascentStream(
      HTTPCodec::StreamID /* push stream ID */,
      folly::Optional<hq::PushId> /* pushId */) = 0;

  /**
   * A push promise has arrived, the push id and the assoc (parent) id
   * are known.
   */
  virtual void onHalfOpenPushedTxn(const HTTPTransaction* /* txn */,
                                   hq::PushId /* server push id */,
                                   HTTPCodec::StreamID /* assoc stream id */,
                                   bool /* eom */) = 0;

  /**
   * Both the promise and the stream have arrived. The newly created transport
   * can be created
   */
  virtual void onPushedTxn(const HTTPTransaction* /* txn */,
                           HTTPCodec::StreamID /* push ingress stream */,
                           hq::PushId /* server push id */,
                           HTTPCodec::StreamID /* assoc stream id */,
                           bool /* eom */) = 0;

  /**
   * A fully opened pushed transaction has timed out
   */
  virtual void onPushedTxnTimeout(const HTTPTransaction* /* txn */) = 0;

  /**
   * A half-opened push transaction has timed out
   */
  virtual void onOrphanedHalfOpenPushedTxn(
      const HTTPTransaction* /* txn */) = 0;

  /**
   * Push ID limit exceeded, possibly closing the stream
   */
  virtual void onPushIdLimitExceeded(
      hq::PushId /* incoming push id */,
      folly::Optional<hq::PushId> /* maximal allowed push id */,
      folly::Optional<HTTPCodec::StreamID> /* possible push stream */) = 0;

}; // ServerPushLifecycleCallback

} // namespace proxygen
