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

use conformance::services::conformance_service::RoundTripExn;
use fbthrift::ApplicationException;
use fbthrift::ApplicationExceptionErrorCode;
use protocol::StandardProtocol;
use serialization::RoundTripRequest;

pub(crate) fn internal_error<E: std::fmt::Display>(err: E) -> ApplicationException {
    ApplicationException::new(
        ApplicationExceptionErrorCode::InternalError,
        format!("{:#}", err),
    )
}

pub(crate) fn unimplemented_method(service: &str, name: &str) -> ApplicationException {
    ApplicationException::unimplemented_method(service, name)
}

pub(crate) fn get_protocol(request: &RoundTripRequest) -> Result<StandardProtocol, RoundTripExn> {
    Ok(
        match (
            request.targetProtocol.as_ref(),
            request.value.protocol.as_ref(),
        ) {
            (Some(s), _) => s.standard,
            (None, Some(&p)) => p,
            (None, None) => StandardProtocol::Compact,
        },
    )
}
