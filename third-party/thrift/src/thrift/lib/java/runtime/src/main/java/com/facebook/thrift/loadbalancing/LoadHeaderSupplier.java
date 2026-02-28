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

/**
 * A LoadHeaderSupplier will be applied via LoadHeaderRpcServerHandler. The request chain will
 * notify the LoadHeaderSupplier before the request is issued, onRequest(), after the request has
 * completed (successfully or exceptionally) with doFinally().
 *
 * <p>The LoadHeaderRpcServerHandler will apply the value returned from getLoad() to the "load"
 * header on the response if "load":"default" header is attached to the thrift request. If a custom
 * load metric is attached the header this class will have no effect.
 */
public interface LoadHeaderSupplier {
  void onRequest();

  void doFinally();

  long getLoad();
}
