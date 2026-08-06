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

use std::error::Error;

use crate::PipelineError;

#[test]
fn pipeline_error_preserves_public_contract() {
    let error = PipelineError::new("deferred exception");
    assert_eq!(error.message(), "deferred exception");
    assert_eq!(error.to_string(), "deferred exception");
    assert_eq!(error.clone(), error);
    assert!(format!("{error:?}").contains("deferred exception"));

    let as_error: &dyn Error = &error;
    assert_eq!(as_error.to_string(), "deferred exception");
}

#[test]
fn pipeline_error_preserves_message_boundaries() {
    assert_eq!(PipelineError::new("").message(), "");
    assert_eq!(
        PipelineError::new("before\0after").message(),
        "before\0after"
    );

    let long = "x".repeat(4096);
    assert_eq!(PipelineError::new(long.clone()).message(), long);
}
