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

#pragma once

#include <thrift/lib/cpp2/async/processor/AsyncProcessor.h>
#include <thrift/lib/cpp2/async/processor/AsyncProcessorFunc.h>
#include <thrift/lib/cpp2/async/processor/EventTask.h>
#include <thrift/lib/cpp2/async/processor/GeneratedAsyncProcessorBase.h>
#include <thrift/lib/cpp2/async/processor/HandlerCallback.h>
#include <thrift/lib/cpp2/async/processor/HandlerCallbackBase.h>
#include <thrift/lib/cpp2/async/processor/HandlerCallbackOneWay.h>
#include <thrift/lib/cpp2/async/processor/RequestParams.h>
#include <thrift/lib/cpp2/async/processor/RequestTask.h>
#include <thrift/lib/cpp2/async/processor/ServerInterface.h>
#include <thrift/lib/cpp2/async/processor/ServerRequest.h>
#include <thrift/lib/cpp2/async/processor/ServerRequestHelper.h>
#include <thrift/lib/cpp2/async/processor/ServerRequestTask.h>
#include <thrift/lib/cpp2/async/processor/ServiceHandlerBase.h>
