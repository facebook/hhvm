// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::server::Service;
use failure_ext::Result;
use serde::{Deserialize, Serialize};

// Implementation of the decl service.
#[derive(Debug, Default)]
pub struct DeclService;

impl Service for DeclService {
    type Request = DeclRequest;
    type Response = DeclResponse;

    fn serve(&self, request: &Self::Request) -> Result<Self::Response> {
        match request {
            DeclRequest::StartFile => unimplemented!(),
            DeclRequest::GetDecl => unimplemented!(),
            DeclRequest::EndFile => unimplemented!(),
        }
    }
}

#[derive(Debug, Deserialize)]
#[serde(tag = "type")]
pub enum DeclRequest {
    StartFile,
    GetDecl,
    EndFile,
}

#[derive(Debug, Serialize)]
#[serde(tag = "type")]
pub enum DeclResponse {
    StartFile,
    GetDecl,
    EndFile,
}
