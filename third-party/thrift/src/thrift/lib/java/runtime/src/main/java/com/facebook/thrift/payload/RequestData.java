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

package com.facebook.thrift.payload;

import io.netty.util.ReferenceCounted;
import java.util.Objects;
import org.apache.thrift.protocol.TProtocol;

/**
 * The request bytes of one request: the reference the framework owns, and the protocol positioned
 * where the arguments begin.
 *
 * <p>Keeping the two together is what makes the ownership handoff safe. {@link
 * ServerRequestPayload#getData} claims this whole value with one atomic operation, so a reader
 * either takes the bytes and holds them for the length of the read, or finds them gone and reports
 * that. Holding the protocol somewhere else — captured in a lambda beside the payload, say — would
 * leave a way to read a buffer that has already been freed.
 *
 * <p>The owner and the protocol are not always over the same object. On the header transports the
 * owner is the {@code ThriftFrame} and the protocol reads {@code frame.getMessage()} from just past
 * the message envelope, which the transport reads early to build the request metadata. On the
 * Rocket transport the owner is the buffer the protocol reads.
 */
public final class RequestData {

  /** Nullable: absent when the transport keeps the buffer lifetime to itself. */
  private final ReferenceCounted owner;

  /** Nullable: absent when the caller reads the arguments through its own closure. */
  private final TProtocol protocol;

  private RequestData(ReferenceCounted owner, TProtocol protocol) {
    this.owner = owner;
    this.protocol = protocol;
  }

  /**
   * Binds a reference the payload will free to the protocol positioned on it.
   *
   * @param owner the reference to release once the arguments have been read
   * @param protocol positioned at the first argument, not at the start of the buffer
   */
  public static RequestData of(ReferenceCounted owner, TProtocol protocol) {
    return new RequestData(
        Objects.requireNonNull(owner, "owner"), Objects.requireNonNull(protocol, "protocol"));
  }

  /**
   * Carries only the reference to free, for callers that read the arguments through their own
   * closure. The bytes are still claimed as one unit, so the release stays safe.
   *
   * @param owner the reference to release, or null when the caller owns no buffer
   */
  public static RequestData ownerOnly(ReferenceCounted owner) {
    return new RequestData(owner, null);
  }

  TProtocol protocol() {
    return protocol;
  }

  void release() {
    if (owner != null) {
      owner.release();
    }
  }
}
