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
include "thrift/annotation/thrift.thrift"

package "facebook.com/thrift/annotation/api"

namespace java com.facebook.thrift.annotation.api_deprecated
namespace py.asyncio facebook_thrift_asyncio.annotation.api
namespace go thrift.annotation.api
namespace py thrift.annotation.api

/**
 * Indicates a field is ignored if sent as an input parameter.
 *
 * For example, life-cycle timestamps fields like `createTime` and `modifyTime`
 * should always be output-only, as they are set as side-effects of CRUD
 * operations.
 */
@scope.Field
@scope.Schema
struct OutputOnly {}

/**
 * Indicates a field is never returned as an output parameter.
 *
 * For example, a field with secret or sensitive information that should never
 * leave the server could use this annotation:
 *
 *   struct AsemetricKeys {
 *     @api.Immutable
 *     @api.Unique
 *     1: string name;
 *
 *     // The public key data.
 *     @api.Unique
 *     2: binary publicKey;
 *
 *     // The private key data.
 *     @api.InputOnly
 *     3: binary privateKey;
 *
 *     // True if a `privateKey` has been set.
 *     @api.OuputOnly
 *     4: bool hasPrivateKey;
 *   }
 *
 * In this case, both publicKey and privateKey can still be set/updated and
 * `hasPrivateKey` can be used to indicate presence, if needed.
 */
@thrift.Testing // TODO(afuller): Enforce with conformance tests before releasing, even to experimental.
@scope.Field
@scope.Schema
struct InputOnly {}

/**
 * Indicates a field cannot change after creation.
 *
 * For example, the name or id of a REST 'resource' and the primary-key of a
 * database 'row', cannot be changed after creation, as that would indicate a
 * different 'resource' or 'row'. Thus the only way to change such a field is
 * to delete and re-create the 'resource' or 'row' itself.
 */
@scope.Field
@scope.Schema
struct Immutable {}

/**
 * Indicates a field must contain a value that is unique in an outer context.
 *
 * For example, the uri of a Thrift type, the name or id of a REST 'resource',
 * and the primary-key of a 'row' in a database, all must be unique within the
 * containing universe, collection, or table.
 */
@scope.Field
@scope.Schema
struct Unique {}
