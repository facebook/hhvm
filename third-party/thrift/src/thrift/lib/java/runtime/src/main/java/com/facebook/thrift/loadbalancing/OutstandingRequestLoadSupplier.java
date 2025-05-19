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

package com.facebook.thrift.loadbalancing;

import java.util.concurrent.atomic.AtomicLong;

/** Default load supplier that will always supply a load = number_outstanding_requests */
public class OutstandingRequestLoadSupplier implements LoadHeaderSupplier {
  private final AtomicLong outstandingRequests = new AtomicLong(0);

  @Override
  public void onRequest() {
    outstandingRequests.incrementAndGet();
  }

  @Override
  public void doFinally() {
    outstandingRequests.decrementAndGet();
  }

  @Override
  public long getLoad() {
    return outstandingRequests.get();
  }
}
