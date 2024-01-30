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

#![recursion_limit = "1024"]
#![deny(warnings)]

macro_rules! bail_err {
    ($e:expr) => {
        return Err(From::from($e))
    };
}

macro_rules! ensure_err {
    ($cond:expr, $e:expr) => {
        if !$cond {
            bail_err!($e);
        }
    };
}

use anyhow::Result;

#[macro_use]
pub mod protocol;

pub mod adapter;
pub mod application_exception;
pub mod binary_protocol;
pub mod binary_type;
pub mod builtin_types;
pub mod compact_protocol;
pub mod context_stack;
pub mod deserialize;
pub mod export;
pub mod framing;
pub mod metadata;
pub mod processor;
pub mod request_context;
pub mod serialize;
pub mod simplejson_protocol;
pub mod thrift_protocol;
pub mod ttype;
pub mod uri;

mod bufext;
mod client;
mod errors;
mod exceptions;
mod varint;

#[cfg(test)]
mod tests;

#[doc(hidden)]
pub mod help;

pub use crate::application_exception::ApplicationException;
pub use crate::application_exception::ApplicationExceptionErrorCode;
pub use crate::binary_protocol::BinaryProtocol;
pub use crate::bufext::BufExt;
pub use crate::bufext::BufMutExt;
pub use crate::bufext::DeserializeSource;
pub use crate::client::ClientFactory;
pub use crate::client::ClientStreamElement;
pub use crate::client::Transport;
pub use crate::compact_protocol::CompactProtocol;
pub use crate::context_stack::ContextStack;
pub use crate::context_stack::DummyContextStack;
pub use crate::context_stack::SerializedMessage;
pub use crate::deserialize::Deserialize;
pub use crate::errors::NonthrowingFunctionError;
pub use crate::errors::ProtocolError;
pub use crate::exceptions::ExceptionInfo;
pub use crate::exceptions::ResultInfo;
pub use crate::exceptions::ResultType;
pub use crate::framing::Framing;
pub use crate::framing::FramingDecoded;
pub use crate::framing::FramingEncoded;
pub use crate::framing::FramingEncodedFinal;
pub use crate::help::NoopSpawner;
pub use crate::processor::NullServiceProcessor;
pub use crate::processor::ReplyState;
pub use crate::processor::SerializedStreamElement;
pub use crate::processor::ServiceProcessor;
pub use crate::processor::ThriftService;
pub use crate::protocol::Field;
pub use crate::protocol::Protocol;
pub use crate::protocol::ProtocolDecoded;
pub use crate::protocol::ProtocolEncoded;
pub use crate::protocol::ProtocolEncodedFinal;
pub use crate::protocol::ProtocolReader;
pub use crate::protocol::ProtocolWriter;
pub use crate::request_context::DummyRequestContext;
pub use crate::request_context::RequestContext;
pub use crate::serialize::Serialize;
pub use crate::simplejson_protocol::SimpleJsonProtocol;
pub use crate::thrift_protocol::MessageType;
pub use crate::thrift_protocol::ProtocolID;
pub use crate::ttype::GetTType;
pub use crate::ttype::TType;
pub use crate::uri::GetUri;

pub trait ThriftEnum: Sized {
    fn enumerate() -> &'static [(Self, &'static str)];

    fn variants() -> &'static [&'static str];

    fn variant_values() -> &'static [Self];
}
