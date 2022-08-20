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

import org.apache.thrift.TException;
import org.apache.thrift.protocol.TProtocol;

/**
 * An interface that defines the write0 method. This class exists because the ASM code generates a
 * method write. Once the ASM code is removed then when we can remove this interface and replace it
 * with the Write interface.
 */
@FunctionalInterface
public interface ThriftSerializable {
  void write0(TProtocol protocol) throws TException;
}
