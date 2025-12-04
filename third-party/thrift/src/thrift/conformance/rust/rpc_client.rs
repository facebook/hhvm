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

use std::net::SocketAddr;
use std::sync::Arc;
use std::time::Duration;

use anyhow::Result;
use anyhow::anyhow;
use clap::Parser;
use futures::stream::StreamExt;
use rpc::rpc::SinkBasicClientInstruction;
use rpc::rpc::SinkBasicClientTestResult;
use rpc::rpc::SinkChunkTimeoutClientInstruction;
use rpc::rpc::SinkChunkTimeoutClientTestResult;
use rpc::rpc::SinkDeclaredExceptionClientInstruction;
use rpc::rpc::SinkDeclaredExceptionClientTestResult;
use rpc::rpc::SinkInitialResponseClientInstruction;
use rpc::rpc::SinkInitialResponseClientTestResult;
use rpc::rpc::SinkUndeclaredExceptionClientInstruction;
use rpc::rpc::SinkUndeclaredExceptionClientTestResult;
use rpc::rpc::StreamBasicClientInstruction;
use rpc::rpc::StreamBasicClientTestResult;
use rpc::rpc::StreamChunkTimeoutClientInstruction;
use rpc::rpc::StreamChunkTimeoutClientTestResult;
use rpc::rpc::StreamCreditTimeoutClientInstruction;
use rpc::rpc::StreamCreditTimeoutClientTestResult;
use rpc::rpc::StreamDeclaredExceptionClientInstruction;
use rpc::rpc::StreamDeclaredExceptionClientTestResult;
use rpc::rpc::StreamInitialDeclaredExceptionClientInstruction;
use rpc::rpc::StreamInitialDeclaredExceptionClientTestResult;
use rpc::rpc::StreamInitialResponseClientInstruction;
use rpc::rpc::StreamInitialResponseClientTestResult;
use rpc::rpc::StreamInitialTimeoutClientInstruction;
use rpc::rpc::StreamInitialTimeoutClientTestResult;
use rpc::rpc::StreamInitialUndeclaredExceptionClientInstruction;
use rpc::rpc::StreamInitialUndeclaredExceptionClientTestResult;
use rpc::rpc::StreamUndeclaredExceptionClientInstruction;
use rpc::rpc::StreamUndeclaredExceptionClientTestResult;
use rpc_clients::rpc::RPCConformanceServiceExt;
use rpc_clients::rpc::errors::r_p_c_conformance_service::SinkChunkTimeoutSinkFinalError;
use rpc_clients::rpc::errors::r_p_c_conformance_service::SinkDeclaredExceptionSinkError;
use rpc_clients::rpc::errors::r_p_c_conformance_service::SinkUndeclaredExceptionSinkError;
use rpc_clients::rpc::errors::r_p_c_conformance_service::StreamDeclaredExceptionStreamError;
use rpc_clients::rpc::errors::r_p_c_conformance_service::StreamInitialDeclaredExceptionError;
use tracing_glog::Glog;
use tracing_glog::GlogFields;
use tracing_subscriber::EnvFilter;
use tracing_subscriber::Registry;
use tracing_subscriber::filter::Directive;
use tracing_subscriber::layer::SubscriberExt;

#[derive(Parser, Debug)]
struct Arguments {
    #[clap(short, long, default_value = "7777")]
    port: u16,
    #[clap(short, long, default_value = "info")]
    log: Vec<Directive>,
}

#[cli::main("rpc-client")]
async fn main(fb: fbinit::FacebookInit, args: Arguments) -> Result<()> {
    init_logging(args.log.clone());
    let client = get_client(fb, args.port).await? as Arc<_>;
    test(&*client).await?;

    Ok(())
}

