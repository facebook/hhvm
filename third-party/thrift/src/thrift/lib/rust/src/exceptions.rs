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

//! Helpers for generated code using exceptions

use crate::thrift_protocol::MessageType;

/// Fault attribution for an exception. Mirrors the IDL exception `client`/`server`
/// qualifier and the wire `ErrorBlame` (RpcMetadata.thrift). Discriminants match
/// the wire enum so the value can cross the server FFI as an `i32`.
#[repr(i32)]
#[derive(Debug, Clone, Copy, Eq, PartialEq)]
pub enum ExceptionBlame {
    Unspecified = 0,
    Server = 1,
    Client = 2,
}

/// Retry semantics for an exception. Mirrors the IDL `transient`/`stateful`/
/// `permanent` qualifier and the wire `ErrorKind`.
#[repr(i32)]
#[derive(Debug, Clone, Copy, Eq, PartialEq)]
pub enum ExceptionKind {
    Unspecified = 0,
    Transient = 1,
    Stateful = 2,
    Permanent = 3,
}

/// Side-effect safety of the RPC that produced the exception. Mirrors the IDL
/// `safe` qualifier and the wire `ErrorSafety`.
#[repr(i32)]
#[derive(Debug, Clone, Copy, Eq, PartialEq)]
pub enum ExceptionSafety {
    Unspecified = 0,
    Safe = 1,
}

/// This trait should be implemented for each individual exception type. It will typically be generated.
pub trait ExceptionInfo {
    /// Exception name
    fn exn_name(&self) -> &'static str {
        std::any::type_name::<Self>()
    }

    // Exception value
    fn exn_value(&self) -> String;

    /// Is a declared exception
    fn exn_is_declared(&self) -> bool;

    /// Fault attribution (from the IDL `client`/`server` exception qualifier).
    /// Defaults to `Unspecified`; generated code overrides it for annotated
    /// exceptions. The server propagates this on the wire as `ErrorBlame` so the
    /// caller (e.g. ServiceRouter) can distinguish client-caused from
    /// server-caused declared exceptions.
    fn exn_blame(&self) -> ExceptionBlame {
        ExceptionBlame::Unspecified
    }

    /// Retry semantics (from the IDL `transient`/`stateful`/`permanent`
    /// qualifier). Defaults to `Unspecified`.
    fn exn_kind(&self) -> ExceptionKind {
        ExceptionKind::Unspecified
    }

    /// Side-effect safety (from the IDL `safe` qualifier). Defaults to
    /// `Unspecified`.
    fn exn_safety(&self) -> ExceptionSafety {
        ExceptionSafety::Unspecified
    }
}

/// An extension of ExceptionInfo that also includes successful results.
/// This is implemented on generated *Exn types.
pub trait ResultInfo: ExceptionInfo {
    fn result_type(&self) -> ResultType;
}

/// Classify a result from a specific method call.
#[derive(Debug, Clone, Copy, Eq, PartialEq)]
pub enum ResultType {
    /// A successful return
    Return,
    /// A declared exception
    Error,
    /// Some other exception (eg ApplicationException)
    Exception,
}

impl ResultType {
    pub fn message_type(&self) -> MessageType {
        match self {
            ResultType::Return | ResultType::Error => MessageType::Reply,
            ResultType::Exception => MessageType::Exception,
        }
    }
}
