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

//! Field definitions, annotations, and related types.

use std::collections::HashMap;

use record::SerializableRecord;

use crate::type_ref::TypeRef;

/// Annotations map: URI -> serializable record value.
#[derive(Clone, Default, Debug)]
pub struct AnnotationsMap(HashMap<String, SerializableRecord>);

impl std::ops::Deref for AnnotationsMap {
    type Target = HashMap<String, SerializableRecord>;
    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl std::ops::DerefMut for AnnotationsMap {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

impl From<HashMap<String, SerializableRecord>> for AnnotationsMap {
    fn from(map: HashMap<String, SerializableRecord>) -> Self {
        Self(map)
    }
}

impl FromIterator<(String, SerializableRecord)> for AnnotationsMap {
    fn from_iter<I: IntoIterator<Item = (String, SerializableRecord)>>(iter: I) -> Self {
        Self(iter.into_iter().collect())
    }
}

/// Compact handle for O(1) field access by ordinal index.
///
/// `0` is invalid; valid handles are `1..=field_count` (ordinal = index + 1).
#[derive(Copy, Clone, Debug, PartialEq, Eq, Hash)]
pub struct FastFieldHandle(u16);

impl FastFieldHandle {
    pub const INVALID: Self = Self(0);

    pub fn new(index: u16) -> Self {
        Self(index + 1)
    }

    pub fn is_valid(self) -> bool {
        self.0 != 0
    }

    /// Returns the 0-based index. Panics if the handle is invalid.
    pub fn index(self) -> u16 {
        assert!(self.is_valid(), "attempted to use invalid FastFieldHandle");
        self.0 - 1
    }
}

/// Identity of a field within a structured type.
#[derive(Clone, Debug)]
pub struct FieldIdentity {
    pub id: i16,
    pub name: String,
}

/// Presence qualifier for a field (maps to thrift-generated `PresenceQualifier`).
#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub struct PresenceQualifier(pub i32);

impl PresenceQualifier {
    pub const DEFAULT_INITIALIZED: Self = Self(0);
    pub const UNQUALIFIED: Self = Self(1);
    pub const OPTIONAL: Self = Self(2);
    pub const TERSE: Self = Self(3);
}

impl From<type_system::PresenceQualifier> for PresenceQualifier {
    fn from(p: type_system::PresenceQualifier) -> Self {
        Self(p.0)
    }
}

/// A field within a struct or union definition.
#[derive(Clone, Debug)]
pub struct FieldDefinition {
    identity: FieldIdentity,
    presence: PresenceQualifier,
    type_ref: TypeRef,
    custom_default: Option<SerializableRecord>,
    annotations: AnnotationsMap,
}

impl FieldDefinition {
    pub(crate) fn new(
        identity: FieldIdentity,
        presence: PresenceQualifier,
        type_ref: TypeRef,
        custom_default: Option<SerializableRecord>,
        annotations: AnnotationsMap,
    ) -> Self {
        Self {
            identity,
            presence,
            type_ref,
            custom_default,
            annotations,
        }
    }

    pub fn identity(&self) -> &FieldIdentity {
        &self.identity
    }

    pub fn id(&self) -> i16 {
        self.identity.id
    }

    pub fn name(&self) -> &str {
        &self.identity.name
    }

    pub fn presence(&self) -> PresenceQualifier {
        self.presence
    }

    pub fn type_ref(&self) -> &TypeRef {
        &self.type_ref
    }

    pub fn custom_default(&self) -> Option<&SerializableRecord> {
        self.custom_default.as_ref()
    }

    pub fn annotations(&self) -> &AnnotationsMap {
        &self.annotations
    }

    pub fn annotation(&self, uri: &str) -> Option<&SerializableRecord> {
        self.annotations.get(uri)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn fast_field_handle_invalid_is_not_valid() {
        assert!(!FastFieldHandle::INVALID.is_valid());
    }

    #[test]
    #[should_panic(expected = "attempted to use invalid FastFieldHandle")]
    fn fast_field_handle_invalid_index_panics() {
        FastFieldHandle::INVALID.index();
    }

    #[test]
    fn fast_field_handle_roundtrip() {
        let h = FastFieldHandle::new(0);
        assert!(h.is_valid());
        assert_eq!(h.index(), 0);
        let h = FastFieldHandle::new(42);
        assert_eq!(h.index(), 42);
    }
}
