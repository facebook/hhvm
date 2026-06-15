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

//! Shared implementation for struct and union nodes.

use std::cell::UnsafeCell;
use std::collections::HashMap;

use crate::error::InvalidTypeError;
use crate::field::AnnotationsMap;
use crate::field::FastFieldHandle;
use crate::field::FieldDefinition;
use crate::structured_node::StructuredNode;

struct StructuredBody {
    fields: Vec<FieldDefinition>,
    field_index_by_id: HashMap<i16, u16>,
    field_index_by_name: HashMap<String, u16>,
}

/// Common storage for struct and union definition nodes.
///
/// Both `StructNode` and `UnionNode` are newtypes around this, differing only in
/// construction-time validation (unions require all fields to be optional).
///
/// Fields are behind an `UnsafeCell<Option<...>>`: a node shell can exist (and
/// be referenced via a `Weak` edge) before its field types — which may reference
/// not-yet-built nodes, including the node itself — are resolved. The builder
/// guarantees single-threaded writes in phase 2 and immutability after that.
pub(crate) struct StructuredNodeImpl {
    pub(crate) uri: String,
    pub(crate) is_sealed: bool,
    pub(crate) annotations: AnnotationsMap,
    body: UnsafeCell<Option<StructuredBody>>,
}

// SAFETY: the body is written exactly once (builder phase 2) before any read.
// The TypeSystem is immutable after construction; no concurrent mutation occurs.
// No manual Drop needed: Option<StructuredBody> drops correctly whether populated
// (Some) or not (None, e.g. if the builder errors mid-phase-2).
unsafe impl Send for StructuredNodeImpl {}
unsafe impl Sync for StructuredNodeImpl {}

impl std::fmt::Debug for StructuredNodeImpl {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("StructuredNodeImpl")
            .field("uri", &self.uri)
            .field("is_sealed", &self.is_sealed)
            .finish_non_exhaustive()
    }
}

impl StructuredNodeImpl {
    pub(crate) fn new_shell(uri: String, is_sealed: bool, annotations: AnnotationsMap) -> Self {
        Self {
            uri,
            is_sealed,
            annotations,
            body: UnsafeCell::new(None),
        }
    }

    /// Validate and install resolved fields. Called exactly once per node
    /// during the builder's phase 2, before the `TypeSystem` is returned.
    pub(crate) fn set_fields(&self, fields: Vec<FieldDefinition>) -> Result<(), InvalidTypeError> {
        if fields.len() > u16::MAX as usize {
            return Err(InvalidTypeError::TooManyFields(
                fields.len(),
                self.uri.clone(),
            ));
        }
        let mut field_index_by_id = HashMap::with_capacity(fields.len());
        let mut field_index_by_name = HashMap::with_capacity(fields.len());

        for (i, field) in fields.iter().enumerate() {
            let idx = i as u16;
            if field_index_by_id.insert(field.id(), idx).is_some() {
                return Err(InvalidTypeError::DuplicateFieldId(
                    field.id(),
                    self.uri.clone(),
                ));
            }
            if field_index_by_name
                .insert(field.name().to_owned(), idx)
                .is_some()
            {
                return Err(InvalidTypeError::DuplicateFieldName(
                    field.name().to_owned(),
                    self.uri.clone(),
                ));
            }
        }

        // SAFETY: no `&`-reference into the cell is live here — `body()` is the
        // only reader and is not called during phase 2, the node's `Arc` is not
        // yet published, and the builder is single-threaded — so this write can
        // neither alias nor race. The debug_assert enforces the "exactly once"
        // contract: a second populate would silently drop the prior body.
        unsafe {
            debug_assert!(
                (*self.body.get()).is_none(),
                "set_fields called more than once for {}",
                self.uri
            );
            *self.body.get() = Some(StructuredBody {
                fields,
                field_index_by_id,
                field_index_by_name,
            });
        }
        Ok(())
    }

    fn body(&self) -> &StructuredBody {
        // SAFETY: `set_fields` populated the body in phase 2 before the
        // TypeSystem was published, and it is never mutated afterward, so this
        // shared deref neither races a write nor observes `None`. The
        // debug_assert flags a read of an un-populated shell (a builder bug).
        unsafe {
            let body = &*self.body.get();
            debug_assert!(
                body.is_some(),
                "body read before set_fields for {}",
                self.uri
            );
            body.as_ref().unwrap_unchecked()
        }
    }
}

impl StructuredNode for StructuredNodeImpl {
    fn uri(&self) -> &str {
        &self.uri
    }

    fn fields(&self) -> &[FieldDefinition] {
        &self.body().fields
    }

    fn is_sealed(&self) -> bool {
        self.is_sealed
    }

    fn annotations(&self) -> &AnnotationsMap {
        &self.annotations
    }

    fn field_by_id(&self, id: i16) -> Option<&FieldDefinition> {
        let body = self.body();
        body.field_index_by_id
            .get(&id)
            .map(|&idx| &body.fields[idx as usize])
    }

    fn field_by_name(&self, name: &str) -> Option<&FieldDefinition> {
        let body = self.body();
        body.field_index_by_name
            .get(name)
            .map(|&idx| &body.fields[idx as usize])
    }

    fn field_handle_for_id(&self, id: i16) -> FastFieldHandle {
        self.body()
            .field_index_by_id
            .get(&id)
            .map_or(FastFieldHandle::INVALID, |&idx| FastFieldHandle::new(idx))
    }

    fn field_handle_for_name(&self, name: &str) -> FastFieldHandle {
        self.body()
            .field_index_by_name
            .get(name)
            .map_or(FastFieldHandle::INVALID, |&idx| FastFieldHandle::new(idx))
    }
}
