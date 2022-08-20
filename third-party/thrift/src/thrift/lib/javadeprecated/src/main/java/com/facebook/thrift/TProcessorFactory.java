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

package com.facebook.thrift;

import com.facebook.thrift.transport.TTransport;

/** The default processor factory just returns a singleton instance. */
public class TProcessorFactory {

  private final TProcessor processor_;

  public TProcessorFactory(TProcessor processor) {
    processor_ = processor;
  }

  public TProcessor getProcessor(TTransport trans) {
    return processor_;
  }
}
