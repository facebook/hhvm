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

use anyhow::Result;
use fbthrift_test_if::TestException;
use fbthrift_test_if::TestExceptionMsgOverride;
use fbthrift_test_if::TestExceptionMsgOverrideOptional;

#[test]
fn test_should_implement_error() -> Result<()> {
    let err = TestException {
        message: "invalid grape peeler".into(),
        ..Default::default()
    };
    assert!(err.source().is_none());

    Ok(())
}

#[test]
fn test_should_print_debug_by_default() -> Result<()> {
    let err = TestException {
        message: "invalid grape peeler".into(),
        ..Default::default()
    };
    assert_eq!(format!("{}", err), format!("{:?}", err),);

    Ok(())
}

#[test]
fn test_should_use_message_field_override() -> Result<()> {
    let err = TestExceptionMsgOverride {
        message: "invalid grape peeler".into(),
        ..Default::default()
    };
    assert_eq!(
        format!("TestExceptionMsgOverride: {}", err.message),
        format!("{}", err)
    );

    Ok(())
}

#[test]
fn test_should_use_message_field_override_optional() -> Result<()> {
    let err = TestExceptionMsgOverrideOptional {
        message: Some("invalid grape peeler".into()),
        ..Default::default()
    };
    assert_eq!(
        format!(
            "TestExceptionMsgOverrideOptional: Some(\"{}\")",
            err.message.as_ref().unwrap(),
        ),
        format!("{}", err)
    );

    let err = TestExceptionMsgOverrideOptional {
        message: None,
        ..Default::default()
    };
    assert_eq!(
        format!("TestExceptionMsgOverrideOptional: None"),
        format!("{}", err)
    );

    Ok(())
}
