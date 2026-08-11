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

use std::future::Future;
use std::marker::PhantomData;

use crate::BytesPtr;
use crate::CallbackContext;
use crate::ContextHandle;
use crate::ErasedCheck;
use crate::HandlerResult;
use crate::PipelineError;
use crate::RustMessageAdapter;
use crate::RustTypeErasedBox;

/// A message that can resume an inbound captured pipeline continuation.
pub trait ContextReadMessage: Sized {
    /// Continue an inbound message from the current callback position.
    fn fire_read_inline(context: &mut CallbackContext<'_>, message: Self) -> HandlerResult;

    /// Continue an inbound message from the captured pipeline position.
    fn fire_read(handle: ContextHandle, message: Self);
}

impl ContextReadMessage for BytesPtr {
    fn fire_read_inline(context: &mut CallbackContext<'_>, message: Self) -> HandlerResult {
        context.fire_read(message)
    }

    fn fire_read(handle: ContextHandle, message: Self) {
        handle.fire_read(message);
    }
}

/// A message that can resume an outbound captured pipeline continuation.
pub trait ContextWriteMessage: Sized {
    /// Continue an outbound message from the current callback position.
    fn fire_write_inline(context: &mut CallbackContext<'_>, message: Self) -> HandlerResult;

    /// Continue an outbound message from the captured pipeline position.
    fn fire_write(handle: ContextHandle, message: Self);
}

impl ContextWriteMessage for BytesPtr {
    fn fire_write_inline(context: &mut CallbackContext<'_>, message: Self) -> HandlerResult {
        context.fire_write(message)
    }

    fn fire_write(handle: ContextHandle, message: Self) {
        handle.fire_write(message);
    }
}

/// Rust-only adapter from a type-erased pipeline read to a Rust coroutine.
pub struct CoroReadHandle<M, F> {
    method: F,
    _message: PhantomData<fn(M)>,
}

impl<M, F> CoroReadHandle<M, F> {
    pub fn new(method: F) -> Self {
        Self {
            method,
            _message: PhantomData,
        }
    }
}

impl<M, F, Fut> CoroReadHandle<M, F>
where
    M: ContextReadMessage + RustMessageAdapter + ErasedCheck + Send + 'static,
    F: FnMut(M) -> Fut,
    Fut: Future<Output = M> + Send + 'static,
{
    /// Take `M` from the borrowed type-erased box and run the returned future
    /// on this callback's pipeline EventBase.
    ///
    /// The first poll runs inline. If it returns `Pending`, later wakes schedule
    /// another poll on the same EventBase. Completion consumes the task-owned
    /// [`ContextHandle`] and resumes the read from its captured pipeline
    /// position.
    pub fn fire_read(
        &mut self,
        context: &mut CallbackContext<'_>,
        mut message: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let future = (self.method)(message.take::<M>());
        context.spawn_deferred(future, M::fire_read_inline, M::fire_read)
    }
}

/// Rust-only adapter from a type-erased pipeline write to a Rust coroutine.
pub struct CoroWriteHandle<M, F> {
    method: F,
    _message: PhantomData<fn(M)>,
}

impl<M, F> CoroWriteHandle<M, F> {
    pub fn new(method: F) -> Self {
        Self {
            method,
            _message: PhantomData,
        }
    }
}

impl<M, F, Fut> CoroWriteHandle<M, F>
where
    M: ContextWriteMessage + RustMessageAdapter + ErasedCheck + Send + 'static,
    F: FnMut(M) -> Fut,
    Fut: Future<Output = M> + Send + 'static,
{
    /// Take `M` from the borrowed type-erased box and run the returned future
    /// on this callback's pipeline EventBase.
    ///
    /// The first poll runs inline. If it returns `Pending`, later wakes schedule
    /// another poll on the same EventBase. Completion consumes the task-owned
    /// [`ContextHandle`] and resumes the write from its captured pipeline
    /// position.
    pub fn fire_write(
        &mut self,
        context: &mut CallbackContext<'_>,
        mut message: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let future = (self.method)(message.take::<M>());
        context.spawn_deferred(future, M::fire_write_inline, M::fire_write)
    }
}

/// Rust-only adapter from an owned pipeline error to a Rust coroutine.
pub struct CoroExceptionHandle<F> {
    method: F,
}

impl<F> CoroExceptionHandle<F> {
    pub fn new(method: F) -> Self {
        Self { method }
    }
}

impl<F, Fut> CoroExceptionHandle<F>
where
    F: FnMut(PipelineError) -> Fut,
    Fut: Future<Output = PipelineError> + Send + 'static,
{
    /// Run the supplied error coroutine on this callback's pipeline EventBase.
    ///
    /// The first poll runs inline. If it returns `Pending`, later wakes schedule
    /// another poll on the same EventBase. Completion consumes the task-owned
    /// [`ContextHandle`] and resumes exception delivery from its captured
    /// pipeline position.
    pub fn fire_exception(
        &mut self,
        context: &mut CallbackContext<'_>,
        error: PipelineError,
    ) -> HandlerResult {
        let future = (self.method)(error);
        context.spawn(future, ContextHandle::fire_exception);
        HandlerResult::Success
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn bytes_ptr_can_resume_read_and_write_continuations() {
        fn assert_context_read_message<T: ContextReadMessage>() {}
        fn assert_context_write_message<T: ContextWriteMessage>() {}
        assert_context_read_message::<BytesPtr>();
        assert_context_write_message::<BytesPtr>();
    }

    #[test]
    fn constructors_accept_coroutine_methods() {
        let _read = CoroReadHandle::<BytesPtr, _>::new(|message: BytesPtr| async move { message });
        let _write =
            CoroWriteHandle::<BytesPtr, _>::new(|message: BytesPtr| async move { message });
        let _exception = CoroExceptionHandle::new(|error: PipelineError| async move { error });
    }
}
