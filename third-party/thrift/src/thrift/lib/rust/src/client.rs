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

use futures::future;
use futures::future::BoxFuture;
use futures::stream::BoxStream;
use futures::FutureExt;

use crate::help::Spawner;
use crate::Framing;
use crate::FramingDecoded;
use crate::FramingEncodedFinal;
use crate::Protocol;

pub trait ClientFactory {
    type Api: ?Sized;

    fn new<P, T>(protocol: P, transport: T) -> Arc<Self::Api>
    where
        P: Protocol<Frame = T> + 'static,
        T: Transport + Sync,
        P::Deserializer: Send,
    {
        let spawner = crate::NoopSpawner;
        Self::with_spawner(protocol, transport, spawner)
    }

    fn with_spawner<P, T, S>(protocol: P, transport: T, spawner: S) -> Arc<Self::Api>
    where
        P: Protocol<Frame = T> + 'static,
        T: Transport + Sync,
        P::Deserializer: Send,
        S: Spawner;
}

pub trait Transport: Framing + Send + Sized + 'static {
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
            BoxStream<'static, anyhow::Result<FramingDecoded<Self>>>,
        )>,
    > {
        future::err(anyhow::Error::msg(
            "Streaming is not supported by this transport",
        ))
        .boxed()
    }

    fn create_interaction(&self, _method_name: &'static CStr) -> Result<Self, anyhow::Error> {
        anyhow::bail!("Interactions are not supported by this transport");
    }
}
