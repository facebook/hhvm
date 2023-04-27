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

package com.facebook.thrift.util;

public class PlatformUtils {
  public enum OS {
    LINUX,
    MAC,
    UNKNOWN,
  }

  public static OS getOS() {
    if (System.getProperty("os.name") == null) {
      return OS.UNKNOWN;
    }

    String operSys = System.getProperty("os.name").toLowerCase();
    if (operSys.contains("nix") || operSys.contains("nux") || operSys.contains("aix")) {
      return OS.LINUX;
    } else if (operSys.contains("mac")) {
      return OS.MAC;
    } else {
      return OS.UNKNOWN;
    }
  }
}
