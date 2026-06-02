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
   * data.
   *
   * <p><b>Single-use for buffer-backed payloads:</b> the request buffer is released by {@link
   * #releaseRequestData()} as soon as the generated server handler finishes the (synchronous) read,
   * so {@code getData} reads the request bytes exactly once. Generated handlers call this once and
   * thereafter retain only the concrete Java arguments. Calling {@code getData} again after the
   * buffer has been released reads freed memory (failing fast with {@code
   * io.netty.util.IllegalReferenceCountException}). Call this at most once per request.
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
   * <p>Idempotent: the reference is dropped on the first call and the method is a no-op thereafter.
   * Payloads whose buffer lifetime is owned by the transport (e.g. RSocket) carry no buffer here
   * and this is a no-op for them.
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
   * Creates a payload that owns {@code requestData} — the framework's reference to the request
   * buffer (e.g. the THeader {@code ThriftFrame}). It is released via {@link #releaseRequestData()}
   * after the generated handler reads the arguments. Pass {@code null} when the transport manages
   * the buffer lifetime itself.
   */
  static ServerRequestPayload create(
      Function<List<Reader>, List<Object>> readerTransformer,
      RequestRpcMetadata requestRpcMetadata,
      RequestContext requestContext,
      int messageSeqId,
      ReferenceCounted requestData) {
    return new DefaultServerRequestPayload(
        readerTransformer, requestRpcMetadata, requestContext, messageSeqId, requestData);
  }

  static ServerRequestPayload create(
      Function<List<Reader>, List<Object>> readerTransformer,
      RequestRpcMetadata requestRpcMetadata,
      RequestContext requestContext,
      int messageSeqId) {
    return new DefaultServerRequestPayload(
        readerTransformer, requestRpcMetadata, requestContext, messageSeqId, null);
  }

  static ServerRequestPayload create(
      Function<List<Reader>, List<Object>> readerTransformer,
      RequestRpcMetadata requestRpcMetadata,
      RequestContext requestContext) {
    return new DefaultServerRequestPayload(
        readerTransformer, requestRpcMetadata, requestContext, 1, null);
  }
}
