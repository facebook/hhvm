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

final class DefaultServerRequestPayload implements ServerRequestPayload {
  private final Function<List<Reader>, List<Object>> readerTransformer;
  private final RequestRpcMetadata requestRpcMetadata;
  private final int messageSeqId;
  private final RequestContext requestContext;

  DefaultServerRequestPayload(
      Function<List<Reader>, List<Object>> readerTransformer,
      RequestRpcMetadata requestRpcMetadata,
      RequestContext requestContext,
      int messageSeqId) {
    this.readerTransformer = readerTransformer;
    this.requestRpcMetadata = requestRpcMetadata;
    this.requestContext = requestContext;
    this.messageSeqId = messageSeqId;
  }

  @Override
  public List<Object> getData(List<Reader> readers) {
    return readerTransformer.apply(readers);
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
