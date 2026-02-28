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

package com.facebook.thrift.exceptions;

import static java.util.Objects.requireNonNull;

import com.google.common.net.HostAndPort;
import org.apache.thrift.transport.TTransportException;

public class ConnectionFailedException extends TTransportException {
  private final HostAndPort address;

  public ConnectionFailedException(HostAndPort address, Throwable cause) {
    super("Failed to connect to " + address, cause);
    this.address = requireNonNull(address, "address is null");
  }

  public HostAndPort getAddress() {
    return address;
  }
}
