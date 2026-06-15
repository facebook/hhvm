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

//! Thrift enum definition node.

use std::collections::HashMap;
use std::sync::Arc;

use record::SerializableRecord;

use crate::error::InvalidTypeError;
use crate::field::AnnotationsMap;
use crate::type_ref::TypeRef;

/// A Thrift enum definition node.
#[derive(Debug)]
pub struct EnumNode {
    uri: String,
    values: Vec<EnumValue>,
    annotations: AnnotationsMap,
}

impl EnumNode {
    pub(crate) fn new(
        uri: String,
        values: Vec<EnumValue>,
        annotations: AnnotationsMap,
    ) -> Result<Self, InvalidTypeError> {
        let mut seen_values = HashMap::with_capacity(values.len());
        let mut seen_names = HashMap::with_capacity(values.len());

        for v in &values {
            if seen_values.insert(v.value, ()).is_some() {
                return Err(InvalidTypeError::DuplicateEnumValue(v.value, uri));
            }
            if seen_names.insert(&v.name, ()).is_some() {
                return Err(InvalidTypeError::DuplicateEnumName(v.name.clone(), uri));
            }
        }

        Ok(Self {
            uri,
            values,
            annotations,
        })
    }

    pub fn uri(&self) -> &str {
        &self.uri
    }

    pub fn values(&self) -> &[EnumValue] {
        &self.values
    }

    pub fn annotations(&self) -> &AnnotationsMap {
        &self.annotations
    }

    pub fn annotation(&self, uri: &str) -> Option<&SerializableRecord> {
        self.annotations.get(uri)
    }

    pub fn as_type_ref(self: &Arc<Self>) -> TypeRef {
        TypeRef::Enum(Arc::downgrade(self))
    }
}

/// A single value within an enum definition.
#[derive(Clone, Debug)]
pub struct EnumValue {
    pub name: String,
    pub value: i32,
    annotations: AnnotationsMap,
}

impl EnumValue {
    pub fn new(name: String, value: i32, annotations: AnnotationsMap) -> Self {
        Self {
            name,
            value,
            annotations,
        }
    }

    pub fn annotations(&self) -> &AnnotationsMap {
        &self.annotations
    }

    pub fn annotation(&self, uri: &str) -> Option<&SerializableRecord> {
        self.annotations.get(uri)
    }
}
