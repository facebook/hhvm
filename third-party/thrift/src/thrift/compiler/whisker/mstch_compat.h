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

#include <thrift/compiler/detail/mustache/mstch.h>
#include <thrift/compiler/whisker/object.h>

namespace whisker {

// See mstch.h for more details

/**
 * A mstch::object is analogous to whisker::native_object.
 */
using mstch_object = apache::thrift::mstch::object;
/**
 * A mstch::map is analogous to whisker::map.
 */
using mstch_map = apache::thrift::mstch::map;
/**
 * A mstch::array is analogous to whisker::array.
 */
using mstch_array = apache::thrift::mstch::array;
/**
 * A mstch::node is analogous to whisker::object.
 */
using mstch_node = apache::thrift::mstch::node;

/**
 * Creates a whisker::object that proxies property and array lookups to the
 * provided mstch::node.
 *
 * For existing mstch::node objects, property lookups will be functionally
 * identical when performed on the returned whisker::object instead.
 *
 * Marshaling of "complex" data types between mstch and Whisker occurs lazily as
 * lookups are performed. The lazily marshaled types are:
 *   - mstch_array (all elements are marshaled on first access)
 *   - mstch_map (every key is marshaled lazily)
 *
 * mstch_object is marshaled lazily by construction since its properties are not
 * finitely enumerable and their values may be volatile (change between
 * invocations).
 *
 * The smaller data types (such as i64, f64, string etc.) are eagerly marshaled.
 * Note that, such data contained within a mstch_array or mstch_map will be
 * lazily marshaled. In other words, the eager marshaling only applies to values
 * in the object tree at depth 0.
 *
 * Internally, this function uses whisker::native_object to implement proxying.
 * Therefore, from_mstch() with a mstch_array as input will not create a
 * whisker::array and so calling is_array() on it will return false. Instead the
 * helper functions of the family, is_mstch_<type>, can be used to check for
 * such objects.
 */
object from_mstch(mstch_node);

/**
 * Determines if the provided object is proxying a mstch_object internally.
 */
bool is_mstch_object(const object&);
/**
 * Determines if the provided object is proxying a mstch_map internally.
 */
bool is_mstch_map(const object&);
/**
 * Determines if the provided object is proxying a mstch_array internally.
 */
bool is_mstch_array(const object&);

} // namespace whisker
