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

use std::io::IsTerminal;

use anyhow::Context;
use async_trait::async_trait;
use clap::Parser;
use futures::StreamExt;
use rpc_services::rpc::RPCConformanceService;
use tracing_subscriber::layer::SubscriberExt;

#[derive(Debug, Parser)]
#[clap(name = "Conformance Server")]
struct Arguments {
    #[clap(short, long, default_value = "0")]
    port: u16,
    #[clap(short, long, default_value = "info", use_value_delimiter = true)]
    log: Vec<tracing_subscriber::filter::Directive>,
}

#[fbinit::main]
fn main(fb: fbinit::FacebookInit) -> anyhow::Result<()> {
    let args = Arguments::parse();

    init_logging(args.log);

    let runtime = tokio::runtime::Runtime::new()?;
    let service = move |proto| {
        rpc_services::rpc::make_RPCConformanceService_server(
            proto,
            RPCConformanceServiceImpl { fb },
        )
    };
    let thrift_server = srserver::ThriftServerBuilder::new(fb)
        .with_port(args.port)
        .with_allow_plaintext_on_loopback()
        .with_metadata(RPCConformanceService_metadata_sys::create_metadata())
        .with_factory(runtime.handle().clone(), move || service)
        .build();

    let mut svc_framework =
        srserver::service_framework::ServiceFramework::from_server("rpc_server", thrift_server)
            .context("Failed to create service framework server")?;
    svc_framework.add_module(srserver::service_framework::BuildModule)?;
    svc_framework.add_module(srserver::service_framework::ThriftStatsModule)?;
    svc_framework.add_module(srserver::service_framework::Fb303Module)?;

    let thrift_service_handle = runtime.spawn(async move {
        use signal_hook::consts::signal::SIGINT;
        use signal_hook::consts::signal::SIGTERM;

        svc_framework.serve_background()?;
        println!("{:#?}", svc_framework.get_address()?.get_port()?);

        let mut signals = signal_hook_tokio::Signals::new([SIGTERM, SIGINT])?;
        signals.next().await;

        svc_framework.stop();
        signals.handle().close();

        Ok(())
    });
    runtime.block_on(thrift_service_handle)?
}

fn init_logging(directives: Vec<tracing_subscriber::filter::Directive>) {
    let fmt = tracing_subscriber::fmt::Layer::default()
        .with_ansi(std::io::stderr().is_terminal())
        .with_writer(std::io::stderr)
        .event_format(tracing_glog::Glog::default().with_timer(tracing_glog::LocalTime::default()))
        .fmt_fields(tracing_glog::GlogFields::default());
    let filter = directives.into_iter().fold(
        tracing_subscriber::EnvFilter::from_default_env(),
        |filter, directive| filter.add_directive(directive),
    );
    let subscriber = tracing_subscriber::Registry::default()
        .with(filter)
        .with(fmt);
    tracing::subscriber::set_global_default(subscriber).expect("to set global subscriber");
}

#[derive(Clone)]
pub struct RPCConformanceServiceImpl {
    pub fb: fbinit::FacebookInit,
}

use futures::stream::BoxStream;
use rpc::rpc::services::r_p_c_conformance_service::BasicInteractionFactoryFunctionExn;
use rpc::rpc::services::r_p_c_conformance_service::GetTestCaseExn;
use rpc::rpc::services::r_p_c_conformance_service::GetTestResultExn;
use rpc::rpc::services::r_p_c_conformance_service::RequestResponseBasicExn;
use rpc::rpc::services::r_p_c_conformance_service::RequestResponseDeclaredExceptionExn;
use rpc::rpc::services::r_p_c_conformance_service::RequestResponseNoArgVoidResponseExn;
use rpc::rpc::services::r_p_c_conformance_service::RequestResponseTimeoutExn;
use rpc::rpc::services::r_p_c_conformance_service::RequestResponseUndeclaredExceptionExn;
use rpc::rpc::services::r_p_c_conformance_service::SendTestCaseExn;
use rpc::rpc::services::r_p_c_conformance_service::SendTestResultExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamBasicExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamBasicStreamExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamChunkTimeoutExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamChunkTimeoutStreamExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamCreditTimeoutExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamCreditTimeoutStreamExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamDeclaredExceptionExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamDeclaredExceptionStreamExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamInitialDeclaredExceptionExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamInitialDeclaredExceptionStreamExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamInitialResponseExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamInitialResponseStreamExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamInitialTimeoutExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamInitialTimeoutStreamExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamInitialUndeclaredExceptionExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamInitialUndeclaredExceptionStreamExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamUndeclaredExceptionExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamUndeclaredExceptionStreamExn;
use rpc::rpc::ClientTestResult;
use rpc::rpc::Request;
use rpc::rpc::Response;
use rpc::rpc::RpcTestCase;
use rpc::rpc::ServerTestResult;
use rpc_services::rpc::BasicInteraction;

