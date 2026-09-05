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
import java.util.function.Function;
import org.apache.thrift.RequestRpcMetadata;
import org.apache.thrift.protocol.TMessage;

public interface ServerRequestPayload {
  /**
   * Materializes the request arguments by applying the given readers to the underlying request
   * data, then releases the request buffer.
   *
   * <p><b>Single-use.</b> This claims the request bytes, so it succeeds at most once. It holds the
   * buffer for the length of the read and frees it on the way out, which is what keeps a concurrent
   * {@link #releaseRequestData()} from freeing a buffer that is being read. Generated handlers call
   * this once and thereafter retain only the concrete Java arguments.
   *
   * @throws IllegalStateException if the request bytes are already gone, either because they were
   *     read once already or because the request ended first and the transport released them
   */
  List<Object> getData(List<Reader> readers);

  /**
   * Releases the framework's owning reference to the request buffer, if any.
   *
   * <p>The generated {@link com.facebook.thrift.server.RpcServerHandler} calls this in each method
   * body in a {@code finally} immediately after the synchronous read phase (after {@link #getData}
   * has materialized the arguments). Transports call this as an idempotent backstop for paths where
   * no generated method body reads the request, such as unknown-method dispatch or scheduler
   * rejection. This returns the request buffer to the allocator right after it is no longer needed,
   * instead of holding it until the response completes.
   *
   * <p>This and {@link #getData} claim the same request bytes, so exactly one of them frees the
   * buffer: whichever runs first. A release that loses the race to an in-progress read is a no-op,
   * and the read frees the buffer when it finishes. Repeat calls are a no-op. Payloads created
   * without an owned buffer (the no-buffer {@code create} overloads, e.g. for custom or test
   * implementations) carry no buffer here and this is a no-op for them.
   *
   * <p>Defaults to a no-op so custom/test {@link ServerRequestPayload} implementations need not
   * implement it; {@link DefaultServerRequestPayload} overrides it to release the owned buffer.
   */
  default void releaseRequestData() {}

  RequestRpcMetadata getRequestRpcMetadata();

  /**
   * The seq id found in {@link TMessage} can be different than the seq id found in the frame so
   * they need to be stored separately. This should go away when we switch to RSocket.
   *
   * @return the TMessage seq id
   */
  int getMessageSeqId();

  RequestContext getRequestContext();

  /**
   * Creates a payload holding {@code requestData} — the reference the framework owns bound to the
   * protocol positioned on it. {@link #getData} claims both together, which is what makes a
   * concurrent {@link #releaseRequestData()} safe.
   */
  static ServerRequestPayload create(
      RequestRpcMetadata requestRpcMetadata,
      RequestContext requestContext,
      int messageSeqId,
      RequestData requestData) {
    return new DefaultServerRequestPayload(
        requestRpcMetadata, requestContext, messageSeqId, requestData, null);
  }

  /**
   * As {@link #create(RequestRpcMetadata, RequestContext, int, RequestData)}, with the default
   * message seq id.
   */
  static ServerRequestPayload create(
      RequestRpcMetadata requestRpcMetadata,
      RequestContext requestContext,
      RequestData requestData) {
    return new DefaultServerRequestPayload(
        requestRpcMetadata, requestContext, 1, requestData, null);
  }

  /**
   * For callers that read the arguments through their own closure rather than a protocol the
   * payload carries. The bytes are still claimed as one unit, so the release stays safe.
   */
  static ServerRequestPayload create(
      Function<List<Reader>, List<Object>> readerTransformer,
      RequestRpcMetadata requestRpcMetadata,
      RequestContext requestContext,
      int messageSeqId,
      ReferenceCounted requestData) {
    return new DefaultServerRequestPayload(
        requestRpcMetadata,
        requestContext,
        messageSeqId,
        RequestData.ownerOnly(requestData),
        readerTransformer);
  }

  /** As above, with the default message seq id. */
  static ServerRequestPayload create(
      Function<List<Reader>, List<Object>> readerTransformer,
      RequestRpcMetadata requestRpcMetadata,
      RequestContext requestContext,
      ReferenceCounted requestData) {
    return create(readerTransformer, requestRpcMetadata, requestContext, 1, requestData);
  }

  static ServerRequestPayload create(
      Function<List<Reader>, List<Object>> readerTransformer,
      RequestRpcMetadata requestRpcMetadata,
      RequestContext requestContext,
      int messageSeqId) {
    return create(readerTransformer, requestRpcMetadata, requestContext, messageSeqId, null);
  }

  static ServerRequestPayload create(
      Function<List<Reader>, List<Object>> readerTransformer,
      RequestRpcMetadata requestRpcMetadata,
      RequestContext requestContext) {
    return create(readerTransformer, requestRpcMetadata, requestContext, 1, null);
  }
}
