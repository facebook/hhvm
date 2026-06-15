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

//! Container types: list, set, and map.

use crate::type_ref::TypeRef;

#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub struct ListType {
    element: TypeRef,
}

impl ListType {
    pub fn new(element: TypeRef) -> Self {
        Self { element }
    }

    pub fn element_type(&self) -> &TypeRef {
        &self.element
    }
}

#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub struct SetType {
    element: TypeRef,
}

impl SetType {
    pub fn new(element: TypeRef) -> Self {
        Self { element }
    }

    pub fn element_type(&self) -> &TypeRef {
        &self.element
    }
}

#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub struct MapType {
    key: TypeRef,
    value: TypeRef,
}

impl MapType {
    pub fn new(key: TypeRef, value: TypeRef) -> Self {
        Self { key, value }
    }

    pub fn key_type(&self) -> &TypeRef {
        &self.key
    }

    pub fn value_type(&self) -> &TypeRef {
        &self.value
    }
}