async fn get_client(
    fb: fbinit::FacebookInit,
    port: u16,
) -> Result<Arc<dyn RPCConformanceServiceExt<thriftclient::ThriftChannel> + Send + Sync + 'static>>
{
    // TODO: Why aren't we using the SRclient builder here? It has more functionality it seems
    // over the bareclient (specifically the ability to configure the RPC options without specying
    // a serialization protocol)

    let channel = thriftclient::ThriftChannelBuilder::from_sock_addr(
        fb,
        SocketAddr::new(std::net::Ipv6Addr::LOCALHOST.into(), port),
    )?
    .with_secure(false)
    .build_channel()?;

    match channel.protocol_id() {
        fbthrift::ProtocolID::BinaryProtocol => Ok(<dyn RPCConformanceServiceExt<_>>::new(
            fbthrift::BinaryProtocol,
            channel,
        )),
        fbthrift::ProtocolID::CompactProtocol => Ok(<dyn RPCConformanceServiceExt<_>>::new(
            fbthrift::CompactProtocol,
            channel,
        )),
        p => anyhow::bail!("Unsupported protocol: {:?}", p),
    }
}

fn init_logging(directives: Vec<Directive>) {
    let fmt = tracing_subscriber::fmt::Layer::default()
        .with_writer(std::io::stderr)
        .event_format(Glog::default().with_timer(tracing_glog::LocalTime::default()))
        .fmt_fields(GlogFields::default());

    let filter = directives
        .into_iter()
        .fold(EnvFilter::from_default_env(), |filter, directive| {
            filter.add_directive(directive)
        });

    let subscriber = Registry::default().with(filter).with(fmt);
    tracing::subscriber::set_global_default(subscriber).expect("to set global subscriber");
}

// --

use rpc::rpc::ClientInstruction;
use rpc::rpc::ClientTestResult;
use rpc::rpc::InteractionConstructorClientInstruction;
use rpc::rpc::InteractionConstructorClientTestResult;
use rpc::rpc::InteractionFactoryFunctionClientInstruction;
use rpc::rpc::InteractionFactoryFunctionClientTestResult;
use rpc::rpc::InteractionPersistsStateClientInstruction;
use rpc::rpc::InteractionPersistsStateClientTestResult;
use rpc::rpc::InteractionTerminationClientInstruction;
use rpc::rpc::InteractionTerminationClientTestResult;
use rpc::rpc::RequestResponseBasicClientInstruction;
use rpc::rpc::RequestResponseBasicClientTestResult;
use rpc::rpc::RequestResponseDeclaredExceptionClientInstruction;
use rpc::rpc::RequestResponseDeclaredExceptionClientTestResult;
use rpc::rpc::RequestResponseNoArgVoidResponseClientTestResult;
use rpc::rpc::RequestResponseTimeoutClientInstruction;
use rpc::rpc::RequestResponseTimeoutClientTestResult;
use rpc::rpc::RequestResponseUndeclaredExceptionClientInstruction;
use rpc::rpc::RequestResponseUndeclaredExceptionClientTestResult;
use rpc_clients::rpc::BasicInteraction;
use rpc_clients::rpc::errors::r_p_c_conformance_service::RequestResponseDeclaredExceptionError;
use rpc_clients::rpc::errors::r_p_c_conformance_service::RequestResponseTimeoutError;
use rpc_clients::rpc::errors::r_p_c_conformance_service::RequestResponseUndeclaredExceptionError;

async fn test(client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>) -> Result<()> {
    use ClientInstruction::*;

    match &client.getTestCase().await?.clientInstruction {
        requestResponseBasic(i) => request_response_basic(client, i).await,
        requestResponseDeclaredException(i) => request_response_declared_exn(client, i).await,
        requestResponseUndeclaredException(i) => request_response_undeclared_exn(client, i).await,
        requestResponseNoArgVoidResponse(_) => request_response_no_arg_void_response(client).await,
        requestResponseTimeout(i) => request_response_timeout(client, i).await,
        interactionConstructor(i) => interaction_constructor(client, i).await,
        interactionFactoryFunction(i) => interaction_factory_function(client, i).await,
        interactionPersistsState(i) => interaction_persists_state(client, i).await,
        interactionTermination(i) => interaction_termination(client, i).await,
        streamBasic(i) => stream_basic(client, i).await,
        streamChunkTimeout(i) => stream_chunk_timeout(client, i).await,
        streamInitialResponse(i) => stream_initial_response(client, i).await,
        streamCreditTimeout(i) => stream_credit_timeout(client, i).await,
        streamDeclaredException(i) => stream_declared_exception(client, i).await,
        streamUndeclaredException(i) => stream_undeclared_exception(client, i).await,
        streamInitialDeclaredException(i) => stream_initial_declared_exception(client, i).await,
        streamInitialUndeclaredException(i) => stream_initial_undeclared_exception(client, i).await,
        streamInitialTimeout(i) => stream_initial_timeout(client, i).await,
        sinkBasic(i) => sink_basic(client, i).await,
        sinkChunkTimeout(i) => sink_chunk_timeout(client, i).await,
        sinkInitialResponse(i) => sink_initial_response(client, i).await,
        sinkInitialDeclaredException(i) => Err(anyhow!(
            "sinkInitialDeclaredException not implemented: {:?}",
            i
        )),
        sinkDeclaredException(i) => sink_declared_exception(client, i).await,
        sinkServerDeclaredException(i) => Err(anyhow!(
            "sinkServerDeclaredException not implemented: {:?}",
            i
        )),
        sinkUndeclaredException(i) => sink_undeclared_exception(client, i).await,
        UnknownField(i) => Err(anyhow!(format!("not supported: {:?}", i))),
    }
}

