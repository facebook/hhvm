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

package com.facebook.thrift.client;

import java.net.SocketAddress;
import java.util.Objects;

public class TierSocketAddress extends SocketAddress {
  private final String tierName;

  public TierSocketAddress(String tierName) {
    this.tierName = Objects.requireNonNull(tierName);
  }

  public String getTierName() {
    return tierName;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;
    TierSocketAddress that = (TierSocketAddress) o;
    return Objects.equals(tierName, that.tierName);
  }

  @Override
  public int hashCode() {
    return Objects.hash(tierName);
  }
}
