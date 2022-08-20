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

use std::ffi::CStr;
use std::marker::PhantomData;
use std::sync::Arc;
use std::sync::Mutex;

use anyhow::bail;
use anyhow::Error;
use anyhow::Result;
use async_trait::async_trait;
use futures::stream::BoxStream;

use crate::application_exception::ApplicationException;
use crate::application_exception::ApplicationExceptionErrorCode;
use crate::context_stack::ContextStack;
use crate::exceptions::ExceptionInfo;
use crate::exceptions::ResultInfo;
use crate::framing::Framing;
use crate::framing::FramingDecoded;
use crate::framing::FramingEncodedFinal;
use crate::protocol::Protocol;
use crate::protocol::ProtocolDecoded;
use crate::protocol::ProtocolReader;
use crate::protocol::ProtocolWriter;
use crate::request_context::RequestContext;
use crate::serialize::Serialize;
use crate::thrift_protocol::ProtocolID;
use crate::ttype::TType;

pub enum SerializedStreamElement<Payload> {
    /// A normal stream response, without any error. Contains the serialized response.
    Success(Payload),
    /// Contains the serialized declared exception.
    DeclaredException(Payload),
    /// Contains the application exception.
    ApplicationException(ApplicationException),
    /// The serialization failed. Contains the error.
    SerializationError(Error),
}

pub trait ReplyState<F>
where
    F: Framing,
{
    fn send_reply(&mut self, reply: FramingEncodedFinal<F>);
    fn send_stream_reply(
        &mut self,
        response: FramingEncodedFinal<F>,
        stream: Option<BoxStream<'static, SerializedStreamElement<FramingEncodedFinal<F>>>>,
        protocol_id: ProtocolID,
    ) -> Result<()>;
}

#[async_trait]
pub trait ThriftService<F>: Send + Sync + 'static
where
    F: Framing + Send + 'static,
{
    type Handler;
    type RequestContext;
    type ReplyState;

    async fn call(
        &self,
        req: FramingDecoded<F>,
        req_ctxt: &Self::RequestContext,
        reply_state: Arc<Mutex<Self::ReplyState>>,
    ) -> Result<(), Error>;

    fn create_interaction(
        &self,
        _name: &str,
    ) -> ::anyhow::Result<
        Arc<
            dyn ThriftService<
                    F,
                    Handler = (),
                    RequestContext = Self::RequestContext,
                    ReplyState = Self::ReplyState,
                > + ::std::marker::Send
                + 'static,
        >,
    > {
        bail!("Thrift server does not support interactions");
    }
}

#[async_trait]
impl<F, T> ThriftService<F> for Box<T>
where
    T: ThriftService<F> + Send + Sync + ?Sized,
    F: Framing + Send + 'static,
    T::RequestContext: Send + Sync + 'static,
    T::ReplyState: Send + Sync + 'static,
{
    type Handler = T::Handler;
    type RequestContext = T::RequestContext;
    type ReplyState = T::ReplyState;

    async fn call(
        &self,
        req: FramingDecoded<F>,
        req_ctxt: &Self::RequestContext,
        reply_state: Arc<Mutex<Self::ReplyState>>,
    ) -> Result<(), Error> {
        (**self).call(req, req_ctxt, reply_state).await
    }

    fn create_interaction(
        &self,
        name: &str,
    ) -> ::anyhow::Result<
        Arc<
            dyn ThriftService<
                    F,
                    Handler = (),
                    RequestContext = Self::RequestContext,
                    ReplyState = Self::ReplyState,
                > + ::std::marker::Send
                + 'static,
        >,
    > {
        (**self).create_interaction(name)
    }
}

#[async_trait]
impl<F, T> ThriftService<F> for Arc<T>
where
    T: ThriftService<F> + Send + Sync + ?Sized,
    F: Framing + Send + 'static,
    T::RequestContext: Send + Sync + 'static,
    T::ReplyState: Send + Sync + 'static,
{
    type Handler = T::Handler;
    type RequestContext = T::RequestContext;
    type ReplyState = T::ReplyState;

    async fn call(
        &self,
        req: FramingDecoded<F>,
        req_ctxt: &Self::RequestContext,
        reply_state: Arc<Mutex<Self::ReplyState>>,
    ) -> Result<(), Error> {
        (**self).call(req, req_ctxt, reply_state).await
    }

    fn create_interaction(
        &self,
        name: &str,
    ) -> ::anyhow::Result<
        Arc<
            dyn ThriftService<
                    F,
                    Handler = (),
                    RequestContext = Self::RequestContext,
                    ReplyState = Self::ReplyState,
                > + ::std::marker::Send
                + 'static,
        >,
    > {
        (**self).create_interaction(name)
    }
}

