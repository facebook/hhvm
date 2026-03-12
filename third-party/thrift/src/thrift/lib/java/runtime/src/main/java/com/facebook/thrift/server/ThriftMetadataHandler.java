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

package com.facebook.thrift.server;

import com.facebook.thrift.metadata.ThriftEnum;
import com.facebook.thrift.metadata.ThriftException;
import com.facebook.thrift.metadata.ThriftService;
import com.facebook.thrift.metadata.ThriftStruct;
import java.util.Map;

public interface ThriftMetadataHandler {

  String getName();

  String getFullyQualifiedName();

  ThriftService getThriftService();

  String getParentServiceName();

  Map<String, ThriftEnum> getEnums();

  Map<String, ThriftStruct> getStructs();

  Map<String, ThriftException> getExceptions();

  ThriftMetadataHandler getParentHandler();
}
