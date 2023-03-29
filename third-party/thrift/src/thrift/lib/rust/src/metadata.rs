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

pub trait ThriftAnnotations: 'static {
    /// Returns the structured annotation `T` that is defined on this Thrift type, if the
    /// annotation exists.
    fn get_structured_annotation<T: Sized + 'static>() -> Option<T> {
        None
    }

    /// Returns the structured annotation `T` that is defined on the given field with id `field_id`
    /// on this Thrift type.
    ///
    /// Returns `None` if the field ID does not exist or if the given `T` does not exist as an
    /// annotation on the given field.
    ///
    /// For example, for the given Thrift file:
    /// ```thrift
    /// struct FieldAnnotation {
    ///     1: string payload;
    /// }
    ///
    /// struct Foo {
    ///     @FieldAnnotation{ payload = "hello world" }
    ///     1: string username;
    /// }
    /// ```
    ///
    /// The call
    /// ```ignore
    /// <Foo as ThriftAnnotations>::get_field_structured_annotation::<FieldAnnotation>(1)
    /// ```
    /// will return
    /// ```ignore
    /// Some(FieldAnnotation {
    ///     payload: "hello world".to_string(),
    /// })
    /// ```
    fn get_field_structured_annotation<T: Sized + 'static>(_field_id: i16) -> Option<T> {
        None
    }
}
