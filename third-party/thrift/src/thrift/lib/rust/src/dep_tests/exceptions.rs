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
use fbthrift::ExceptionBlame;
use fbthrift::ExceptionInfo;
use fbthrift::ExceptionKind;
use fbthrift::ExceptionSafety;
use fbthrift_test_if::TestClientException;
use fbthrift_test_if::TestException;
use fbthrift_test_if::TestExceptionMsgOverride;
use fbthrift_test_if::TestExceptionMsgOverrideOptional;
use fbthrift_test_if::TestUnclassifiedException;

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

// Validates that the IDL exception classification qualifiers (client/server,
// transient/stateful/permanent, safe) are propagated into the generated
// `ExceptionInfo::exn_blame/exn_kind/exn_safety` accessors.

#[test]
fn test_exception_classification_server() {
    // `safe stateful server exception TestException`
    let err = TestException {
        message: "boom".into(),
        ..Default::default()
    };
    assert_eq!(err.exn_blame(), ExceptionBlame::Server);
    assert_eq!(err.exn_kind(), ExceptionKind::Stateful);
    assert_eq!(err.exn_safety(), ExceptionSafety::Safe);
    assert!(err.exn_is_declared());
}

#[test]
fn test_exception_classification_client() {
    // `permanent client exception TestClientException`
    let err = TestClientException {
        message: "bad request".into(),
        ..Default::default()
    };
    assert_eq!(err.exn_blame(), ExceptionBlame::Client);
    assert_eq!(err.exn_kind(), ExceptionKind::Permanent);
    assert_eq!(err.exn_safety(), ExceptionSafety::Unspecified);
    assert!(err.exn_is_declared());
}

#[test]
fn test_exception_classification_unspecified_by_default() {
    // `exception TestUnclassifiedException` (no classification qualifiers)
    let err = TestUnclassifiedException {
        message: "unknown".into(),
        ..Default::default()
    };
    assert_eq!(err.exn_blame(), ExceptionBlame::Unspecified);
    assert_eq!(err.exn_kind(), ExceptionKind::Unspecified);
    assert_eq!(err.exn_safety(), ExceptionSafety::Unspecified);
    assert!(err.exn_is_declared());
}

// Validates that the per-method server `*Exn` enum forwards the classification
// of the active declared-exception variant, and that the ApplicationException
// variant falls back to the `Unspecified` trait defaults.
#[test]
fn test_method_exn_delegates_classification() {
    use fbthrift_test_if_services::errors::test_service::Method1Exn;

    let exn = Method1Exn::ex(TestException {
        message: "boom".into(),
        ..Default::default()
    });
    assert_eq!(exn.exn_blame(), ExceptionBlame::Server);
    assert_eq!(exn.exn_kind(), ExceptionKind::Stateful);
    assert_eq!(exn.exn_safety(), ExceptionSafety::Safe);
    assert!(exn.exn_is_declared());

    let aexn = Method1Exn::ApplicationException(fbthrift::ApplicationException::default());
    assert_eq!(aexn.exn_blame(), ExceptionBlame::Unspecified);
    assert_eq!(aexn.exn_kind(), ExceptionKind::Unspecified);
    assert_eq!(aexn.exn_safety(), ExceptionSafety::Unspecified);
    assert!(!aexn.exn_is_declared());
}

// Exercises {Method}Exn delegation for the client-blame and unspecified cases,
// via TestClassifiedService which throws the corresponding declared exceptions.
#[test]
fn test_method_exn_delegates_client_and_unspecified() {
    use fbthrift_test_if_services::errors::test_classified_service::ClientMethodExn;
    use fbthrift_test_if_services::errors::test_classified_service::UnclassifiedMethodExn;

    let c = ClientMethodExn::ex(TestClientException {
        message: "bad request".into(),
        ..Default::default()
    });
    assert_eq!(c.exn_blame(), ExceptionBlame::Client);
    assert_eq!(c.exn_kind(), ExceptionKind::Permanent);
    assert!(c.exn_is_declared());

    let u = UnclassifiedMethodExn::ex(TestUnclassifiedException {
        message: "unknown".into(),
        ..Default::default()
    });
    assert_eq!(u.exn_blame(), ExceptionBlame::Unspecified);
    assert_eq!(u.exn_kind(), ExceptionKind::Unspecified);
    assert_eq!(u.exn_safety(), ExceptionSafety::Unspecified);
    assert!(u.exn_is_declared());
}
