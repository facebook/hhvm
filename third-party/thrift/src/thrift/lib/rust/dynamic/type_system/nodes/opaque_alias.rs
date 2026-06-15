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

//! Thrift opaque alias definition node.

use std::cell::UnsafeCell;
use std::sync::Arc;

use record::SerializableRecord;

use crate::error::InvalidTypeError;
use crate::field::AnnotationsMap;
use crate::type_ref::TypeRef;

/// A Thrift opaque alias definition node.
///
/// The target is behind an `UnsafeCell<Option<...>>`: a target such as
/// `list<SomeStruct>` can transitively reference a not-yet-built node, so it
/// is resolved in the builder's second pass via [`Self::set_target`]. The
/// builder guarantees single-threaded writes and immutability after that.
pub struct OpaqueAliasNode {
    uri: String,
    annotations: AnnotationsMap,
    target_type: UnsafeCell<Option<TypeRef>>,
}

// SAFETY: the target is written exactly once (builder phase 2) before any read.
// The TypeSystem is immutable after construction; no concurrent mutation occurs.
// No manual Drop needed: Option<TypeRef> drops correctly whether populated or not.
unsafe impl Send for OpaqueAliasNode {}
unsafe impl Sync for OpaqueAliasNode {}

impl std::fmt::Debug for OpaqueAliasNode {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("OpaqueAliasNode")
            .field("uri", &self.uri)
            .finish_non_exhaustive()
    }
}

impl OpaqueAliasNode {
    pub(crate) fn new_shell(uri: String, annotations: AnnotationsMap) -> Self {
        Self {
            uri,
            annotations,
            target_type: UnsafeCell::new(None),
        }
    }

    /// Validate and install the resolved target. Called exactly once per node
    /// during the builder's phase 2, before the `TypeSystem` is returned.
    pub(crate) fn set_target(&self, target_type: TypeRef) -> Result<(), InvalidTypeError> {
        if target_type.is_struct()
            || target_type.is_union()
            || target_type.is_enum()
            || target_type.is_opaque_alias()
        {
            return Err(InvalidTypeError::InvalidOpaqueAlias(self.uri.clone()));
        }
        // SAFETY: no `&`-reference into the cell is live here — `target_type()`
        // is the only reader and is not called during phase 2, the node's `Arc`
        // is not yet published, and the builder is single-threaded — so this
        // write can neither alias nor race. The debug_assert enforces the
        // "exactly once" contract: a second populate would silently drop the
        // prior target.
        unsafe {
            debug_assert!(
                (*self.target_type.get()).is_none(),
                "set_target called more than once for {}",
                self.uri
            );
            *self.target_type.get() = Some(target_type);
        }
        Ok(())
    }

    pub fn uri(&self) -> &str {
        &self.uri
    }

    pub fn target_type(&self) -> &TypeRef {
        // SAFETY: `set_target` populated the target in phase 2 before the
        // TypeSystem was published, and it is never mutated afterward, so this
        // shared deref neither races a write nor observes `None`. The
        // debug_assert flags a read of an un-populated shell (a builder bug).
        unsafe {
            let target = &*self.target_type.get();
            debug_assert!(
                target.is_some(),
                "target_type read before set_target for {}",
                self.uri
            );
            target.as_ref().unwrap_unchecked()
        }
    }

    pub fn annotations(&self) -> &AnnotationsMap {
        &self.annotations
    }

    pub fn annotation(&self, uri: &str) -> Option<&SerializableRecord> {
        self.annotations.get(uri)
    }

    pub fn as_type_ref(self: &Arc<Self>) -> TypeRef {
        TypeRef::OpaqueAlias(Arc::downgrade(self))
    }
}
