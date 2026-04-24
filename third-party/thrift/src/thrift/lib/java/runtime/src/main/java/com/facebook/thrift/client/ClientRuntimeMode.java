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

/** Selects which Java client runtime backs generated typed clients. */
public enum ClientRuntimeMode {
  LEGACY,
  V2;

  public static ClientRuntimeMode fromString(String value) {
    if (value == null || value.isEmpty()) {
      return LEGACY;
    }

    switch (value.trim().toLowerCase()) {
      case "legacy":
        return LEGACY;
      case "v2":
        return V2;
      default:
        throw new IllegalArgumentException(
            "Unsupported thrift Java client runtime: " + value + ". Expected legacy or v2.");
    }
  }
}
