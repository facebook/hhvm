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

//! The `StructuredNode` trait shared by struct and union nodes.

use record::SerializableRecord;

use crate::field::AnnotationsMap;
use crate::field::FastFieldHandle;
use crate::field::FieldDefinition;

/// Common interface for struct and union nodes.
pub trait StructuredNode {
    fn uri(&self) -> &str;
    fn fields(&self) -> &[FieldDefinition];
    fn is_sealed(&self) -> bool;
    fn annotations(&self) -> &AnnotationsMap;

    fn annotation(&self, uri: &str) -> Option<&SerializableRecord> {
        self.annotations().get(uri)
    }

    fn field_by_id(&self, id: i16) -> Option<&FieldDefinition> {
        self.fields().iter().find(|f| f.id() == id)
    }

    fn field_by_name(&self, name: &str) -> Option<&FieldDefinition> {
        self.fields().iter().find(|f| f.name() == name)
    }

    fn has_field_id(&self, id: i16) -> bool {
        self.field_by_id(id).is_some()
    }

    fn has_field_name(&self, name: &str) -> bool {
        self.field_by_name(name).is_some()
    }

    fn field_handle_for_id(&self, id: i16) -> FastFieldHandle {
        self.fields()
            .iter()
            .position(|f| f.id() == id)
            .map_or(FastFieldHandle::INVALID, |i| FastFieldHandle::new(i as u16))
    }

    fn field_handle_for_name(&self, name: &str) -> FastFieldHandle {
        self.fields()
            .iter()
            .position(|f| f.name() == name)
            .map_or(FastFieldHandle::INVALID, |i| FastFieldHandle::new(i as u16))
    }

    fn field_by_handle(&self, handle: FastFieldHandle) -> Option<&FieldDefinition> {
        if handle.is_valid() {
            self.fields().get(handle.index() as usize)
        } else {
            None
        }
    }
}
