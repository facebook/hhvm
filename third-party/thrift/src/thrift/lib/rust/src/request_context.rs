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

use std::marker::PhantomData;

use anyhow::Error;

use crate::context_stack::ContextStack;
use crate::context_stack::DummyContextStack;

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
}

pub struct DummyRequestContext<Name: ?Sized, Buffer> {
    _phantom: PhantomData<(Buffer, Name)>,
}

impl<Name: ?Sized, Buffer> DummyRequestContext<Name, Buffer> {
    pub fn new() -> Self {
        Self {
            _phantom: PhantomData,
        }
    }
}

impl<Name: ?Sized, Buffer> RequestContext for DummyRequestContext<Name, Buffer> {
    type ContextStack = crate::context_stack::DummyContextStack<Name, Buffer>;
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

fn _assert_context_stack(_: &impl RequestContext) {}

#[test]
fn check_unsized() {
    _assert_context_stack(&DummyRequestContext::<std::ffi::CStr, Vec<u8>>::new());
}
