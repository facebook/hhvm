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

#include <thrift/compiler/source_location.h>

// This file is a shim of a part of the Thrift compiler infrastructure.
namespace whisker {
using source_location = apache::thrift::compiler::source_location;
using source_range = apache::thrift::compiler::source_range;
using resolved_location = apache::thrift::compiler::resolved_location;
using source = apache::thrift::compiler::source_view;
using source_manager = apache::thrift::compiler::source_manager;
} // namespace whisker
