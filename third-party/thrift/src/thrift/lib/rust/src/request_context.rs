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

use std::marker::PhantomData;

use anyhow::Error;

use crate::bufext::BufExt;
use crate::context_stack::ContextStack;
use crate::context_stack::DummyContextStack;
use crate::framing::Framing;
use crate::framing::FramingEncodedFinal;

pub trait RequestContext {
    type ContextStack: ContextStack;
    type Name: ?Sized;

    // This should return a ContextStack with a lifetime bounded by &self and the
    // names, but that needs GATs, so require 'static for now.
    fn get_context_stack(
        &self,
        service_name: &'static Self::Name,
        fn_name: &'static Self::Name,
    ) -> Result<Self::ContextStack, Error>;

    fn set_user_exception_header(&self, ex_type: &str, ex_reason: &str) -> Result<(), Error>;

    /// Attach error classification (blame/kind/safety) for a declared exception
    /// so it can be propagated to the caller on the wire. Defaults to a no-op;
    /// server request contexts that support it (e.g. srserver) override this.
    fn set_error_classification(
        &self,
        _blame: crate::exceptions::ExceptionBlame,
        _kind: crate::exceptions::ExceptionKind,
        _safety: crate::exceptions::ExceptionSafety,
    ) -> Result<(), Error> {
        Ok(())
    }
}

pub struct DummyRequestContext<Name: ?Sized, Frame> {
    _phantom: PhantomData<(Frame, Name)>,
}

impl<Name: ?Sized, Frame> DummyRequestContext<Name, Frame> {
    pub fn new() -> Self {
        Self {
            _phantom: PhantomData,
        }
    }
}

impl<Name: ?Sized, Frame: Framing> RequestContext for DummyRequestContext<Name, Frame>
where
    FramingEncodedFinal<Frame>: BufExt,
{
    type ContextStack = crate::context_stack::DummyContextStack<Name, Frame>;
    type Name = Name;

    fn get_context_stack(
        &self,
        _service_name: &Self::Name,
        _fn_name: &Self::Name,
    ) -> Result<Self::ContextStack, Error> {
        Ok(DummyContextStack::new())
    }

    fn set_user_exception_header(&self, _ex_type: &str, _ex_reason: &str) -> Result<(), Error> {
        Ok(())
    }
}

#[cfg(test)]
mod test {
    use bytes::Bytes;

    use super::*;

    fn assert_context_stack(_: &impl RequestContext) {}

    #[test]
    fn check_unsized() {
        assert_context_stack(&DummyRequestContext::<std::ffi::CStr, Bytes>::new());
    }
}
