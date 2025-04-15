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
use std::sync::Arc;
use std::time::Duration;

use anyhow::Context;
use clap::Parser;
use fbthrift::ApplicationException;
use futures::FutureExt;
use futures::StreamExt;
use futures::TryStreamExt;
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

    let test_case = Arc::new(Mutex::new(RpcTestCase::default()));
    let test_result = Arc::new(Mutex::new(ServerTestResult::default()));
    let runtime = tokio::runtime::Runtime::new()?;
    let service = move |proto| {
        rpc_services::rpc::make_RPCConformanceService_server(
            proto,
            RPCConformanceServiceImpl {
                fb,
                test_case: Arc::clone(&test_case),
                test_result: Arc::clone(&test_result),
            },
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

use std::sync::Mutex;

use async_trait::async_trait;
use futures::stream::BoxStream;
use rpc::rpc::InteractionConstructorServerTestResult;
use rpc::rpc::InteractionFactoryFunctionServerTestResult;
use rpc::rpc::InteractionPersistsStateServerTestResult;
use rpc::rpc::InteractionTerminationServerTestResult;
use rpc::rpc::Request;
use rpc::rpc::RequestResponseBasicServerTestResult;
use rpc::rpc::RequestResponseDeclaredExceptionServerTestResult;
use rpc::rpc::RequestResponseNoArgVoidResponseServerTestResult;
use rpc::rpc::RequestResponseUndeclaredExceptionServerTestResult;
use rpc::rpc::Response;
use rpc::rpc::RpcTestCase;
use rpc::rpc::ServerInstruction;
use rpc::rpc::ServerTestResult;
use rpc::rpc::SinkBasicServerTestResult;
use rpc::rpc::SinkChunkTimeoutServerTestResult;
use rpc::rpc::SinkDeclaredExceptionServerTestResult;
use rpc::rpc::SinkInitialResponseServerTestResult;
use rpc::rpc::SinkUndeclaredExceptionServerTestResult;
use rpc::rpc::StreamBasicServerTestResult;
use rpc::rpc::StreamDeclaredExceptionServerTestResult;
use rpc::rpc::StreamInitialDeclaredExceptionServerTestResult;
use rpc::rpc::StreamInitialResponseServerTestResult;
use rpc::rpc::StreamInitialUndeclaredExceptionServerTestResult;
use rpc::rpc::StreamUndeclaredExceptionServerTestResult;
use rpc::rpc::services::basic_interaction::AddExn;
use rpc::rpc::services::basic_interaction::InitExn;
use rpc::rpc::services::r_p_c_conformance_service::BasicInteractionFactoryFunctionExn;
use rpc::rpc::services::r_p_c_conformance_service::GetTestResultExn;
use rpc::rpc::services::r_p_c_conformance_service::RequestResponseBasicExn;
use rpc::rpc::services::r_p_c_conformance_service::RequestResponseDeclaredExceptionExn;
use rpc::rpc::services::r_p_c_conformance_service::RequestResponseNoArgVoidResponseExn;
use rpc::rpc::services::r_p_c_conformance_service::RequestResponseUndeclaredExceptionExn;
use rpc::rpc::services::r_p_c_conformance_service::SendTestCaseExn;
use rpc::rpc::services::r_p_c_conformance_service::SinkBasicExn;
use rpc::rpc::services::r_p_c_conformance_service::SinkBasicSinkExn;
use rpc::rpc::services::r_p_c_conformance_service::SinkBasicSinkFinalExn;
use rpc::rpc::services::r_p_c_conformance_service::SinkBasicSinkResult;
use rpc::rpc::services::r_p_c_conformance_service::SinkChunkTimeoutExn;
use rpc::rpc::services::r_p_c_conformance_service::SinkChunkTimeoutSinkExn;
use rpc::rpc::services::r_p_c_conformance_service::SinkChunkTimeoutSinkFinalExn;
use rpc::rpc::services::r_p_c_conformance_service::SinkChunkTimeoutSinkResult;
use rpc::rpc::services::r_p_c_conformance_service::SinkDeclaredExceptionExn;
use rpc::rpc::services::r_p_c_conformance_service::SinkDeclaredExceptionSinkExn;
use rpc::rpc::services::r_p_c_conformance_service::SinkDeclaredExceptionSinkResult;
use rpc::rpc::services::r_p_c_conformance_service::SinkInitialResponseExn;
use rpc::rpc::services::r_p_c_conformance_service::SinkInitialResponseSinkExn;
use rpc::rpc::services::r_p_c_conformance_service::SinkInitialResponseSinkFinalExn;
use rpc::rpc::services::r_p_c_conformance_service::SinkInitialResponseSinkResult;
use rpc::rpc::services::r_p_c_conformance_service::SinkUndeclaredExceptionExn;
use rpc::rpc::services::r_p_c_conformance_service::SinkUndeclaredExceptionSinkExn;
use rpc::rpc::services::r_p_c_conformance_service::SinkUndeclaredExceptionSinkResult;
use rpc::rpc::services::r_p_c_conformance_service::StreamBasicExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamBasicStreamExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamDeclaredExceptionExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamDeclaredExceptionStreamExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamInitialDeclaredExceptionExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamInitialDeclaredExceptionStreamExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamInitialResponseExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamInitialResponseStreamExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamInitialUndeclaredExceptionExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamInitialUndeclaredExceptionStreamExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamUndeclaredExceptionExn;
use rpc::rpc::services::r_p_c_conformance_service::StreamUndeclaredExceptionStreamExn;
use rpc_services::rpc::BasicInteraction;
use rpc_services::rpc::RPCConformanceService;

// ---

struct BasicInteractionImpl {
    pub test_case: Arc<Mutex<rpc::rpc::RpcTestCase>>,
    pub test_result: Arc<Mutex<rpc::rpc::ServerTestResult>>,
    pub storage: Mutex<i32>,
}

impl BasicInteractionImpl {
    fn new(
        init: i32,
        test_case: Arc<Mutex<rpc::rpc::RpcTestCase>>,
        test_result: Arc<Mutex<rpc::rpc::ServerTestResult>>,
    ) -> Self {
        Self {
            storage: std::sync::Mutex::new(init),
            test_case,
            test_result,
        }
    }
}

#[async_trait::async_trait]
impl rpc_services::rpc::BasicInteraction for BasicInteractionImpl {
    async fn init(&self) -> Result<(), InitExn> {
        let mut cell = self.storage.lock().unwrap();
        *cell = 0i32;
        Ok(())
    }
    async fn add(&self, i: i32) -> Result<i32, AddExn> {
        let mut cell = self.storage.lock().unwrap();
        *cell += i;
        Ok(*cell)
    }
    async fn on_termination(&self) {
        let r = self.test_case.lock().unwrap();
        if let ServerInstruction::interactionTermination(_instr) = &r.serverInstruction {
            let mut w = self.test_result.lock().unwrap();
            *w = ServerTestResult::interactionTermination(InteractionTerminationServerTestResult {
                terminationReceived: true,
                ..Default::default()
            });
        }
    }
}

// --

#[derive(Clone)]
pub struct RPCConformanceServiceImpl {
    pub fb: fbinit::FacebookInit,
    pub test_case: Arc<Mutex<rpc::rpc::RpcTestCase>>,
    pub test_result: Arc<Mutex<rpc::rpc::ServerTestResult>>,
}

#[async_trait]
impl RPCConformanceService for RPCConformanceServiceImpl {
    async fn sendTestCase(&self, test_case: RpcTestCase) -> Result<(), SendTestCaseExn> {
        let mut w = self.test_case.lock().unwrap();
        *w = test_case;
        Ok(())
    }

    async fn getTestResult(&self) -> Result<ServerTestResult, GetTestResultExn> {
        let r = self.test_result.lock().unwrap();
        Ok(r.clone())
    }

    async fn requestResponseBasic(
        &self,
        request: Request,
    ) -> Result<Response, RequestResponseBasicExn> {
        let mut w = self.test_result.lock().unwrap();
        *w = ServerTestResult::requestResponseBasic(RequestResponseBasicServerTestResult {
            request,
            ..Default::default()
        });

        let r = self.test_case.lock().unwrap();
        match &r.serverInstruction {
            ServerInstruction::requestResponseBasic(instr) => Ok(instr.response.clone()),
            _ => Err(RequestResponseBasicExn::ApplicationException(
                instruction_match_error(),
            )),
        }
    }

    async fn requestResponseDeclaredException(
        &self,
        request: Request,
    ) -> Result<(), RequestResponseDeclaredExceptionExn> {
        let mut w = self.test_result.lock().unwrap();
        *w = ServerTestResult::requestResponseDeclaredException(
            RequestResponseDeclaredExceptionServerTestResult {
                request,
                ..Default::default()
            },
        );

        let r = self.test_case.lock().unwrap();
        match &r.serverInstruction {
            ServerInstruction::requestResponseDeclaredException(instr) => {
                match &instr.userException {
                    Some(e) => Err(RequestResponseDeclaredExceptionExn::e(*e.clone())),
                    None => Err(RequestResponseDeclaredExceptionExn::ApplicationException(
                        none_error(),
                    )),
                }
            }
            _ => Err(RequestResponseDeclaredExceptionExn::ApplicationException(
                instruction_match_error(),
            )),
        }
    }

    async fn requestResponseUndeclaredException(
        &self,
        request: Request,
    ) -> Result<(), RequestResponseUndeclaredExceptionExn> {
        let mut w = self.test_result.lock().unwrap();
        *w = ServerTestResult::requestResponseUndeclaredException(
            RequestResponseUndeclaredExceptionServerTestResult {
                request,
                ..Default::default()
            },
        );

        let r = self.test_case.lock().unwrap();
        match &r.serverInstruction {
            ServerInstruction::requestResponseUndeclaredException(instr) => {
                Err(RequestResponseUndeclaredExceptionExn::ApplicationException(
                    custom_error(instr.exceptionMessage.clone()),
                ))
            }
            _ => Err(RequestResponseUndeclaredExceptionExn::ApplicationException(
                instruction_match_error(),
            )),
        }
    }

    async fn requestResponseNoArgVoidResponse(
        &self,
    ) -> Result<(), RequestResponseNoArgVoidResponseExn> {
        let mut w = self.test_result.lock().unwrap();
        *w = ServerTestResult::requestResponseNoArgVoidResponse(
            RequestResponseNoArgVoidResponseServerTestResult {
                ..Default::default()
            },
        );
        Ok(())
    }

    fn createBasicInteraction(&self) -> ::anyhow::Result<Box<dyn BasicInteraction>> {
        let r = self.test_case.lock().unwrap();
        match &r.serverInstruction {
            ServerInstruction::interactionConstructor(_instr) => {
                let mut w = self.test_result.lock().unwrap();
                *w = ServerTestResult::interactionConstructor(
                    InteractionConstructorServerTestResult {
                        constructorCalled: true,
                        ..Default::default()
                    },
                );
                Ok(Box::new(BasicInteractionImpl::new(
                    0i32,
                    Arc::clone(&self.test_case),
                    Arc::clone(&self.test_result),
                )))
            }
            ServerInstruction::interactionPersistsState(_instr) => {
                let mut w = self.test_result.lock().unwrap();
                *w = ServerTestResult::interactionPersistsState(
                    InteractionPersistsStateServerTestResult {
                        ..Default::default()
                    },
                );
                Ok(Box::new(BasicInteractionImpl::new(
                    0i32,
                    Arc::clone(&self.test_case),
                    Arc::clone(&self.test_result),
                )))
            }
            ServerInstruction::interactionTermination(_instr) => {
                let mut w = self.test_result.lock().unwrap();
                *w = ServerTestResult::interactionTermination(
                    InteractionTerminationServerTestResult {
                        terminationReceived: false,
                        ..Default::default()
                    },
                );
                Ok(Box::new(BasicInteractionImpl::new(
                    0i32,
                    Arc::clone(&self.test_case),
                    Arc::clone(&self.test_result),
                )))
            }
            _ => Err(instruction_match_error().into()),
        }
    }

    async fn basicInteractionFactoryFunction(
        &self,
        init: i32,
    ) -> Result<Box<dyn BasicInteraction>, BasicInteractionFactoryFunctionExn> {
        let r = self.test_case.lock().unwrap();
        match &r.serverInstruction {
            ServerInstruction::interactionFactoryFunction(_instr) => {
                let mut w = self.test_result.lock().unwrap();
                *w = ServerTestResult::interactionFactoryFunction(
                    InteractionFactoryFunctionServerTestResult {
                        initialSum: init,
                        ..Default::default()
                    },
                );
                Ok(Box::new(BasicInteractionImpl::new(
                    init,
                    Arc::clone(&self.test_case),
                    Arc::clone(&self.test_result),
                )))
            }
            ServerInstruction::interactionPersistsState(_instr) => {
                let mut w = self.test_result.lock().unwrap();
                *w = ServerTestResult::interactionPersistsState(
                    InteractionPersistsStateServerTestResult {
                        ..Default::default()
                    },
                );
                Ok(Box::new(BasicInteractionImpl::new(
                    init,
                    Arc::clone(&self.test_case),
                    Arc::clone(&self.test_result),
                )))
            }
            ServerInstruction::interactionTermination(_instr) => {
                let mut w = self.test_result.lock().unwrap();
                *w = ServerTestResult::interactionTermination(
                    InteractionTerminationServerTestResult {
                        terminationReceived: false,
                        ..Default::default()
                    },
                );
                Ok(Box::new(BasicInteractionImpl::new(
                    init,
                    Arc::clone(&self.test_case),
                    Arc::clone(&self.test_result),
                )))
            }
            _ => Err(BasicInteractionFactoryFunctionExn::ApplicationException(
                instruction_match_error(),
            )),
        }
    }

    async fn streamBasic(
        &self,
        request: Request,
    ) -> Result<BoxStream<'static, Result<Response, StreamBasicStreamExn>>, StreamBasicExn> {
        let mut w = self.test_result.lock().unwrap();
        *w = ServerTestResult::streamBasic(StreamBasicServerTestResult {
            request,
            ..Default::default()
        });
        let r = self.test_case.lock().unwrap();
        match &r.serverInstruction {
            ServerInstruction::streamBasic(instr) => Ok(Box::pin(
                futures::stream::iter(instr.streamPayloads.clone()).map(Result::Ok),
            )),
            _ => Err(StreamBasicExn::ApplicationException(
                instruction_match_error(),
            )),
        }
    }

    async fn streamInitialResponse(
        &self,
        request: Request,
    ) -> Result<
        (
            Response,
            BoxStream<'static, Result<Response, StreamInitialResponseStreamExn>>,
        ),
        StreamInitialResponseExn,
    > {
        let mut w = self.test_result.lock().unwrap();
        *w = ServerTestResult::streamInitialResponse(StreamInitialResponseServerTestResult {
            request,
            ..Default::default()
        });
        let r = self.test_case.lock().unwrap();
        match &r.serverInstruction {
            ServerInstruction::streamInitialResponse(instr) => Ok((
                instr.initialResponse.clone(),
                Box::pin(futures::stream::iter(instr.streamPayloads.clone()).map(Result::Ok)),
            )),
            _ => Err(StreamInitialResponseExn::ApplicationException(
                instruction_match_error(),
            )),
        }
    }

    async fn streamDeclaredException(
        &self,
        request: Request,
    ) -> Result<
        BoxStream<'static, Result<Response, StreamDeclaredExceptionStreamExn>>,
        StreamDeclaredExceptionExn,
    > {
        let mut w = self.test_result.lock().unwrap();
        *w = ServerTestResult::streamDeclaredException(StreamDeclaredExceptionServerTestResult {
            request,
            ..Default::default()
        });
        let r = self.test_case.lock().unwrap();
        match &r.serverInstruction {
            ServerInstruction::streamDeclaredException(instr) => {
                if let Some(exception) = &instr.userException {
                    Ok(Box::pin(futures::stream::once(futures::future::ready(
                        Err(StreamDeclaredExceptionStreamExn::e(*exception.clone())),
                    ))))
                } else {
                    Err(StreamDeclaredExceptionExn::ApplicationException(
                        none_error(),
                    ))
                }
            }
            _ => Err(StreamDeclaredExceptionExn::ApplicationException(
                instruction_match_error(),
            )),
        }
    }

    async fn streamInitialDeclaredException(
        &self,
        request: Request,
    ) -> Result<
        BoxStream<'static, Result<Response, StreamInitialDeclaredExceptionStreamExn>>,
        StreamInitialDeclaredExceptionExn,
    > {
        let mut w = self.test_result.lock().unwrap();
        *w = ServerTestResult::streamInitialDeclaredException(
            StreamInitialDeclaredExceptionServerTestResult {
                request,
                ..Default::default()
            },
        );
        let r = self.test_case.lock().unwrap();
        match &r.serverInstruction {
            ServerInstruction::streamInitialDeclaredException(instr) => {
                if let Some(exception) = &instr.userException {
                    Err(StreamInitialDeclaredExceptionExn::e(*exception.clone()))
                } else {
                    Err(StreamInitialDeclaredExceptionExn::ApplicationException(
                        none_error(),
                    ))
                }
            }
            _ => Err(StreamInitialDeclaredExceptionExn::ApplicationException(
                instruction_match_error(),
            )),
        }
    }

    async fn streamUndeclaredException(
        &self,
        request: Request,
    ) -> Result<
        BoxStream<'static, Result<Response, StreamUndeclaredExceptionStreamExn>>,
        StreamUndeclaredExceptionExn,
    > {
        let mut w = self.test_result.lock().unwrap();
        *w = ServerTestResult::streamUndeclaredException(
            StreamUndeclaredExceptionServerTestResult {
                request,
                ..Default::default()
            },
        );
        let r = self.test_case.lock().unwrap();
        match &r.serverInstruction {
            ServerInstruction::streamUndeclaredException(instr) => {
                Ok(Box::pin(futures::stream::once(futures::future::ready(
                    Err(StreamUndeclaredExceptionStreamExn::ApplicationException(
                        custom_error(instr.exceptionMessage.clone()),
                    )),
                ))))
            }
            _ => Err(StreamUndeclaredExceptionExn::ApplicationException(
                instruction_match_error(),
            )),
        }
    }

    async fn streamInitialUndeclaredException(
        &self,
        request: Request,
    ) -> Result<
        BoxStream<'static, Result<Response, StreamInitialUndeclaredExceptionStreamExn>>,
        StreamInitialUndeclaredExceptionExn,
    > {
        let mut w = self.test_result.lock().unwrap();
        *w = ServerTestResult::streamInitialUndeclaredException(
            StreamInitialUndeclaredExceptionServerTestResult {
                request,
                ..Default::default()
            },
        );
        let r = self.test_case.lock().unwrap();
        match &r.serverInstruction {
            ServerInstruction::streamInitialUndeclaredException(instr) => {
                Err(StreamInitialUndeclaredExceptionExn::ApplicationException(
                    custom_error(instr.exceptionMessage.clone()),
                ))
            }
            _ => Err(StreamInitialUndeclaredExceptionExn::ApplicationException(
                instruction_match_error(),
            )),
        }
    }

    async fn sinkBasic(&self, request: Request) -> Result<SinkBasicSinkResult, SinkBasicExn> {
        let self_clone = self.clone();
        // extract test settings
        let r = self.test_case.lock().unwrap();
        let (buffer_size, final_response) = match &r.serverInstruction {
            ServerInstruction::sinkBasic(instr) => {
                (instr.bufferSize as u64, instr.finalResponse.clone())
            }
            _ => {
                return Err(SinkBasicExn::ApplicationException(instruction_match_error()));
            }
        };
        drop(r);

        let handler = Box::new(
            move |stream: BoxStream<'static, Result<Request, SinkBasicSinkExn>>| {
                (async move {
                    let stream_items = stream
                        .try_collect::<Vec<Request>>()
                        .await
                        .unwrap_or_default();
                    let mut w = self_clone.test_result.lock().unwrap();
                    *w = ServerTestResult::sinkBasic(SinkBasicServerTestResult {
                        request,
                        sinkPayloads: stream_items,
                        ..Default::default()
                    });
                    Ok::<Response, SinkBasicSinkFinalExn>(final_response)
                })
                .boxed()
            },
        );
        Ok(SinkBasicSinkResult::new(handler).with_buffer_size(buffer_size))
    }

    async fn sinkChunkTimeout(
        &self,
        request: Request,
    ) -> Result<SinkChunkTimeoutSinkResult, SinkChunkTimeoutExn> {
        let self_clone = self.clone();
        // extract test settings
        let r = self.test_case.lock().unwrap();
        let (chunk_timeout, final_response) = match &r.serverInstruction {
            ServerInstruction::sinkChunkTimeout(instr) => (
                Duration::from_millis(instr.chunkTimeoutMs as u64),
                instr.finalResponse.clone(),
            ),
            _ => {
                return Err(SinkChunkTimeoutExn::ApplicationException(
                    instruction_match_error(),
                ));
            }
        };
        drop(r);

        let handler = Box::new(
            move |mut stream: BoxStream<'static, Result<Request, SinkChunkTimeoutSinkExn>>| {
                (async move {
                    let mut payloads = vec![];
                    let mut timeout = false;
                    while let Some(item) = stream.next().await {
                        match item {
                            Ok(payload) => payloads.push(payload),
                            Err(SinkChunkTimeoutSinkExn::ApplicationException(
                                ApplicationException { .. },
                            )) => {
                                timeout = true;
                            }
                            #[allow(unreachable_patterns)]
                            Err(_other) => {
                                // ignore
                            }
                        }
                    }
                    let mut w = self_clone.test_result.lock().unwrap();
                    *w = ServerTestResult::sinkChunkTimeout(SinkChunkTimeoutServerTestResult {
                        request,
                        sinkPayloads: payloads,
                        chunkTimeoutException: timeout,
                        ..Default::default()
                    });
                    Ok::<Response, SinkChunkTimeoutSinkFinalExn>(final_response)
                })
                .boxed()
            },
        );
        Ok(SinkChunkTimeoutSinkResult::new(handler).with_chunk_timeout(chunk_timeout))
    }

    async fn sinkInitialResponse(
        &self,
        request: Request,
    ) -> Result<SinkInitialResponseSinkResult, SinkInitialResponseExn> {
        let self_clone = self.clone();
        // extract test settings
        let r = self.test_case.lock().unwrap();
        let (buffer_size, initial_response, final_response) = match &r.serverInstruction {
            ServerInstruction::sinkInitialResponse(instr) => (
                instr.bufferSize as u64,
                instr.initialResponse.clone(),
                instr.finalResponse.clone(),
            ),
            _ => {
                return Err(SinkInitialResponseExn::ApplicationException(
                    instruction_match_error(),
                ));
            }
        };
        drop(r);

        let handler = Box::new(
            move |stream: BoxStream<'static, Result<Request, SinkInitialResponseSinkExn>>| {
                (async move {
                    let stream_items = stream
                        .try_collect::<Vec<Request>>()
                        .await
                        .unwrap_or_default();
                    let mut w = self_clone.test_result.lock().unwrap();
                    *w = ServerTestResult::sinkInitialResponse(
                        SinkInitialResponseServerTestResult {
                            request,
                            sinkPayloads: stream_items,
                            ..Default::default()
                        },
                    );
                    Ok::<Response, SinkInitialResponseSinkFinalExn>(final_response)
                })
                .boxed()
            },
        );
        Ok(
            SinkInitialResponseSinkResult::new(initial_response, handler)
                .with_buffer_size(buffer_size),
        )
    }

    async fn sinkUndeclaredException(
        &self,
        request: Request,
    ) -> Result<SinkUndeclaredExceptionSinkResult, SinkUndeclaredExceptionExn> {
        let self_clone = self.clone();
        // extract test settings
        let r = self.test_case.lock().unwrap();
        let buffer_size = match &r.serverInstruction {
            ServerInstruction::sinkUndeclaredException(instr) => instr.bufferSize as u64,
            _ => {
                return Err(SinkUndeclaredExceptionExn::ApplicationException(
                    instruction_match_error(),
                ));
            }
        };
        drop(r);

        let handler = Box::new(
            move |mut stream: BoxStream<
                'static,
                Result<Request, SinkUndeclaredExceptionSinkExn>,
            >| {
                (async move {
                    let message = if let Some(Err(
                        SinkUndeclaredExceptionSinkExn::ApplicationException(aexn),
                    )) = stream.next().await
                    {
                        Some(aexn.message)
                    } else {
                        None
                    };

                    let mut w = self_clone.test_result.lock().unwrap();
                    *w = ServerTestResult::sinkUndeclaredException(
                        SinkUndeclaredExceptionServerTestResult {
                            request,
                            exceptionMessage: message,
                            ..Default::default()
                        },
                    );

                    Ok(Response::default())
                })
                .boxed()
            },
        );
        Ok(SinkUndeclaredExceptionSinkResult::new(handler).with_buffer_size(buffer_size))
    }

    async fn sinkDeclaredException(
        &self,
        request: Request,
    ) -> Result<SinkDeclaredExceptionSinkResult, SinkDeclaredExceptionExn> {
        let self_clone = self.clone();
        // extract test settings
        let r = self.test_case.lock().unwrap();
        let buffer_size = match &r.serverInstruction {
            ServerInstruction::sinkDeclaredException(instr) => instr.bufferSize as u64,
            _ => {
                return Err(SinkDeclaredExceptionExn::ApplicationException(
                    instruction_match_error(),
                ));
            }
        };
        drop(r);

        let handler = Box::new(
            move |mut stream: BoxStream<'static, Result<Request, SinkDeclaredExceptionSinkExn>>| {
                (async move {
                    let next = stream.next().await;
                    let ex = if let Some(Err(SinkDeclaredExceptionSinkExn::e(ue))) = &next {
                        Some(ue.clone())
                    } else {
                        None
                    };

                    let mut w = self_clone.test_result.lock().unwrap();
                    *w = ServerTestResult::sinkDeclaredException(
                        SinkDeclaredExceptionServerTestResult {
                            request,
                            userException: ex,
                            ..Default::default()
                        },
                    );

                    Ok(Response::default())
                })
                .boxed()
            },
        );
        Ok(SinkDeclaredExceptionSinkResult::new(handler).with_buffer_size(buffer_size))
    }
}

fn instruction_match_error() -> fbthrift::ApplicationException {
    fbthrift::ApplicationException::new(
        fbthrift::ApplicationExceptionErrorCode::InternalError,
        "the current instruction is of an unexpected case".to_owned(),
    )
}

fn none_error() -> fbthrift::ApplicationException {
    fbthrift::ApplicationException::new(
        fbthrift::ApplicationExceptionErrorCode::InternalError,
        "an option value was not expected to contain `None`".to_owned(),
    )
}

fn custom_error(msg: String) -> fbthrift::ApplicationException {
    fbthrift::ApplicationException::new(fbthrift::ApplicationExceptionErrorCode::InternalError, msg)
}
