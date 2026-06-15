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

//! Thrift struct definition node.

use std::sync::Arc;

use super::structured_impl::StructuredNodeImpl;
use crate::error::InvalidTypeError;
use crate::field::AnnotationsMap;
use crate::field::FastFieldHandle;
use crate::field::FieldDefinition;
use crate::structured_node::StructuredNode;
use crate::type_ref::TypeRef;

/// A Thrift struct definition node.
#[derive(Debug)]
pub struct StructNode(pub(crate) StructuredNodeImpl);

impl StructNode {
    pub(crate) fn new_shell(uri: String, is_sealed: bool, annotations: AnnotationsMap) -> Self {
        Self(StructuredNodeImpl::new_shell(uri, is_sealed, annotations))
    }

    pub(crate) fn set_fields(&self, fields: Vec<FieldDefinition>) -> Result<(), InvalidTypeError> {
        self.0.set_fields(fields)
    }

    pub fn as_type_ref(self: &Arc<Self>) -> TypeRef {
        TypeRef::Struct(Arc::downgrade(self))
    }
}

impl StructuredNode for StructNode {
    fn uri(&self) -> &str {
        self.0.uri()
    }
    fn fields(&self) -> &[FieldDefinition] {
        self.0.fields()
    }
    fn is_sealed(&self) -> bool {
        self.0.is_sealed()
    }
    fn annotations(&self) -> &AnnotationsMap {
        self.0.annotations()
    }

    fn field_by_id(&self, id: i16) -> Option<&FieldDefinition> {
        self.0.field_by_id(id)
    }
    fn field_by_name(&self, name: &str) -> Option<&FieldDefinition> {
        self.0.field_by_name(name)
    }
    fn field_handle_for_id(&self, id: i16) -> FastFieldHandle {
        self.0.field_handle_for_id(id)
    }
    fn field_handle_for_name(&self, name: &str) -> FastFieldHandle {
        self.0.field_handle_for_name(name)
    }
}
