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

use std::sync::Arc;

use anyhow::Result;
use clap::Parser;
use rpc_clients::rpc::make_RPCConformanceService;
use rpc_clients::rpc::RPCConformanceService;
use srclient::SRChannelBuilder;
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
    let _client = get_client(fb, args.port).await?;

    Ok(())
}

async fn get_client(
    fb: fbinit::FacebookInit,
    port: u16,
) -> Result<Arc<dyn RPCConformanceService + Send + Sync + 'static>> {
    SRChannelBuilder::from_service_name(fb, "")?
        .with_client_params(srclient::ClientParams::new().with_localhost_only(port))
        // patternlint-disable-next-line rust-sr-client-creation-method
        .build_client(make_RPCConformanceService)
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
