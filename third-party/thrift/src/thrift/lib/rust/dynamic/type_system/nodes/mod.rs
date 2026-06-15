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

//! Node types for the type system graph: struct, union, enum, opaque alias,
//! and container types (list, set, map).

mod containers;
mod enum_node;
mod opaque_alias;
mod struct_node;
pub(crate) mod structured_impl;
mod union_node;

pub use containers::ListType;
pub use containers::MapType;
pub use containers::SetType;
pub use enum_node::EnumNode;
pub use enum_node::EnumValue;
pub use opaque_alias::OpaqueAliasNode;
pub use struct_node::StructNode;
pub use union_node::UnionNode;