// ---

async fn create_interaction(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    initial_sum: &Option<i32>,
) -> Result<Arc<dyn BasicInteraction>> {
    Ok(match initial_sum {
        Some(value) => client.basicInteractionFactoryFunction(*value).await?,
        None => client.createBasicInteraction()? as Arc<_>,
    })
}

// ---

async fn request_response_basic(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &RequestResponseBasicClientInstruction,
) -> Result<()> {
    let test_result =
        ClientTestResult::requestResponseBasic(RequestResponseBasicClientTestResult {
            response: client.requestResponseBasic(&instr.request).await?,
            ..Default::default()
        });
    client.sendTestResult(&test_result).await?;
    Ok(())
}

async fn request_response_declared_exn(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &RequestResponseDeclaredExceptionClientInstruction,
) -> Result<()> {
    match &client
        .requestResponseDeclaredException(&instr.request)
        .await
    {
        Err(RequestResponseDeclaredExceptionError::e(exn)) => {
            let test_result = ClientTestResult::requestResponseDeclaredException(
                RequestResponseDeclaredExceptionClientTestResult {
                    userException: Some(Box::new(exn.clone())),
                    ..Default::default()
                },
            );
            client.sendTestResult(&test_result).await?;
            Ok(())
        }
        r => Err(anyhow!(
            "request_response_declared_exception: unexpected server response:  {:?}",
            r
        )),
    }
}

async fn request_response_undeclared_exn(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &RequestResponseUndeclaredExceptionClientInstruction,
) -> Result<()> {
    match &client
        .requestResponseUndeclaredException(&instr.request)
        .await
    {
        Err(RequestResponseUndeclaredExceptionError::ApplicationException(exn)) => {
            let test_result = ClientTestResult::requestResponseUndeclaredException(
                RequestResponseUndeclaredExceptionClientTestResult {
                    exceptionMessage: exn.message.clone(),
                    ..Default::default()
                },
            );
            client.sendTestResult(&test_result).await?;
            Ok(())
        }
        r => Err(anyhow!(
            "request_response_undeclared_exception: unexpected server response: {:?}",
            r
        )),
    }
}

async fn request_response_no_arg_void_response(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
) -> Result<()> {
    let () = client.requestResponseNoArgVoidResponse().await?;
    let test_result = ClientTestResult::requestResponseNoArgVoidResponse(
        RequestResponseNoArgVoidResponseClientTestResult {
            ..Default::default()
        },
    );
    client.sendTestResult(&test_result).await?;
    Ok(())
}

async fn request_response_timeout(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &RequestResponseTimeoutClientInstruction,
) -> Result<()> {
    let mut opts = rust_types::RpcOptions::default();
    opts.set_timeout_ms(instr.timeoutMs as u32);
    let result = client
        .requestResponseTimeout_with_rpc_opts(&instr.request, opts)
        .await;

    match &result {
        Err(RequestResponseTimeoutError::ThriftError(exn)) => {
            tracing::info!("thrift error: {:?}", exn);
            let test_result =
                ClientTestResult::requestResponseTimeout(RequestResponseTimeoutClientTestResult {
                    timeoutException: exn.to_string().contains("TTransportException"),
                    ..Default::default()
                });
            client.sendTestResult(&test_result).await?;
            Ok(())
        }
        r => Err(anyhow!(
            "request_response_timeout: unexpected server response {:?}",
            r
        )),
    }
}