/// Trait implemented by a generated type to implement a service.
#[async_trait]
pub trait ServiceProcessor<P>
where
    P: Protocol,
{
    type RequestContext;
    type ReplyState;

    /// Given a method name, return a reference to the processor for that index.
    fn method_idx(&self, name: &[u8]) -> Result<usize, ApplicationException>;

    /// Given a method index and the remains of the message input, get a future
    /// for the result of the method. This will only be called if the corresponding
    /// `method_idx()` returns an (index, ServiceProcessor) tuple.
    /// `frame` is a reference to the frame containing the request.
    /// `request` is a deserializer instance set up to decode the request.
    async fn handle_method(
        &self,
        idx: usize,
        //frame: &P::Frame,
        request: &mut P::Deserializer,
        req_ctxt: &Self::RequestContext,
        reply_state: Arc<Mutex<Self::ReplyState>>,
        seqid: u32,
    ) -> Result<(), Error>;

    /// Given a method name, return a reference to the interaction creation fn for that index
    fn create_interaction_idx(&self, _name: &str) -> ::anyhow::Result<::std::primitive::usize> {
        bail!("Processor does not support interactions");
    }

    /// Given a creation method index, it produces a fresh interaction processor
    fn handle_create_interaction(
        &self,
        _idx: ::std::primitive::usize,
    ) -> ::anyhow::Result<
        Arc<
            dyn ThriftService<
                    P::Frame,
                    Handler = (),
                    RequestContext = Self::RequestContext,
                    ReplyState = Self::ReplyState,
                > + ::std::marker::Send
                + 'static,
        >,
    > {
        bail!("Processor does not support interactions");
    }
}

/// Null processor which implements no methods - it acts as the super for any service
/// which has no super-service.
#[derive(Debug, Clone)]
pub struct NullServiceProcessor<P, R, RS> {
    _phantom: PhantomData<(P, R, RS)>,
}

impl<P, R, RS> NullServiceProcessor<P, R, RS> {
    pub fn new() -> Self {
        Self {
            _phantom: PhantomData,
        }
    }
}

impl<P, R, RS> Default for NullServiceProcessor<P, R, RS> {
    fn default() -> Self {
        Self::new()
    }
}

#[async_trait]
impl<P, R, RS> ServiceProcessor<P> for NullServiceProcessor<P, R, RS>
where
    P: Protocol + Sync,
    P::Deserializer: Send,
    R: Sync,
    RS: Sync + Send,
{
    type RequestContext = R;
    type ReplyState = RS;

    #[inline]
    fn method_idx(&self, name: &[u8]) -> Result<usize, ApplicationException> {
        Err(ApplicationException::new(
            ApplicationExceptionErrorCode::UnknownMethod,
            format!("Unknown method {}", String::from_utf8_lossy(name)),
        ))
    }

    async fn handle_method(
        &self,
        _idx: usize,
        //_frame: &P::Frame,
        _d: &mut P::Deserializer,
        _req_ctxt: &R,
        _reply_state: Arc<Mutex<RS>>,
        _seqid: u32,
    ) -> Result<(), Error> {
        // Should never be called since method_idx() always returns an error
        unimplemented!("NullServiceProcessor implements no methods")
    }

    fn create_interaction_idx(&self, name: &str) -> ::anyhow::Result<::std::primitive::usize> {
        bail!("Unknown interaction {}", name);
    }

    fn handle_create_interaction(
        &self,
        _idx: ::std::primitive::usize,
    ) -> ::anyhow::Result<
        Arc<
            dyn ThriftService<
                    P::Frame,
                    Handler = (),
                    RequestContext = Self::RequestContext,
                    ReplyState = Self::ReplyState,
                > + ::std::marker::Send
                + 'static,
        >,
    > {
        unimplemented!("NullServiceProcessor implements no interactions")
    }
}

#[async_trait]
impl<P, R, RS> ThriftService<P::Frame> for NullServiceProcessor<P, R, RS>
where
    P: Protocol + Send + Sync + 'static,
    P::Frame: Send + 'static,
    R: RequestContext<Name = CStr> + Send + Sync + 'static,
    R::ContextStack: ContextStack<Name = CStr>,
    RS: ReplyState<P::Frame> + Send + Sync + 'static,
{
    type Handler = ();
    type RequestContext = R;
    type ReplyState = RS;

    async fn call(
        &self,
        req: ProtocolDecoded<P>,
        rctxt: &R,
        reply_state: Arc<Mutex<RS>>,
    ) -> Result<(), Error> {
        let mut p = P::deserializer(req);

        const SERVICE_NAME: &str = "NullService";
        let ((name, ae), _, seqid) = p.read_message_begin(|name| {
            let name = String::from_utf8_lossy(name).into_owned();
            let ae = ApplicationException::unimplemented_method(SERVICE_NAME, &name);
            (name, ae)
        })?;

        p.skip(TType::Struct)?;
        p.read_message_end()?;

        rctxt.set_user_exception_header(ae.exn_name(), &ae.exn_value())?;
        let res = serialize!(P, |p| {
            p.write_message_begin(&name, ae.result_type().message_type(), seqid);
            ae.write(p);
            p.write_message_end();
        });
        reply_state.lock().unwrap().send_reply(res);
        Ok(())
    }

    fn create_interaction(
        &self,
        name: &str,
    ) -> ::anyhow::Result<
        Arc<
            dyn ThriftService<
                    P::Frame,
                    Handler = Self::Handler,
                    RequestContext = Self::RequestContext,
                    ReplyState = Self::ReplyState,
                > + ::std::marker::Send
                + 'static,
        >,
    > {
        bail!("Unimplemented interaction {}", name);
    }
}
