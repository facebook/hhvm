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

use std::any::TypeId;
use std::error::Error;
use std::fmt::Debug;

pub trait ThriftTypeAdapter {
    type ThriftType;
    type AdaptedType: Clone + Debug + PartialEq + Send + Sync;

    /// The error type thrown if `from_thrift` fails during deserialization.
    type Error: Error;

    /// Converts from a basic Thrift type to the specified `AdaptedType` during deserialization.
    ///
    /// The `Err` returned by this method will be propagated as a deserialization error.
    fn from_thrift(value: Self::ThriftType) -> Result<Self::AdaptedType, Self::Error>;

    /// Converts from the specified `AdaptedType` back to the native Thrift type during
    /// serialization.
    ///
    /// This must be an infallible operation as `serialize` is infallible.
    ///
    /// WARNING: you should be particularly cautious when using `.unwrap()` or any other panic-able
    /// methods in this method. If this method panics, it will be at serialization time and not at
    /// the Thrift struct creation time, meaning it will be extremely difficult to debug what the
    /// true "source" of the panic.
    ///
    /// If your `AdaptedType` -> `ThriftType` conversion is truly fallible, you probably shouldn't
    /// use an adapter to begin with.
    fn to_thrift(value: &Self::AdaptedType) -> Self::ThriftType;

    /// Method called when the `ThriftTypeAdapter` is used on a Thrift struct's field. Provides
    /// information about the specific field ID and the type ID of the parent struct this field is
    /// in.
    ///
    /// Defaults to calling `from_thrift`.
    fn from_thrift_field(
        value: Self::ThriftType,
        _field_id: i16,
        _strct: TypeId,
    ) -> Result<Self::AdaptedType, Self::Error> {
        Self::from_thrift(value)
    }

    /// Method called when the `ThriftTypeAdapter` is used on a Thrift struct's field. Provides
    /// information about the specific field ID and the type ID of the parent struct this field is
    /// in.
    ///
    /// Defaults to calling `to_thrift`.
    fn to_thrift_field(
        value: &Self::AdaptedType,
        _field_id: i16,
        _strct: TypeId,
    ) -> Self::ThriftType {
        Self::to_thrift(value)
    }

    // TODO(emersonford): generate unit tests for thrift libraries using Rust adapter type to
    // ensure this does not panic with a default value.
    /// Method called when the adapted type field is not present during deserialization or is
    /// populated with `..Default::default()`. The value passed here is the default Thrift type
    /// value for the field. This can be used to record that the field was not present inside
    /// of your adapted type.
    ///
    /// **This method must be infallible, as it will be called when `Default::default()` is used
    /// (which is infallible).**
    ///
    /// WARNING: This defaults to calling `from_thrift_field` and **assumes `from_thrift_field`
    /// will not return an `Err` for the default value**.
    fn from_thrift_default(
        value: Self::ThriftType,
        field_id: i16,
        strct: TypeId,
    ) -> Self::AdaptedType {
        Self::from_thrift_field(value, field_id, strct).expect(
            "`from_thrift_field` should not return an `Err` with the Thrift field's default value",
        )
    }
}
