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

package com.facebook.swift.service;

import com.facebook.nifty.ssl.SslSession;
import io.netty.util.AttributeKey;

@Deprecated
public class SwiftConstants {
  public static final String STICKY_HASH_KEY = "LB_STICKY_HASH";
  public static final String UPGRADE_TO_ROCKET_METHOD_NAME = "upgradeToRocket";
  public static final AttributeKey<SslSession> THRIFT_SSL_SESSION_KEY =
      AttributeKey.newInstance("sslSession");
}