#[async_trait]
impl RPCConformanceService for RPCConformanceServiceImpl {
    async fn sendTestCase(&self, _test_case: RpcTestCase) -> Result<(), SendTestCaseExn> {
        Err(SendTestCaseExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "sendTestCase",
            ),
        ))
    }

    async fn getTestResult(&self) -> Result<ServerTestResult, GetTestResultExn> {
        Err(GetTestResultExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "getTestResult",
            ),
        ))
    }

    async fn getTestCase(&self) -> Result<RpcTestCase, GetTestCaseExn> {
        Err(GetTestCaseExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "getTestCase",
            ),
        ))
    }

    async fn sendTestResult(&self, _result: ClientTestResult) -> Result<(), SendTestResultExn> {
        Err(SendTestResultExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "sendTestResult",
            ),
        ))
    }

    async fn requestResponseBasic(
        &self,
        _req: Request,
    ) -> Result<Response, RequestResponseBasicExn> {
        Err(RequestResponseBasicExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "requestResponseBasic",
            ),
        ))
    }

    async fn requestResponseDeclaredException(
        &self,
        _req: Request,
    ) -> Result<(), RequestResponseDeclaredExceptionExn> {
        Err(RequestResponseDeclaredExceptionExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "requestResponseDeclaredException",
            ),
        ))
    }

    async fn requestResponseUndeclaredException(
        &self,
        _req: Request,
    ) -> Result<(), RequestResponseUndeclaredExceptionExn> {
        Err(RequestResponseUndeclaredExceptionExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "requestResponseUndeclaredException",
            ),
        ))
    }

    async fn requestResponseNoArgVoidResponse(
        &self,
    ) -> Result<(), RequestResponseNoArgVoidResponseExn> {
        Err(RequestResponseNoArgVoidResponseExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "requestResponseNoArgVoidResponse",
            ),
        ))
    }

    async fn requestResponseTimeout(
        &self,
        _req: Request,
    ) -> Result<Response, RequestResponseTimeoutExn> {
        Err(RequestResponseTimeoutExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "requestResponseTimeout",
            ),
        ))
    }

    async fn streamBasic(
        &self,
        _req: Request,
    ) -> Result<BoxStream<'static, Result<Response, StreamBasicStreamExn>>, StreamBasicExn> {
        Err(StreamBasicExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "streamBasic",
            ),
        ))
    }

    async fn streamChunkTimeout(
        &self,
        _req: Request,
    ) -> Result<
        BoxStream<'static, Result<Response, StreamChunkTimeoutStreamExn>>,
        StreamChunkTimeoutExn,
    > {
        Err(StreamChunkTimeoutExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "streamChunkTimeout",
            ),
        ))
    }

    async fn streamInitialResponse(
        &self,
        _req: Request,
    ) -> Result<
        (
            Response,
            BoxStream<'static, Result<Response, StreamInitialResponseStreamExn>>,
        ),
        StreamInitialResponseExn,
    > {
        Err(StreamInitialResponseExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "streamInitialResponse",
            ),
        ))
    }

    async fn streamCreditTimeout(
        &self,
        _req: Request,
    ) -> Result<
        BoxStream<'static, Result<Response, StreamCreditTimeoutStreamExn>>,
        StreamCreditTimeoutExn,
    > {
        Err(StreamCreditTimeoutExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "streamCreditTimeout",
            ),
        ))
    }

    async fn streamDeclaredException(
        &self,
        _req: Request,
    ) -> Result<
        BoxStream<'static, Result<Response, StreamDeclaredExceptionStreamExn>>,
        StreamDeclaredExceptionExn,
    > {
        Err(StreamDeclaredExceptionExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "streamDeclaredException",
            ),
        ))
    }

    async fn streamUndeclaredException(
        &self,
        _req: Request,
    ) -> Result<
        BoxStream<'static, Result<Response, StreamUndeclaredExceptionStreamExn>>,
        StreamUndeclaredExceptionExn,
    > {
        Err(StreamUndeclaredExceptionExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "streamUndeclaredException",
            ),
        ))
    }

    async fn streamInitialDeclaredException(
        &self,
        _req: Request,
    ) -> Result<
        BoxStream<'static, Result<Response, StreamInitialDeclaredExceptionStreamExn>>,
        StreamInitialDeclaredExceptionExn,
    > {
        Err(StreamInitialDeclaredExceptionExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "streamInitialDeclaredException",
            ),
        ))
    }

    async fn streamInitialUndeclaredException(
        &self,
        _req: Request,
    ) -> Result<
        BoxStream<'static, Result<Response, StreamInitialUndeclaredExceptionStreamExn>>,
        StreamInitialUndeclaredExceptionExn,
    > {
        Err(StreamInitialUndeclaredExceptionExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "streamInitialUndeclaredException",
            ),
        ))
    }

    async fn streamInitialTimeout(
        &self,
        _req: Request,
    ) -> Result<
        BoxStream<'static, Result<Response, StreamInitialTimeoutStreamExn>>,
        StreamInitialTimeoutExn,
    > {
        Err(StreamInitialTimeoutExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "streamInitialTimeout",
            ),
        ))
    }

    fn createBasicInteraction(&self) -> anyhow::Result<Box<dyn BasicInteraction>> {
        anyhow::bail!("RPCConformanceService.createBasicInteraction not implemented");
    }

    async fn basicInteractionFactoryFunction(
        &self,
        _initial_sum: i32,
    ) -> Result<Box<dyn BasicInteraction>, BasicInteractionFactoryFunctionExn> {
        Err(BasicInteractionFactoryFunctionExn::ApplicationException(
            ::fbthrift::ApplicationException::unimplemented_method(
                "RPCConformanceService",
                "basicInteractionFactoryFunction",
            ),
        ))
    }
}
