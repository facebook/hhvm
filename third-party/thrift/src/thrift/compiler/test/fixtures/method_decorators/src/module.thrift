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

include "thrift/annotation/cpp.thrift"

struct Request {
  1: string id;
}

struct Response {
  1: string text;
}

interaction LegacyPerforms {
  void perform();
}

interaction EchoInteraction {
  string interactionEcho(1: string text);
}

/**
 * This service just tests basic decorator generation
 */
@cpp.GenerateServiceMethodDecorator
service DecoratedService {
  performs LegacyPerforms;
  void noop();
  string echo(1: string text);
  i64 increment(1: i64 num);
  i64 sum(1: list<i64> nums);
  Response withStruct(1: Request request);
  Response multiParam(1: string text, 2: i64 num, 3: Request request);
  EchoInteraction echoInteraction();
}

/**
 * Baseline for a service with no decorators
 */
service UndecoratedService {
  void noop();
  string echo(1: string text);
  i64 increment(1: i64 num);
  i64 sum(1: list<i64> nums);
  Response withStruct(1: Request request);
  Response multiParam(1: string text, 2: i64 num, 3: Request request);
}

// Test 1st level of inheritance

/**
 * We'll check this codegen for the case where a service is Decorated, but its
 * parent has no decorators
 */
@cpp.GenerateServiceMethodDecorator
service DecoratedService_ExtendsUndecoratedService extends UndecoratedService {
  void extension();
}

/**
 * Decorated service that has a decorated parent
 */
@cpp.GenerateServiceMethodDecorator
service DecoratedService_ExtendsDecoratedService extends DecoratedService {
  void extension();
}

/**
 * Service with no decorator, but has decorated parent
 */
service UndecoratedService_ExtendsDecoratedService extends DecoratedService {
  void extension();
}

// Test 2nd level of inheritance

@cpp.GenerateServiceMethodDecorator
service DecoratedService_ExtendsUndecoratedService_ExtendsDecoratedService extends UndecoratedService_ExtendsDecoratedService {
  string secondExtension(1: string input);
}
