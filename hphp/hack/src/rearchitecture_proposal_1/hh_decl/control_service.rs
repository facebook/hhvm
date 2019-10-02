// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::server::Service;
use failure_ext::Result;
use serde::{Deserialize, Serialize};

/// Implementation of the control service.
#[derive(Debug, Default)]
pub struct ControlService;

impl Service for ControlService {
    type Request = ControlRequest;
    type Response = ControlResponse;

    fn serve(&self, request: &Self::Request) -> Result<Self::Response> {
        match request {
            ControlRequest::Status => Ok(ControlResponse::Status {
                message: "initializing".to_string(),
            }),
            ControlRequest::UpdateNamingTable { .. } => unimplemented!(),
        }
    }
}

#[derive(Debug, Deserialize)]
#[serde(tag = "type")]
pub enum ControlRequest {
    Status,
    UpdateNamingTable { files: Vec<String> },
}

#[derive(Debug, Serialize)]
#[serde(tag = "type")]
pub enum ControlResponse {
    Status { message: String },
    UpdateNamingTable,
}
