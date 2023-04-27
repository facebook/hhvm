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
import java.util.List;
import java.util.function.Function;
import org.apache.thrift.RequestRpcMetadata;
import org.apache.thrift.protocol.TMessage;

public interface ServerRequestPayload {
  List<Object> getData(List<Reader> readers);

  RequestRpcMetadata getRequestRpcMetadata();

  /**
   * The seq id found in {@link TMessage} can be different than the seq id found in the frame so
   * they need to be stored separately. This should go away when we switch to RSocket.
   *
   * @return the TMessage seq id
   */
  int getMessageSeqId();

  RequestContext getRequestContext();

  static ServerRequestPayload create(
      Function<List<Reader>, List<Object>> readerTransformer,
      RequestRpcMetadata requestRpcMetadata,
      RequestContext requestContext,
      int messageSeqId) {
    return new DefaultServerRequestPayload(
        readerTransformer, requestRpcMetadata, requestContext, messageSeqId);
  }

  static ServerRequestPayload create(
      Function<List<Reader>, List<Object>> readerTransformer,
      RequestRpcMetadata requestRpcMetadata,
      RequestContext requestContext) {
    return new DefaultServerRequestPayload(
        readerTransformer, requestRpcMetadata, requestContext, 1);
  }
}
