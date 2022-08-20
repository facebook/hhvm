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

include "thrift/annotation/scope.thrift"

package "facebook.com/thrift/annotation/api"

namespace java com.facebook.thrift.annotation.api_deprecated
namespace py.asyncio facebook_thrift_asyncio.annotation.api
namespace go thrift.annotation.api
namespace py thrift.annotation.api

/**
 * Indicates a field is ignored if sent as input.
 *
 * For example, life-cycle timestamps fields like `createTime` and `modifyTime`
 * should always be output-only, as they are set as side-effects of CRUD
 * operations.
 */
@scope.Field
struct OutputOnly {}

/**
 * Indicates a field cannot change after creation.
 *
 * For example, the name or id of a REST 'resource' and the primary-key of a
 * database 'row', cannot be changed after creation, as that would indicate a
 * different 'resource' or 'row'. Thus the only way to change such a field is
 * to delete and re-create the 'resource' or 'row' itself.
 */
@scope.Field
struct Immutable {}

/**
 * Indicates a field must contain a value that is unique in an outer context.
 *
 * For example, the uri of a Thrift type, the name or id of a REST 'resource',
 * and the primary-key of a 'row' in a database, all must be unique within the
 * containing universe, collection, or table.
 */
@scope.Field
struct Unique {}
