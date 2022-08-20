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

use std::any::Any;

use crate::deserialize::Deserialize;
use crate::exceptions::ExceptionInfo;
use crate::exceptions::ResultInfo;
use crate::exceptions::ResultType;
use crate::protocol::Field;
use crate::protocol::ProtocolReader;
use crate::protocol::ProtocolWriter;
use crate::serialize::Serialize;
use crate::thrift_protocol::ProtocolID;
use crate::ttype::TType;
use crate::Result;

// Reference is thrift/lib/cpp/TApplicationException.h
#[derive(Debug, Copy, Clone, Eq, PartialEq, Ord, PartialOrd, Hash)]
#[repr(i32)]
pub enum ApplicationExceptionErrorCode {
    Unknown = 0,
    UnknownMethod = 1,
    InvalidMessageType = 2,
    WrongMethodName = 3,
    BadSequenceID = 4,
    MissingResult = 5,
    InternalError = 6,
    ProtocolError = 7,
    InvalidTransform = 8,
    InvalidProtocol = 9,
    UnsupportedClientType = 10,
    Loadshedding = 11,
    Timeout = 12,
    InjectedFailure = 13,
    ChecksumMismatch = 14,
    Interruption = 15,
}

impl Default for ApplicationExceptionErrorCode {
    fn default() -> Self {
        ApplicationExceptionErrorCode::Unknown
    }
}

const TAPPLICATION_EXCEPTION_ERROR_CODE: &str = "ApplicationExceptionErrorCode";

/// ApplicationException is *not* actually an error type in the Rust sense, but
/// a Thrift-specific serializable
#[derive(Debug, Clone, Eq, PartialEq, Ord, PartialOrd, Hash, Default)]
pub struct ApplicationException {
    pub message: String,
    pub type_: ApplicationExceptionErrorCode,
}

impl ApplicationException {
    #[cold]
    pub fn new(type_: ApplicationExceptionErrorCode, message: String) -> Self {
        ApplicationException { message, type_ }
    }

    // Used for methods which should exist but don't
    #[cold]
    pub fn unimplemented_method(handler: &str, method: &str) -> Self {
        ApplicationException {
            type_: ApplicationExceptionErrorCode::UnknownMethod,
            message: format!("Method {} not implemented for {}", method, handler),
        }
    }

    /// Indicator that this service doesn't have the method, but another might
    #[inline]
    pub fn unknown_method() -> Self {
        ApplicationException {
            type_: ApplicationExceptionErrorCode::UnknownMethod,
            message: String::new(), // no allocation
        }
    }

    #[cold]
    pub fn missing_arg(method: &str, arg: &str) -> Self {
        ApplicationException {
            type_: ApplicationExceptionErrorCode::ProtocolError,
            message: format!("{} missing arg {}", method, arg),
        }
    }

    #[cold]
    pub fn missing_field(method: &str, field: &str) -> Self {
        ApplicationException {
            type_: ApplicationExceptionErrorCode::ProtocolError,
            message: format!("Struct {} missing field {}", method, field),
        }
    }

    #[cold]
    pub fn invalid_protocol(badproto: ProtocolID) -> Self {
        ApplicationException {
            type_: ApplicationExceptionErrorCode::InvalidProtocol,
            message: format!("Invalid protocol {:?}", badproto),
        }
    }

    /// An undeclared "exception"/panic etc
    #[cold]
    pub fn handler_panic(method: &str, exn: Box<dyn Any + Send + 'static>) -> Self {
        ApplicationException {
            type_: ApplicationExceptionErrorCode::Unknown,
            message: format!(
                "Handler for `{}` panicked with: {}",
                method,
                panic_message::get_panic_message(&exn).unwrap_or("<panic>")
            ),
        }
    }
}

