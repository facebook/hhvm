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

package com.facebook.thrift.transport.unified;

/**
 * Netty user event fired by {@link OptionalSslHandler} when an inbound connection is confirmed to
 * be plaintext (i.e. the first 5 bytes are not a TLS record header).
 *
 * <p>{@link DeferChannelActiveHandler} listens for this event to release a deferred {@code
 * channelActive} on the plaintext branch, mirroring the role that {@code
 * SslHandshakeCompletionEvent.SUCCESS} plays on the TLS branch.
 */
enum PlaintextConfirmedEvent {
  INSTANCE
}
