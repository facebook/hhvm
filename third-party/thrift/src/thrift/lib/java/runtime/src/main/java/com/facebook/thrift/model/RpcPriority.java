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

package com.facebook.thrift.model;

import com.facebook.swift.codec.ThriftEnumValue;

public enum RpcPriority {
  HIGH_IMPORTANT(0),
  HIGH(1),
  IMPORTANT(2),
  NORMAL(3),
  BEST_EFFORT(4),
  N_PRIORITIES(5);

  private final int value;

  private RpcPriority(int value) {
    this.value = value;
  }

  @ThriftEnumValue
  public int getValue() {
    return this.value;
  }

  public static RpcPriority fromInteger(int n) {
    switch (n) {
      case 0:
        return HIGH_IMPORTANT;
      case 1:
        return HIGH;
      case 2:
        return IMPORTANT;
      case 3:
        return NORMAL;
      case 4:
        return BEST_EFFORT;
      case 5:
        return N_PRIORITIES;
      default:
        return null;
    }
  }
}
