/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

use num_derive::FromPrimitive;
use num_traits::FromPrimitive;

use crate::errors::ProtocolError;
use crate::Result;

/// Protocol kind. int16
#[repr(i16)]
#[derive(Debug, Copy, Clone, Eq, PartialEq, Hash, FromPrimitive)]
pub enum ProtocolID {
    BinaryProtocol = 0,
    JSONProtocol = 1,
    CompactProtocol = 2,
    DebugProtocol = 3,
    VirtualProtocol = 4,
    SimpleJSONProtocol = 5,
}

impl TryFrom<i16> for ProtocolID {
    type Error = anyhow::Error;

    fn try_from(val: i16) -> Result<Self> {
        match ProtocolID::from_i16(val) {
            Some(id) => Ok(id),
            None => bail_err!(ProtocolError::InvalidProtocolID(val)),
        }
    }
}

impl From<ProtocolID> for &'static str {
    fn from(t: ProtocolID) -> &'static str {
        match t {
            ProtocolID::BinaryProtocol => "binary",
            ProtocolID::JSONProtocol => "json",
            ProtocolID::CompactProtocol => "compact",
            ProtocolID::DebugProtocol => "debug",
            ProtocolID::VirtualProtocol => "virtual",
            ProtocolID::SimpleJSONProtocol => "simplejson",
        }
    }
}

/// Message type constants in the Thrift protocol, treated as i32 in golang
#[derive(Debug, Copy, Clone, Eq, PartialEq, FromPrimitive)]
#[repr(u32)]
pub enum MessageType {
    InvalidMessageType = 0,
    Call = 1,
    Reply = 2,
    Exception = 3,
    Oneway = 4, // Unused
}

impl TryFrom<u32> for MessageType {
    type Error = anyhow::Error;

    fn try_from(val: u32) -> Result<Self> {
        match MessageType::from_u32(val) {
            Some(t) => Ok(t),
            None => bail_err!(ProtocolError::InvalidMessageType(val)),
        }
    }
}
