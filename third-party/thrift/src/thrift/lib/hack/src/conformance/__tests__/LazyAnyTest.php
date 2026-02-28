<?hh
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
 *
 */

<<Oncalls('search_topaggr'), Feature('FBApp_Search_IndexServe')>>
final class LazyAnyTest extends LazyAnyTestBase {

  use ClassLevelTest;

  const class<IThriftStruct> CLASSNAME = unicorn_test_MainStruct::class;
  const classname<TProtocolSerializer> DATA_SERIALIZER =
    TCompactSerializer::class;
  const ?apache_thrift_StandardProtocol THRIFT_PROTOCOL = null;
  const string CPP_HEX_BINARY_SERIALIZED =
    "1c2810d58158475b86f69ea747c26d807b4d89380d150a292803666f6f03626172000000";
}

<<Oncalls('search_topaggr'), Feature('FBApp_Search_IndexServe')>>
final class LazyAnySimpleJsonTest extends LazyAnyTestBase {

  use ClassLevelTest;

  const class<IThriftStruct> CLASSNAME =
    unicorn_test_MainStructSimpleJson::class;
  const classname<TProtocolSerializer> DATA_SERIALIZER =
    JSONThriftSerializer::class;
  const ?apache_thrift_StandardProtocol THRIFT_PROTOCOL =
    apache_thrift_StandardProtocol::SimpleJson;
  const string CPP_HEX_BINARY_SERIALIZED =
    "1c2810d58158475b86f69ea747c26d807b4d891508281d7b226e756d223a352c22766563223a5b22666f6f222c22626172225d7d0000";
}
