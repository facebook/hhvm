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
use std::sync::Arc;

use futures::FutureExt;
use futures::future;
use futures::future::BoxFuture;
use futures::stream::BoxStream;

use crate::Framing;
use crate::FramingDecoded;
use crate::FramingEncodedFinal;
use crate::Protocol;
use crate::help::Spawner;

#[derive(Debug)]
pub enum ClientStreamElement<Payload> {
    Reply(Payload),
    DeclaredEx(Payload),
    ApplicationEx(Payload),
}

impl<Payload> ClientStreamElement<Payload> {
    pub fn payload(&self) -> &Payload {
        match self {
            ClientStreamElement::Reply(payload)
            | ClientStreamElement::ApplicationEx(payload)
            | ClientStreamElement::DeclaredEx(payload) => payload,
        }
    }

    pub fn into_payload(self) -> Payload {
        match self {
            ClientStreamElement::Reply(payload)
            | ClientStreamElement::ApplicationEx(payload)
            | ClientStreamElement::DeclaredEx(payload) => payload,
        }
    }
}

pub struct SinkResult<TInitialReply, TSinkItem, TSinkExn, TFinalReply, TFinalExn> {
    pub initial_response: TInitialReply,
    pub sink: Box<
        dyn FnOnce(
                BoxStream<'static, Result<TSinkItem, TSinkExn>>,
            ) -> BoxFuture<'static, Result<TFinalReply, TFinalExn>>
            + Send
            + 'static,
    >,
}

pub struct SinkReply<P>
where
    P: Framing,
{
    pub initial_response: FramingDecoded<P>,
    pub sink_processor: Box<
        dyn FnOnce(
                BoxStream<'static, ClientStreamElement<FramingEncodedFinal<P>>>,
            )
                -> BoxFuture<'static, Result<FramingDecoded<P>, crate::NonthrowingFunctionError>>
            + Send
            + 'static,
    >,
}

#[allow(dead_code)]
pub struct BidiReply<P>
where
    P: Framing,
{
    pub initial_response: FramingDecoded<P>,
    pub stream: BoxStream<'static, Result<ClientStreamElement<FramingDecoded<P>>, anyhow::Error>>,
    pub sink_processor: Box<
        dyn FnOnce(
                BoxStream<'static, ClientStreamElement<FramingEncodedFinal<P>>>,
            )
                -> BoxFuture<'static, Result<FramingDecoded<P>, crate::NonthrowingFunctionError>>
            + Send
            + 'static,
    >,
}

pub trait ClientFactory {
    type Api: ?Sized;

    fn new<P, T>(protocol: P, transport: T) -> Arc<Self::Api>
    where
        P: Protocol<Frame = T> + 'static,
        T: Transport,
        P::Deserializer: Send,
    {
        let spawner = crate::NoopSpawner;
        Self::with_spawner(protocol, transport, spawner)
    }

    fn with_spawner<P, T, S>(protocol: P, transport: T, spawner: S) -> Arc<Self::Api>
    where
        P: Protocol<Frame = T> + 'static,
        T: Transport,
        P::Deserializer: Send,
        S: Spawner;
}

pub trait Transport: Framing + Send + Sync + Sized + 'static {
    type RpcOptions: Default;

    fn call(
        &self,
        service_name: &'static CStr,
        fn_name: &'static CStr,
        req: FramingEncodedFinal<Self>,
        rpc_options: Self::RpcOptions,
    ) -> BoxFuture<'static, anyhow::Result<FramingDecoded<Self>>>;

    fn call_stream(
        &self,
        _service_name: &'static CStr,
        _fn_name: &'static CStr,
        _req: FramingEncodedFinal<Self>,
        _rpc_options: Self::RpcOptions,
    ) -> BoxFuture<
        'static,
        anyhow::Result<(
            FramingDecoded<Self>,
            BoxStream<'static, anyhow::Result<ClientStreamElement<FramingDecoded<Self>>>>,
        )>,
    > {
        future::err(anyhow::Error::msg(
            "Streaming is not supported by this transport",
        ))
        .boxed()
    }

    fn call_sink(
        &self,
        _service_name: &'static CStr,
        _fn_name: &'static CStr,
        _req: FramingEncodedFinal<Self>,
        _rpc_options: Self::RpcOptions,
    ) -> BoxFuture<'static, anyhow::Result<SinkReply<Self>>> {
        future::err(anyhow::Error::msg(
            "Sink is not supported by this transport",
        ))
        .boxed()
    }

    fn call_bidirectional(
        &self,
        _service_name: &'static CStr,
        _fn_name: &'static CStr,
        _req: FramingEncodedFinal<Self>,
        _rpc_options: Self::RpcOptions,
    ) -> BoxFuture<'static, anyhow::Result<BidiReply<Self>>> {
        future::err(anyhow::Error::msg(
            "Bidirectional streaming is not supported by this transport",
        ))
        .boxed()
    }

    fn create_interaction(&self, _method_name: &'static CStr) -> Result<Self, anyhow::Error> {
        anyhow::bail!("Interactions are not supported by this transport");
    }
}