async fn interaction_constructor(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    _instr: &InteractionConstructorClientInstruction,
) -> Result<()> {
    let i = client.createBasicInteraction()?;
    let () = i.init().await?;
    let test_result =
        ClientTestResult::interactionConstructor(InteractionConstructorClientTestResult {
            ..Default::default()
        });
    client.sendTestResult(&test_result).await?;
    Ok(())
}

async fn interaction_factory_function(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &InteractionFactoryFunctionClientInstruction,
) -> Result<()> {
    let _i = client
        .basicInteractionFactoryFunction(instr.initialSum)
        .await?;
    let test_result =
        ClientTestResult::interactionFactoryFunction(InteractionFactoryFunctionClientTestResult {
            ..Default::default()
        });
    client.sendTestResult(&test_result).await?;
    Ok(())
}

async fn interaction_persists_state(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &InteractionPersistsStateClientInstruction,
) -> Result<()> {
    let i = create_interaction(client, &instr.initialSum).await?;
    let mut test_result: InteractionPersistsStateClientTestResult = Default::default();
    for value in &instr.valuesToAdd {
        test_result.responses.push(i.add(*value).await?);
    }
    client
        .sendTestResult(&ClientTestResult::interactionPersistsState(test_result))
        .await?;
    Ok(())
}

async fn interaction_termination(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &InteractionTerminationClientInstruction,
) -> Result<()> {
    let i = create_interaction(client, &instr.initialSum).await?;
    let () = i.init().await?;
    drop(i);
    let test_result =
        ClientTestResult::interactionTermination(InteractionTerminationClientTestResult {
            ..Default::default()
        });
    client.sendTestResult(&test_result).await?;
    Ok(())
}

async fn stream_basic(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &StreamBasicClientInstruction,
) -> Result<()> {
    let mut opts = rust_types::RpcOptions::default();
    opts.set_chunk_buffer_size(instr.bufferSize as i32);

    let mut stream = client
        .streamBasic_with_rpc_opts(&instr.request, opts)
        .await?;
    let mut test_result = StreamBasicClientTestResult {
        streamPayloads: vec![],
        ..Default::default()
    };
    while let Some(response) = stream.next().await {
        test_result.streamPayloads.push(response?);
    }
    client
        .sendTestResult(&ClientTestResult::streamBasic(test_result))
        .await?;
    Ok(())
}

async fn stream_chunk_timeout(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &StreamChunkTimeoutClientInstruction,
) -> Result<()> {
    let mut opts = rust_types::RpcOptions::default();
    opts.set_chunk_timeout_ms(instr.chunkTimeoutMs as u32);

    let mut stream = client
        .streamChunkTimeout_with_rpc_opts(&instr.request, opts)
        .await?;
    let mut test_result = StreamChunkTimeoutClientTestResult {
        chunkTimeoutException: false,
        streamPayloads: vec![],
        ..Default::default()
    };

    while let Some(response) = stream.next().await {
        match response {
            Ok(r) => {
                test_result.streamPayloads.push(r);
            }
            Err(exn) => {
                test_result.chunkTimeoutException |=
                    exn.to_string().contains("TTransportException");
            }
        }
    }

    client
        .sendTestResult(&ClientTestResult::streamChunkTimeout(test_result))
        .await?;
    Ok(())
}

async fn stream_initial_response(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &StreamInitialResponseClientInstruction,
) -> Result<()> {
    let (initial_response, mut stream) = client.streamInitialResponse(&instr.request).await?;
    let mut test_result = StreamInitialResponseClientTestResult {
        initialResponse: initial_response,
        streamPayloads: vec![],
        ..Default::default()
    };

    while let Some(response) = stream.next().await {
        test_result.streamPayloads.push(response?);
    }

    client
        .sendTestResult(&ClientTestResult::streamInitialResponse(test_result))
        .await?;

    Ok(())
}

