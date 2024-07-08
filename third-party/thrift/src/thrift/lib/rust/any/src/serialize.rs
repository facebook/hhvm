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

use fbthrift::binary_protocol;
use fbthrift::compact_protocol;
use fbthrift::simplejson_protocol;
use standard::StandardProtocol;
use type_rep::ProtocolUnion;

use crate::GetThriftAnyType;

// For convenience
pub trait SerializableToAny:
    GetThriftAnyType
    + fbthrift::GetUri
    + fbthrift::GetTypeNameType
    + fbthrift::GetTType
    + compact_protocol::SerializeRef
    + simplejson_protocol::SerializeRef
{
}
impl<T> SerializableToAny for T where
    T: GetThriftAnyType
        + fbthrift::GetUri
        + fbthrift::GetTypeNameType
        + fbthrift::GetTType
        + compact_protocol::SerializeRef
        + simplejson_protocol::SerializeRef
{
}

pub fn serialize<T: SerializableToAny>(object: &T) -> any::Any {
    any::Any {
        r#type: <T as GetThriftAnyType>::get_thrift_any_type(),
        protocol: ProtocolUnion::standard(StandardProtocol::Compact),
        data: compact_protocol::serialize(object).to_vec(),
        ..Default::default()
    }
}

pub fn serialize_json<T: SerializableToAny>(object: &T) -> any::Any {
    any::Any {
        r#type: <T as GetThriftAnyType>::get_thrift_any_type(),
        protocol: ProtocolUnion::standard(StandardProtocol::SimpleJson),
        data: simplejson_protocol::serialize(object).to_vec(),
        ..Default::default()
    }
}

// 'any_wrapper'
pub trait SerializableThriftObject:
    'static
    + GetThriftAnyType
    + fbthrift::GetTType
    + fbthrift::GetUri
    + fbthrift::GetTypeNameType
    + binary_protocol::DeserializeSlice
    + compact_protocol::DeserializeSlice
    + simplejson_protocol::DeserializeSlice
    + binary_protocol::SerializeRef
    + compact_protocol::SerializeRef
    + simplejson_protocol::SerializeRef
{
}

impl<T> SerializableThriftObject for T where
    T: 'static
        + GetThriftAnyType
        + fbthrift::GetTType
        + fbthrift::GetUri
        + fbthrift::GetTypeNameType
        + binary_protocol::DeserializeSlice
        + compact_protocol::DeserializeSlice
        + simplejson_protocol::DeserializeSlice
        + binary_protocol::SerializeRef
        + compact_protocol::SerializeRef
        + simplejson_protocol::SerializeRef
{
}
