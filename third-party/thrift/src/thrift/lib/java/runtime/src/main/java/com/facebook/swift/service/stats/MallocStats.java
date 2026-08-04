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

package com.facebook.swift.service.stats;

import com.google.common.collect.ImmutableMap;
import java.util.Map;

/**
 * Native (glibc) allocator statistics, reported under the {@code memory.malloc.*} counter family.
 *
 * <p>This is the Java 8 baseline of a multi-release class. Reading glibc allocator state requires
 * the FFM API, so this implementation reports nothing; the {@code src/main/java25} override
 * supplies the real counters when the runtime is Java 25 or newer <em>and</em> the linker can
 * resolve {@code mallinfo2}. Exporting native allocator stats is best effort by design: callers get
 * an empty map rather than an error whenever the data cannot be obtained.
 *
 * @see com.facebook.thrift.malloc.malloc_h
 */
public class MallocStats {

  /**
   * Whether native allocator statistics can be collected on this runtime. Always false on the Java
   * 8 baseline.
   */
  public boolean isAvailable() {
    return false;
  }

  /**
   * A snapshot of native allocator usage, keyed by counter name.
   *
   * @return an empty map on this runtime; never null
   */
  public Map<String, Long> getMallocStats() {
    return ImmutableMap.of();
  }
}
