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

/// Errors produced when validating or querying type definitions.
#[derive(Debug, thiserror::Error)]
pub enum InvalidTypeError {
    #[error("Unknown URI: {0}")]
    UnknownUri(String),
    #[error("Duplicate field ID {0} in {1}")]
    DuplicateFieldId(i16, String),
    #[error("Duplicate field name '{0}' in {1}")]
    DuplicateFieldName(String, String),
    #[error("Non-optional field {0} in union {1}")]
    NonOptionalUnionField(i16, String),
    #[error("Duplicate enum value {0} in {1}")]
    DuplicateEnumValue(i32, String),
    #[error("Duplicate enum name '{0}' in {1}")]
    DuplicateEnumName(String, String),
    #[error("Accessed {expected} but TypeRef is {actual}")]
    WrongKind {
        expected: &'static str,
        actual: &'static str,
    },
    #[error("Duplicate URI: {0}")]
    DuplicateUri(String),
    #[error("Opaque alias target must not be user-defined: {0}")]
    InvalidOpaqueAlias(String),
    #[error("Unresolvable TypeId in {uri}: {detail}")]
    UnresolvableTypeId { uri: String, detail: String },
    #[error("Empty TypeId")]
    EmptyTypeId,
    #[error("Too many fields ({0}) in {1}, maximum is 65535")]
    TooManyFields(usize, String),
    #[error("Duplicate source identifier '{0}' at location '{1}'")]
    DuplicateSourceIdentifier(String, String),
}
