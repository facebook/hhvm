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

package com.meta.thrift.conformance.data.server;

import com.facebook.thrift.any.LazyAny;
import com.facebook.thrift.client.RpcOptions;
import com.facebook.thrift.util.SerializationProtocol;
import org.apache.thrift.TException;
import org.apache.thrift.conformance.Any;
import org.apache.thrift.conformance.ConformanceService;
import org.apache.thrift.conformance.PatchOpRequest;
import org.apache.thrift.conformance.PatchOpResponse;
import org.apache.thrift.conformance.RoundTripRequest;
import org.apache.thrift.conformance.RoundTripResponse;
import org.apache.thrift.conformance.StandardProtocol;

public class ConformanceServiceImpl implements ConformanceService {

  @Override
  public RoundTripResponse roundTrip(RoundTripRequest roundTripRequest) throws TException {
    Any any = roundTripRequest.getValue();

    LazyAny request = LazyAny.wrap(any);
    Object o = request.get();

    LazyAny.Builder builder = new LazyAny.Builder(o);

    StandardProtocol returnProtocol = any.getProtocol();
    if (roundTripRequest.getTargetProtocol() != null) {
      returnProtocol = roundTripRequest.getTargetProtocol().getStandard();
    }

    if (returnProtocol == null) {
      returnProtocol = StandardProtocol.COMPACT;
    }

    if (returnProtocol == StandardProtocol.COMPACT) {
      builder.setProtocol(SerializationProtocol.TCompact);
    } else if (returnProtocol == StandardProtocol.BINARY) {
      builder.setProtocol(SerializationProtocol.TBinary);
    } else if (returnProtocol == StandardProtocol.JSON) {
      builder.setProtocol(SerializationProtocol.TJSON);
    } else if (returnProtocol == StandardProtocol.SIMPLE_JSON) {
      builder.setProtocol(SerializationProtocol.TSimpleJSON);
    }

    LazyAny response = builder.build();
    return new RoundTripResponse.Builder().setValue(response.getAny()).build();
  }

  @Override
  public RoundTripResponse roundTrip(RoundTripRequest request, RpcOptions rpcOptions)
      throws TException {
    return roundTrip(request);
  }

  @Override
  public PatchOpResponse patch(PatchOpRequest roundTripRequest) throws TException {
    throw new UnsupportedOperationException("Not implemented");
  }

  @Override
  public PatchOpResponse patch(PatchOpRequest request, RpcOptions rpcOptions) throws TException {
    return patch(request);
  }

  @Override
  public void close() {}
}
