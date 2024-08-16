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

use anyhow::anyhow;
use anyhow::Result;
use clap::Parser;
use rpc_clients::rpc::make_RPCConformanceService;
use rpc_clients::rpc::RPCConformanceService;
use tracing_glog::Glog;
use tracing_glog::GlogFields;
use tracing_subscriber::filter::Directive;
use tracing_subscriber::layer::SubscriberExt;
use tracing_subscriber::EnvFilter;
use tracing_subscriber::Registry;

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
) -> Result<Arc<dyn RPCConformanceService + Send + Sync + 'static>> {
    thriftclient::ThriftChannelBuilder::from_sock_addr(
        fb,
        SocketAddr::new(std::net::Ipv6Addr::LOCALHOST.into(), port),
    )?
    .with_secure(false)
    .build_client(make_RPCConformanceService) // patternlint-disable-line rust-sr-client-creation-method
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
use rpc::rpc::RequestResponseBasicClientInstruction;
use rpc::rpc::RequestResponseBasicClientTestResult;
use rpc::rpc::RequestResponseDeclaredExceptionClientInstruction;
use rpc::rpc::RequestResponseDeclaredExceptionClientTestResult;
use rpc::rpc::RequestResponseNoArgVoidResponseClientTestResult;
use rpc::rpc::RequestResponseTimeoutClientInstruction;
use rpc::rpc::RequestResponseTimeoutClientTestResult;
use rpc::rpc::RequestResponseUndeclaredExceptionClientInstruction;
use rpc::rpc::RequestResponseUndeclaredExceptionClientTestResult;
use rpc_clients::rpc::errors::r_p_c_conformance_service::RequestResponseDeclaredExceptionError;
use rpc_clients::rpc::errors::r_p_c_conformance_service::RequestResponseTimeoutError;
use rpc_clients::rpc::errors::r_p_c_conformance_service::RequestResponseUndeclaredExceptionError;

async fn test(client: &dyn RPCConformanceService) -> Result<()> {
    match &client.getTestCase().await?.clientInstruction {
        ClientInstruction::requestResponseBasic(instr) => {
            request_response_basic(client, instr).await
        }
        ClientInstruction::requestResponseDeclaredException(instr) => {
            request_response_declared_exception(client, instr).await
        }
        ClientInstruction::requestResponseUndeclaredException(instr) => {
            request_response_undeclared_exception(client, instr).await
        }
        ClientInstruction::requestResponseNoArgVoidResponse(_) => {
            request_response_no_arg_void_response(client).await
        }
        ClientInstruction::requestResponseTimeout(instr) => {
            request_response_timeout(client, instr).await
        }
        ClientInstruction::interactionConstructor(_) => not_implemented(),
        ClientInstruction::interactionFactoryFunction(_) => not_implemented(),
        ClientInstruction::interactionPersistsState(_) => not_implemented(),
        ClientInstruction::interactionTermination(_) => not_implemented(),
        ClientInstruction::streamBasic(_) => not_implemented(),
        ClientInstruction::streamChunkTimeout(_) => not_implemented(),
        ClientInstruction::streamInitialResponse(_) => not_implemented(),
        ClientInstruction::streamCreditTimeout(_) => not_implemented(),
        ClientInstruction::streamDeclaredException(_) => not_implemented(),
        ClientInstruction::streamUndeclaredException(_) => not_implemented(),
        ClientInstruction::streamInitialDeclaredException(_) => not_implemented(),
        ClientInstruction::streamInitialUndeclaredException(_) => not_implemented(),
        ClientInstruction::streamInitialTimeout(_) => not_implemented(),
        i => Err(anyhow!(format!("not supported: {:?}", i))),
    }
}

fn not_implemented() -> Result<()> {
    Err(anyhow!("not implemented"))
}

async fn request_response_basic(
    client: &dyn RPCConformanceService,
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

async fn request_response_declared_exception(
    client: &dyn RPCConformanceService,
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

async fn request_response_undeclared_exception(
    client: &dyn RPCConformanceService,
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

async fn request_response_no_arg_void_response(client: &dyn RPCConformanceService) -> Result<()> {
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
    client: &dyn RPCConformanceService,
    instr: &RequestResponseTimeoutClientInstruction,
) -> Result<()> {
    match &client.requestResponseTimeout(&instr.request).await {
        // TODO: This is wrong! On timeout we are receiving a default
        // constructed response. The conformant behavior is as per the next
        // case, a `ThriftError` containing a `TTransportException`.
        // Ok(r) if r.data.as_str() == "" && r.num == None => {
        //     let test_result =
        //         ClientTestResult::requestResponseTimeout(RequestResponseTimeoutClientTestResult {
        //             timeoutException: true,
        //             ..Default::default()
        //         });
        //     client.sendTestResult(&test_result).await?;
        //     Ok(())
        // }
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