async fn stream_credit_timeout(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &StreamCreditTimeoutClientInstruction,
) -> Result<()> {
    let mut stream = client.streamCreditTimeout(&instr.request).await?;
    let mut test_result = StreamCreditTimeoutClientTestResult {
        creditTimeoutException: false,
        ..Default::default()
    };

    while let Some(response) = stream.next().await {
        match response {
            Err(exn) if exn.to_string().contains("TTransportException") => {
                test_result.creditTimeoutException = true;
            }
            _ => {
                // Sleep longer than the stream expiration time so that the server
                // will run out of credit and throw a credit timeout exception
                tokio::time::sleep(Duration::from_millis(instr.creditTimeoutMs as u64)).await;
            }
        }
    }

    client
        .sendTestResult(&ClientTestResult::streamCreditTimeout(test_result))
        .await?;

    Ok(())
}

async fn stream_declared_exception(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &StreamDeclaredExceptionClientInstruction,
) -> Result<()> {
    let mut stream = client.streamDeclaredException(&instr.request).await?;
    let mut test_result = StreamDeclaredExceptionClientTestResult {
        userException: None,
        ..Default::default()
    };
    while let Some(result) = stream.next().await {
        if let Err(StreamDeclaredExceptionStreamError::e(user_error)) = result {
            test_result.userException = Some(Box::new(user_error));
        };
    }
    client
        .sendTestResult(&ClientTestResult::streamDeclaredException(test_result))
        .await?;
    Ok(())
}

async fn stream_undeclared_exception(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &StreamUndeclaredExceptionClientInstruction,
) -> Result<()> {
    let mut stream = client.streamUndeclaredException(&instr.request).await?;
    let mut test_result = StreamUndeclaredExceptionClientTestResult {
        exceptionMessage: "".to_string(),
        ..Default::default()
    };
    while let Some(result) = stream.next().await {
        if let Err(err) = result {
            test_result.exceptionMessage = err.to_string();
        };
    }
    client
        .sendTestResult(&ClientTestResult::streamUndeclaredException(test_result))
        .await?;
    Ok(())
}

async fn stream_initial_declared_exception(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &StreamInitialDeclaredExceptionClientInstruction,
) -> Result<()> {
    let result = client.streamInitialDeclaredException(&instr.request).await;
    let mut test_result = StreamInitialDeclaredExceptionClientTestResult {
        userException: None,
        ..Default::default()
    };
    if let Err(StreamInitialDeclaredExceptionError::e(user_error)) = result {
        test_result.userException = Some(Box::new(user_error));
    };
    client
        .sendTestResult(&ClientTestResult::streamInitialDeclaredException(
            test_result,
        ))
        .await?;
    Ok(())
}

async fn stream_initial_undeclared_exception(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &StreamInitialUndeclaredExceptionClientInstruction,
) -> Result<()> {
    let result = client
        .streamInitialUndeclaredException(&instr.request)
        .await;
    let mut test_result = StreamInitialUndeclaredExceptionClientTestResult {
        exceptionMessage: "".to_string(),
        ..Default::default()
    };
    if let Err(err) = result {
        test_result.exceptionMessage = err.to_string();
    };
    client
        .sendTestResult(&ClientTestResult::streamInitialUndeclaredException(
            test_result,
        ))
        .await?;
    Ok(())
}

async fn stream_initial_timeout(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &StreamInitialTimeoutClientInstruction,
) -> Result<()> {
    let mut opts = rust_types::RpcOptions::default();
    opts.set_timeout_ms(instr.timeoutMs as u32);
    let result = client
        .streamInitialTimeout_with_rpc_opts(&instr.request, opts)
        .await;
    let mut test_result = StreamInitialTimeoutClientTestResult {
        timeoutException: false,
        ..Default::default()
    };
    if let Err(_err) = result {
        test_result.timeoutException = true;
    }
    client
        .sendTestResult(&ClientTestResult::streamInitialTimeout(test_result))
        .await?;
    Ok(())
}

