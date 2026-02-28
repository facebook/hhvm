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

package com.facebook.thrift.util.resources;

import java.util.HashSet;
import java.util.Set;
import org.junit.Assert;
import org.junit.Test;

public class ContextPropagationRegistryTest {
  @Test
  public void testRegistry() {
    Set<String> keys = new HashSet<>();
    Assert.assertEquals(false, ContextPropagationRegistry.isContextPropEnabled());
    Assert.assertEquals(keys, ContextPropagationRegistry.getContextPropagationKeys());

    keys.add("foo");
    ContextPropagationRegistry.registerContextPropagationKey("foo");

    Assert.assertEquals(true, ContextPropagationRegistry.isContextPropEnabled());
    Assert.assertEquals(keys, ContextPropagationRegistry.getContextPropagationKeys());
  }
}
