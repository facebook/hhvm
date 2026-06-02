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
import io.netty.util.ReferenceCounted;
import java.util.List;
import java.util.concurrent.atomic.AtomicReference;
import java.util.function.Function;
import org.apache.thrift.RequestRpcMetadata;

final class DefaultServerRequestPayload implements ServerRequestPayload {
  private final Function<List<Reader>, List<Object>> readerTransformer;
  private final RequestRpcMetadata requestRpcMetadata;
  private final int messageSeqId;
  private final RequestContext requestContext;

  // The framework's owning reference to the request buffer (the THeader ThriftFrame, or null when
  // the transport manages the buffer itself, e.g. RSocket). Released exactly once via
  // releaseRequestData(): the generated handler releases it eagerly right after the read, and the
  // transport calls it again as a backstop for paths where the generated handler never runs (e.g.
  // off-loop scheduler rejection under load shedding). getAndSet(null) makes the release atomic and
  // idempotent so exactly one caller frees the buffer.
  private final AtomicReference<ReferenceCounted> requestData;

  DefaultServerRequestPayload(
      Function<List<Reader>, List<Object>> readerTransformer,
      RequestRpcMetadata requestRpcMetadata,
      RequestContext requestContext,
      int messageSeqId,
      ReferenceCounted requestData) {
    this.readerTransformer = readerTransformer;
    this.requestRpcMetadata = requestRpcMetadata;
    this.requestContext = requestContext;
    this.messageSeqId = messageSeqId;
    this.requestData = new AtomicReference<>(requestData);
  }

  @Override
  public List<Object> getData(List<Reader> readers) {
    return readerTransformer.apply(readers);
  }

  @Override
  public void releaseRequestData() {
    ReferenceCounted rc = requestData.getAndSet(null);
    if (rc != null) {
      rc.release();
    }
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
