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

import io.netty.util.ReferenceCountUtil;
import io.netty.util.ReferenceCounted;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.publisher.Hooks;

public final class ReactorHooks {
  private static final Logger LOGGER = LoggerFactory.getLogger(ReactorHooks.class);

  static {
    Hooks.onNextDropped(
        o -> {
          if (o instanceof ReferenceCounted) {
            ReferenceCounted r = (ReferenceCounted) o;
            ReferenceCountUtil.safeRelease(r);
          }
        });

    Hooks.onErrorDropped(t -> LOGGER.error("onErrorDropped exception", t));
  }

  private static final ReactorHooks INSTANCE = new ReactorHooks();

  private ReactorHooks() {}

  public static void init() {}
}
