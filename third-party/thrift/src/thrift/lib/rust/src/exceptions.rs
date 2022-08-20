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

//! Helpers for generated code using exceptions

use crate::thrift_protocol::MessageType;

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