impl ExceptionInfo for ApplicationException {
    #[rustfmt::skip]
    fn exn_name(&self) -> &'static str {
        match self.type_ {
            ApplicationExceptionErrorCode::Unknown => "ApplicationExceptionErrorCode::Unknown",
            ApplicationExceptionErrorCode::UnknownMethod => "ApplicationExceptionErrorCode::UnknownMethod",
            ApplicationExceptionErrorCode::InvalidMessageType => "ApplicationExceptionErrorCode::InvalidMessageType",
            ApplicationExceptionErrorCode::WrongMethodName => "ApplicationExceptionErrorCode::WrongMethodName",
            ApplicationExceptionErrorCode::BadSequenceID => "ApplicationExceptionErrorCode::BadSequenceID",
            ApplicationExceptionErrorCode::MissingResult => "ApplicationExceptionErrorCode::MissingResult",
            ApplicationExceptionErrorCode::InternalError => "ApplicationExceptionErrorCode::InternalError",
            ApplicationExceptionErrorCode::ProtocolError => "ApplicationExceptionErrorCode::ProtocolError",
            ApplicationExceptionErrorCode::InvalidTransform => "ApplicationExceptionErrorCode::InvalidTransform",
            ApplicationExceptionErrorCode::InvalidProtocol => "ApplicationExceptionErrorCode::InvalidProtocol",
            ApplicationExceptionErrorCode::UnsupportedClientType => "ApplicationExceptionErrorCode::UnsupportedClientType",
            ApplicationExceptionErrorCode::Loadshedding => "ApplicationExceptionErrorCode::Loadshedding",
            ApplicationExceptionErrorCode::Timeout => "ApplicationExceptionErrorCode::Timeout",
            ApplicationExceptionErrorCode::InjectedFailure => "ApplicationExceptionErrorCode::InjectedFailure",
            ApplicationExceptionErrorCode::ChecksumMismatch => "ApplicationExceptionErrorCode::ChecksumMismatch",
            ApplicationExceptionErrorCode::Interruption => "ApplicationExceptionErrorCode::Interruption",
        }
    }

    fn exn_value(&self) -> String {
        self.message.clone()
    }

    #[inline]
    fn exn_is_declared(&self) -> bool {
        false
    }
}

impl ResultInfo for ApplicationException {
    fn result_type(&self) -> ResultType {
        ResultType::Exception
    }
}

impl<P> Deserialize<P> for ApplicationException
where
    P: ProtocolReader,
{
    /// Decodes a ApplicationException message from a Protocol stream
    fn read(iprot: &mut P) -> Result<Self> {
        iprot.read_struct_begin(|_| ())?;

        let mut message = String::from("");
        let mut type_ = ApplicationExceptionErrorCode::Unknown;

        loop {
            // Start reading the next field
            static FIELDS: &[Field] = &[
                Field::new("message", TType::String, 1),
                Field::new("type", TType::I32, 2),
            ];
            let (_, ttype, id) = iprot.read_field_begin(|_| (), FIELDS)?;

            match (ttype, id) {
                (TType::Stop, _) => break,
                (TType::String, 1) => message = iprot.read_string()?,
                (TType::I32, 2) => {
                    type_ = match iprot.read_i32()? {
                        1 => ApplicationExceptionErrorCode::UnknownMethod,
                        2 => ApplicationExceptionErrorCode::InvalidMessageType,
                        3 => ApplicationExceptionErrorCode::WrongMethodName,
                        4 => ApplicationExceptionErrorCode::BadSequenceID,
                        5 => ApplicationExceptionErrorCode::MissingResult,
                        6 => ApplicationExceptionErrorCode::InternalError,
                        7 => ApplicationExceptionErrorCode::ProtocolError,
                        8 => ApplicationExceptionErrorCode::InvalidTransform,
                        9 => ApplicationExceptionErrorCode::InvalidProtocol,
                        10 => ApplicationExceptionErrorCode::UnsupportedClientType,
                        11 => ApplicationExceptionErrorCode::Loadshedding,
                        12 => ApplicationExceptionErrorCode::Timeout,
                        13 => ApplicationExceptionErrorCode::InjectedFailure,
                        14 => ApplicationExceptionErrorCode::ChecksumMismatch,
                        15 => ApplicationExceptionErrorCode::Interruption,

                        _ => ApplicationExceptionErrorCode::Unknown,
                    }
                }
                (ttype, _) => iprot.skip(ttype)?,
            };

            // Finished reading the end of the field
            iprot.read_field_end()?;
        }
        iprot.read_struct_end()?;
        Ok(ApplicationException::new(type_, message))
    }
}

impl<P> Serialize<P> for ApplicationException
where
    P: ProtocolWriter,
{
    /// Writes an application exception to the Protocol stream
    fn write(&self, oprot: &mut P) {
        oprot.write_struct_begin(TAPPLICATION_EXCEPTION_ERROR_CODE);

        if !self.message.is_empty() {
            oprot.write_field_begin("message", TType::String, 1);
            oprot.write_string(&self.message);
            oprot.write_field_end();
        }
        oprot.write_field_begin("type", TType::I32, 2);
        oprot.write_i32(self.type_ as i32);
        oprot.write_field_end();
        oprot.write_field_stop();
        oprot.write_struct_end();
    }
}
