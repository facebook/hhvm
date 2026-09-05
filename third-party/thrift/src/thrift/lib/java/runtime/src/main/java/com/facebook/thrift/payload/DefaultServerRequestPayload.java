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

import com.facebook.nifty.core.RequestContext;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.atomic.AtomicReference;
import java.util.function.Function;
import org.apache.thrift.RequestRpcMetadata;
import org.apache.thrift.protocol.TProtocol;

final class DefaultServerRequestPayload implements ServerRequestPayload {
  private final RequestRpcMetadata requestRpcMetadata;
  private final int messageSeqId;
  private final RequestContext requestContext;

  // The request bytes, held once. Reading and releasing both claim this cell with getAndSet, so
  // exactly one of them wins and becomes responsible for the buffer:
  //
  //   - the reader wins: it owns the buffer for the whole read and frees it afterwards, and a
  //     concurrent release finds the cell empty and does nothing;
  //   - the release wins: it frees at once, and a later read finds the cell empty and reports it.
  //
  // Neither order can free the buffer while a reader is part way through it, which matters because
  // the two can run on different threads: the generated handler reads on the off-loop scheduler,
  // while a cancelled or expired request terminates the sequence elsewhere. An idempotent release
  // would not be enough — that prevents a double free, not a free underneath a reader, which hands
  // a pooled buffer to another request mid-read.
  private final AtomicReference<RequestData> requestData;

  // Set only by the create overloads that take a Function, for callers that read the arguments
  // through their own closure instead of the protocol carried in requestData.
  private final Function<List<Reader>, List<Object>> customReader;

  DefaultServerRequestPayload(
      RequestRpcMetadata requestRpcMetadata,
      RequestContext requestContext,
      int messageSeqId,
      RequestData requestData,
      Function<List<Reader>, List<Object>> customReader) {
    this.requestRpcMetadata = requestRpcMetadata;
    this.requestContext = requestContext;
    this.messageSeqId = messageSeqId;
    this.requestData = new AtomicReference<>(requestData);
    this.customReader = customReader;
  }

  @Override
  public List<Object> getData(List<Reader> readers) {
    RequestData claimed = requestData.getAndSet(null);
    if (claimed == null) {
      throw new IllegalStateException(
          "request data for "
              + requestRpcMetadata.getName()
              + " was already released; it can only be read once, and not after the request ends");
    }
    try {
      return customReader != null
          ? customReader.apply(readers)
          : readArgs(readers, claimed.protocol());
    } finally {
      claimed.release();
    }
  }

  @Override
  public void releaseRequestData() {
    RequestData claimed = requestData.getAndSet(null);
    if (claimed != null) {
      claimed.release();
    }
  }

  /** Both server transports frame their arguments identically, so this serves all of them. */
  private static List<Object> readArgs(List<Reader> readers, TProtocol protocol) {
    protocol.readStructBegin();
    List<Object> requestArguments = Collections.emptyList();
    if (readers != null && !readers.isEmpty()) {
      requestArguments = new ArrayList<>(readers.size());
      for (Reader r : readers) {
        protocol.readFieldBegin();
        requestArguments.add(r.read(protocol));
        protocol.readFieldEnd();
      }
    }
    protocol.readStructEnd();
    protocol.readMessageEnd();
    return requestArguments;
  }

  @Override
  public RequestRpcMetadata getRequestRpcMetadata() {
    return requestRpcMetadata;
  }

  @Override
  public RequestContext getRequestContext() {
    return requestContext;
  }

  public int getMessageSeqId() {
    return messageSeqId;
  }
}
