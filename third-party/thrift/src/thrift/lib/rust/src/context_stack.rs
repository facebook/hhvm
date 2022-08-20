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

use crate::thrift_protocol::ProtocolID;

pub struct SerializedMessage<'a, Name: ?Sized, Buffer> {
    pub protocol: ProtocolID,
    pub buffer: PhantomData<Buffer>, // Buffer,
    pub method_name: &'a Name,
}

pub trait ContextStack {
    /// Type for method names
    type Name: ?Sized;
    /// Type for buffers
    type Buffer;

    /// Called before the request is read.
    fn pre_read(&mut self) -> Result<(), Error>;

    /// Called before post_read after reading arguments (server) / after reading
    /// reply (client), with the actual (unparsed, serialized) data.
    fn on_read_data(
        &mut self,
        msg: &SerializedMessage<Self::Name, Self::Buffer>,
    ) -> Result<(), Error>;

    /// Called after the request is read.
    fn post_read(&mut self, bytes: u32) -> Result<(), Error>;

    /// Called before a response is written.
    fn pre_write(&mut self) -> Result<(), Error>;

    /// Called before post_write, after serializing response (server) / after
    /// serializing request (client), with the actual (serialized) data.
    fn on_write_data(
        &mut self,
        msg: &SerializedMessage<Self::Name, Self::Buffer>,
    ) -> Result<(), Error>;

    /// Called after a response a written.
    fn post_write(&mut self, bytes: u32) -> Result<(), Error>;
}

pub struct DummyContextStack<Name: ?Sized, Buffer> {
    _phantom: PhantomData<(Buffer, Name)>,
}

impl<Name: ?Sized, Buffer> DummyContextStack<Name, Buffer> {
    pub fn new() -> Self {
        Self {
            _phantom: PhantomData,
        }
    }
}

impl<Name: ?Sized, Buffer> ContextStack for DummyContextStack<Name, Buffer> {
    type Name = Name;
    type Buffer = Buffer;

    fn pre_read(&mut self) -> Result<(), Error> {
        Ok(())
    }

    fn on_read_data(
        &mut self,
        _msg: &SerializedMessage<Self::Name, Self::Buffer>,
    ) -> Result<(), Error> {
        Ok(())
    }

    fn post_read(&mut self, _bytes: u32) -> Result<(), Error> {
        Ok(())
    }

    fn pre_write(&mut self) -> Result<(), Error> {
        Ok(())
    }

    fn on_write_data(
        &mut self,
        _msg: &SerializedMessage<Self::Name, Self::Buffer>,
    ) -> Result<(), Error> {
        Ok(())
    }

    fn post_write(&mut self, _bytes: u32) -> Result<(), Error> {
        Ok(())
    }
}

fn _assert_context_stack(_: &impl ContextStack) {}

#[test]
fn check_unsized() {
    _assert_context_stack(&DummyContextStack::<std::ffi::CStr, Vec<u8>>::new());
}