async fn sink_basic(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &SinkBasicClientInstruction,
) -> Result<()> {
    let sink_result = client.sinkBasic(&instr.request).await?;
    let final_response = (sink_result.sink)(
        futures::stream::iter(instr.sinkPayloads.clone())
            .map(Ok)
            .boxed(),
    )
    .await
    .map_err(|err| anyhow!("sink_basic: sink failed: {:?}", err))?;

    let test_result = SinkBasicClientTestResult {
        finalResponse: final_response,
        ..Default::default()
    };
    client
        .sendTestResult(&ClientTestResult::sinkBasic(test_result))
        .await?;

    Ok(())
}

async fn sink_chunk_timeout(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &SinkChunkTimeoutClientInstruction,
) -> Result<()> {
    let sink_result = client.sinkChunkTimeout(&instr.request).await?;
    let final_response = (sink_result.sink)(
        futures::stream::iter(instr.sinkPayloads.clone())
            .map(Ok)
            .boxed(),
    )
    .await;

    let chunk_timeout = match final_response {
        Ok(_) => false,
        Err(SinkChunkTimeoutSinkFinalError::ApplicationException(aexn))
            if aexn.type_ == fbthrift::ApplicationExceptionErrorCode::Timeout =>
        {
            true
        }
        _ => false,
    };

    let test_result = SinkChunkTimeoutClientTestResult {
        chunkTimeoutException: chunk_timeout,
        ..Default::default()
    };
    client
        .sendTestResult(&ClientTestResult::sinkChunkTimeout(test_result))
        .await?;

    Ok(())
}

async fn sink_initial_response(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &SinkInitialResponseClientInstruction,
) -> Result<()> {
    let sink_result = client.sinkInitialResponse(&instr.request).await?;
    let final_response = (sink_result.sink)(
        futures::stream::iter(instr.sinkPayloads.clone())
            .map(Ok)
            .boxed(),
    )
    .await?;

    let test_result = SinkInitialResponseClientTestResult {
        finalResponse: final_response,
        initialResponse: sink_result.initial_response,
        ..Default::default()
    };
    client
        .sendTestResult(&ClientTestResult::sinkInitialResponse(test_result))
        .await?;

    Ok(())
}

async fn sink_declared_exception(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &SinkDeclaredExceptionClientInstruction,
) -> Result<()> {
    let sink_result = client.sinkDeclaredException(&instr.request).await?;
    let final_response = if let Some(ue) = &instr.userException {
        (sink_result.sink)(
            futures::stream::iter([ue.clone()])
                .map(|ex| Err(SinkDeclaredExceptionSinkError::e(ex)))
                .boxed(),
        )
        .await
    } else {
        (sink_result.sink)(futures::stream::empty().boxed()).await
    };

    if instr.userException.is_some() {
        assert!(final_response.is_err());
    } else {
        assert!(final_response.is_ok());
    }

    let test_result = SinkDeclaredExceptionClientTestResult {
        sinkThrew: instr.userException.is_some(),
        ..Default::default()
    };
    client
        .sendTestResult(&ClientTestResult::sinkDeclaredException(test_result))
        .await?;

    Ok(())
}

async fn sink_undeclared_exception(
    client: &dyn RPCConformanceServiceExt<thriftclient::ThriftChannel>,
    instr: &SinkUndeclaredExceptionClientInstruction,
) -> Result<()> {
    let sink_result = client.sinkUndeclaredException(&instr.request).await?;
    let final_response = if let Some(ue) = &instr.exceptionMessage {
        (sink_result.sink)(
            futures::stream::iter([ue.clone()])
                .map(|ex| {
                    Err(SinkUndeclaredExceptionSinkError::ApplicationException(
                        fbthrift::ApplicationException::new(
                            fbthrift::ApplicationExceptionErrorCode::Unknown,
                            ex,
                        ),
                    ))
                })
                .boxed(),
        )
        .await
    } else {
        (sink_result.sink)(futures::stream::empty().boxed()).await
    };

    if instr.exceptionMessage.is_some() {
        assert!(final_response.is_err());
    } else {
        assert!(final_response.is_ok());
    }

    let test_result = SinkUndeclaredExceptionClientTestResult {
        sinkThrew: instr.exceptionMessage.is_some(),
        ..Default::default()
    };
    client
        .sendTestResult(&ClientTestResult::sinkUndeclaredException(test_result))
        .await?;

    Ok(())
}
