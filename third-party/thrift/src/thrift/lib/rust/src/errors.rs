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

use anyhow::Error;
use thiserror::Error;

use crate::ApplicationException;
use crate::ttype::TType;

#[derive(Debug, Error, PartialEq)]
#[allow(dead_code)]
pub enum ProtocolError {
    #[error("end of file reached")]
    EOF,
    #[error("bad thrift version specified")]
    BadVersion,
    #[error("missing protocol version")]
    ProtocolVersionMissing,
    #[error("protocol skip depth exceeded")]
    SkipDepthExceeded,
    #[error("streams unsupported")]
    StreamUnsupported,
    #[error("STOP outside of struct in skip")]
    UnexpectedStopInSkip,
    #[error("Invalid type in skip {0:?}")]
    InvalidTypeInSkip(TType),
    #[error("Unknown or invalid protocol ID {0}")]
    InvalidProtocolID(i16),
    #[error("Unknown or invalid TMessage type {0}")]
    InvalidMessageType(u32),
    #[error("Unknown or invalid type tag")]
    InvalidTypeTag,
    #[error("Unknown or invalid data length")]
    InvalidDataLength,
    #[error("Invalid value for type")]
    InvalidValue,
    #[error("Unexpected trailing data after the end of a value")]
    TrailingData,
    #[error("Unwanted extra union {0} field ty {1:?} id {2}")]
    UnwantedExtraUnionField(String, TType, i32),
    #[error("Deserializing collection containing Void element is not supported")]
    VoidCollectionElement,
}

/// Error value returned by functions that do not throw any user-defined exceptions.
#[derive(Debug, Error)]
pub enum NonthrowingFunctionError {
    #[error(transparent)]
    ApplicationException(#[from] ApplicationException),
    #[error(transparent)]
    ThriftError(#[from] Error),
}

#[derive(Debug, Error)]
#[error("Error while deserializing {arg} arg of {function}")]
pub struct DeserializingArgError {
    pub arg: &'static str,
    pub function: &'static str,
}

#[derive(Debug, Error)]
#[error("Error while deserializing {field} field of {strct}")]
pub struct DeserializingFieldError {
    pub field: &'static str,
    pub strct: &'static str,
}
