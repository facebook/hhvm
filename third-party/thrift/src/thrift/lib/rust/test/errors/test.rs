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

use fbthrift::application_exception::ApplicationException;
use fbthrift::application_exception::ApplicationExceptionErrorCode;
use k9::snapshot;
use thrift_test::types::ErrorCode;
use thrift_test::types::InternalError;
use thrift_test::types::RetryableError;
use thrift_test_clients::errors::DoFooError;
use thrift_test_clients::errors::DoFooStreamStreamError;
use thrift_test_clients::errors::DoNoThrowError;

fn make_ie() -> DoFooError {
    DoFooError::ie(InternalError {
        code: ErrorCode::INVALID_VALUE,
        message: "got some invalid value".to_string(),
        ..Default::default()
    })
}

#[test]
fn test_internal_error() {
    let ie = make_ie();

    snapshot!(
        format!("{}", ie),
        "TestService::do_foo failed with ie(InternalError)"
    );
    snapshot!(
        format!("{:#}", ie),
        "TestService::do_foo failed with variant `ie`: InternalError: got some invalid value"
    );
    snapshot!(
        format!("{:?}", ie),
        "ie(InternalError { code: ErrorCode::INVALID_VALUE, message: \"got some invalid value\" })"
    );
    snapshot!(
        format!("{:#?}", ie),
        r#"
ie(
    InternalError {
        code: ErrorCode::INVALID_VALUE,
        message: "got some invalid value",
    },
)
"#
    );

    snapshot!(
        format!("{}", Into::<anyhow::Error>::into(make_ie())),
        "TestService::do_foo failed with ie(InternalError)"
    );
    snapshot!(
        format!("{:#}", Into::<anyhow::Error>::into(make_ie())),
        "TestService::do_foo failed with ie(InternalError): InternalError: got some invalid value"
    );
    snapshot!(
        format!("{:?}", Into::<anyhow::Error>::into(make_ie())),
        "
TestService::do_foo failed with ie(InternalError)

Caused by:
    InternalError: got some invalid value
"
    );
    snapshot!(
        format!("{:#?}", Into::<anyhow::Error>::into(make_ie())),
        r#"
ie(
    InternalError {
        code: ErrorCode::INVALID_VALUE,
        message: "got some invalid value",
    },
)
"#
    );

    assert_eq!(
        ie.source()
            .unwrap()
            .downcast_ref::<InternalError>()
            .unwrap()
            .message,
        "got some invalid value"
    );
}

fn make_ie_stream() -> DoFooStreamStreamError {
    DoFooStreamStreamError::ie(InternalError {
        code: ErrorCode::INVALID_VALUE,
        message: "got some invalid value".to_string(),
        ..Default::default()
    })
}

#[test]
fn test_internal_error_stream() {
    let ie = make_ie_stream();

    snapshot!(
        format!("{}", ie),
        "TestService::do_foo_stream failed with ie(InternalError)"
    );
    snapshot!(
        format!("{:#}", ie),
        "TestService::do_foo_stream failed with variant `ie`: InternalError: got some invalid value"
    );
    snapshot!(
        format!("{:?}", ie),
        "ie(InternalError { code: ErrorCode::INVALID_VALUE, message: \"got some invalid value\" })"
    );
    snapshot!(
        format!("{:#?}", ie),
        "
ie(
    InternalError {
        code: ErrorCode::INVALID_VALUE,
        message: \"got some invalid value\",
    },
)
"
    );

    snapshot!(
        format!("{}", Into::<anyhow::Error>::into(make_ie_stream())),
        "TestService::do_foo_stream failed with ie(InternalError)"
    );
    snapshot!(
        format!("{:#}", Into::<anyhow::Error>::into(make_ie_stream())),
        "TestService::do_foo_stream failed with ie(InternalError): InternalError: got some invalid value"
    );
    snapshot!(
        format!("{:?}", Into::<anyhow::Error>::into(make_ie_stream())),
        "
TestService::do_foo_stream failed with ie(InternalError)

Caused by:
    InternalError: got some invalid value
"
    );
    snapshot!(
        format!("{:#?}", Into::<anyhow::Error>::into(make_ie_stream())),
        "
ie(
    InternalError {
        code: ErrorCode::INVALID_VALUE,
        message: \"got some invalid value\",
    },
)
"
    );

    assert_eq!(
        ie.source()
            .unwrap()
            .downcast_ref::<InternalError>()
            .unwrap()
            .message,
        "got some invalid value"
    );
}

fn make_re() -> DoFooError {
    DoFooError::re(RetryableError {
        code: ErrorCode::INVALID_VALUE,
        ..Default::default()
    })
}

#[test]
fn test_retryable_error() {
    let re = make_re();

    snapshot!(
        format!("{}", re),
        "TestService::do_foo failed with re(RetryableError)"
    );
    snapshot!(
        format!("{:#}", re),
        "TestService::do_foo failed with variant `re`: RetryableError { code: ErrorCode::INVALID_VALUE }"
    );
    snapshot!(
        format!("{:?}", re),
        "re(RetryableError { code: ErrorCode::INVALID_VALUE })"
    );
    snapshot!(
        format!("{:#?}", re),
        "
re(
    RetryableError {
        code: ErrorCode::INVALID_VALUE,
    },
)
"
    );

    snapshot!(
        format!("{}", Into::<anyhow::Error>::into(make_re())),
        "TestService::do_foo failed with re(RetryableError)"
    );
    snapshot!(
        format!("{:#}", Into::<anyhow::Error>::into(make_re())),
        "TestService::do_foo failed with re(RetryableError): RetryableError { code: ErrorCode::INVALID_VALUE }"
    );
    snapshot!(
        format!("{:?}", Into::<anyhow::Error>::into(make_re())),
        "
TestService::do_foo failed with re(RetryableError)

Caused by:
    RetryableError { code: ErrorCode::INVALID_VALUE }
"
    );
    snapshot!(
        format!("{:#?}", Into::<anyhow::Error>::into(make_re())),
        "
re(
    RetryableError {
        code: ErrorCode::INVALID_VALUE,
    },
)
"
    );

    assert_eq!(
        re.source()
            .unwrap()
            .downcast_ref::<RetryableError>()
            .unwrap()
            .code,
        ErrorCode::INVALID_VALUE
    );
}

fn make_re_stream() -> DoFooStreamStreamError {
    DoFooStreamStreamError::re(RetryableError {
        code: ErrorCode::INVALID_VALUE,
        ..Default::default()
    })
}

#[test]
fn test_retryable_error_stream() {
    let re = make_re_stream();

    snapshot!(
        format!("{}", re),
        "TestService::do_foo_stream failed with re(RetryableError)"
    );
    snapshot!(
        format!("{:#}", re),
        "TestService::do_foo_stream failed with variant `re`: RetryableError { code: ErrorCode::INVALID_VALUE }"
    );
    snapshot!(
        format!("{:?}", re),
        "re(RetryableError { code: ErrorCode::INVALID_VALUE })"
    );
    snapshot!(
        format!("{:#?}", re),
        "
re(
    RetryableError {
        code: ErrorCode::INVALID_VALUE,
    },
)
"
    );

    snapshot!(
        format!("{}", Into::<anyhow::Error>::into(make_re_stream())),
        "TestService::do_foo_stream failed with re(RetryableError)"
    );
    snapshot!(
        format!("{:#}", Into::<anyhow::Error>::into(make_re_stream())),
        "TestService::do_foo_stream failed with re(RetryableError): RetryableError { code: ErrorCode::INVALID_VALUE }"
    );
    snapshot!(
        format!("{:?}", Into::<anyhow::Error>::into(make_re_stream())),
        "
TestService::do_foo_stream failed with re(RetryableError)

Caused by:
    RetryableError { code: ErrorCode::INVALID_VALUE }
"
    );
    snapshot!(
        format!("{:#?}", Into::<anyhow::Error>::into(make_re_stream())),
        "
re(
    RetryableError {
        code: ErrorCode::INVALID_VALUE,
    },
)
"
    );

    assert_eq!(
        re.source()
            .unwrap()
            .downcast_ref::<RetryableError>()
            .unwrap()
            .code,
        ErrorCode::INVALID_VALUE
    );
}

fn make_ae() -> DoFooError {
    DoFooError::ApplicationException(ApplicationException {
        message: "some injected failure".to_string(),
        type_: ApplicationExceptionErrorCode::InjectedFailure,
        ..Default::default()
    })
}

#[test]
fn test_application_exception() {
    let ae = make_ae();

    snapshot!(
        format!("{}", ae),
        "TestService::do_foo failed with ApplicationException"
    );
    snapshot!(
        format!("{:#}", ae),
        "TestService::do_foo failed with ApplicationException: InjectedFailure: some injected failure"
    );
    snapshot!(
        format!("{:?}", ae),
        r#"ApplicationException(ApplicationException { message: "some injected failure", type_: InjectedFailure })"#
    );
    snapshot!(
        format!("{:#?}", ae),
        r#"
ApplicationException(
    ApplicationException {
        message: "some injected failure",
        type_: InjectedFailure,
    },
)
"#
    );

    snapshot!(
        format!("{}", Into::<anyhow::Error>::into(make_ae())),
        "TestService::do_foo failed with ApplicationException"
    );
    snapshot!(
        format!("{:#}", Into::<anyhow::Error>::into(make_ae())),
        "TestService::do_foo failed with ApplicationException: InjectedFailure: some injected failure"
    );
    snapshot!(
        format!("{:?}", Into::<anyhow::Error>::into(make_ae())),
        "
TestService::do_foo failed with ApplicationException

Caused by:
    InjectedFailure: some injected failure
"
    );
    snapshot!(
        format!("{:#?}", Into::<anyhow::Error>::into(make_ae())),
        r#"
ApplicationException(
    ApplicationException {
        message: "some injected failure",
        type_: InjectedFailure,
    },
)
"#
    );

    assert_eq!(
        ae.source()
            .unwrap()
            .downcast_ref::<ApplicationException>()
            .unwrap()
            .type_,
        ApplicationExceptionErrorCode::InjectedFailure
    );
}

fn make_ae_stream() -> DoFooStreamStreamError {
    DoFooStreamStreamError::ApplicationException(ApplicationException {
        message: "some injected failure".to_string(),
        type_: ApplicationExceptionErrorCode::InjectedFailure,
        ..Default::default()
    })
}

#[test]
fn test_application_exception_stream() {
    let ae = make_ae_stream();

    snapshot!(
        format!("{}", ae),
        "TestService::do_foo_stream failed with ApplicationException"
    );
    snapshot!(
        format!("{:#}", ae),
        "TestService::do_foo_stream failed with ApplicationException: InjectedFailure: some injected failure"
    );
    snapshot!(
        format!("{:?}", ae),
        r#"ApplicationException(ApplicationException { message: "some injected failure", type_: InjectedFailure })"#
    );
    snapshot!(
        format!("{:#?}", ae),
        r#"
ApplicationException(
    ApplicationException {
        message: "some injected failure",
        type_: InjectedFailure,
    },
)
"#
    );

    snapshot!(
        format!("{}", Into::<anyhow::Error>::into(make_ae_stream())),
        "TestService::do_foo_stream failed with ApplicationException"
    );
    snapshot!(
        format!("{:#}", Into::<anyhow::Error>::into(make_ae_stream())),
        "TestService::do_foo_stream failed with ApplicationException: InjectedFailure: some injected failure"
    );
    snapshot!(
        format!("{:?}", Into::<anyhow::Error>::into(make_ae_stream())),
        "
TestService::do_foo_stream failed with ApplicationException

Caused by:
    InjectedFailure: some injected failure
"
    );
    snapshot!(
        format!("{:#?}", Into::<anyhow::Error>::into(make_ae_stream())),
        r#"
ApplicationException(
    ApplicationException {
        message: "some injected failure",
        type_: InjectedFailure,
    },
)
"#
    );

    assert_eq!(
        ae.source()
            .unwrap()
            .downcast_ref::<ApplicationException>()
            .unwrap()
            .type_,
        ApplicationExceptionErrorCode::InjectedFailure
    );
}

fn make_te() -> DoFooError {
    DoFooError::ThriftError(anyhow::anyhow!("foo").context("bar").context("baz"))
}

#[test]
fn test_thrift_error() {
    let te = make_te();

    snapshot!(
        format!("{}", te),
        "TestService::do_foo failed with ThriftError"
    );
    snapshot!(
        format!("{:#}", te),
        "TestService::do_foo failed with ThriftError: baz: bar: foo"
    );
    snapshot!(
        format!("{:?}", te),
        "
ThriftError(baz

Caused by:
    0: bar
    1: foo)
"
    );
    snapshot!(
        format!("{:#?}", te),
        r#"
ThriftError(
    Error {
        context: "baz",
        source: Error {
            context: "bar",
            source: "foo",
        },
    },
)
"#
    );

    snapshot!(
        format!("{}", Into::<anyhow::Error>::into(make_te())),
        "TestService::do_foo failed with ThriftError"
    );
    snapshot!(
        format!("{:#}", Into::<anyhow::Error>::into(make_te())),
        "TestService::do_foo failed with ThriftError: baz: bar: foo"
    );
    snapshot!(
        format!("{:?}", Into::<anyhow::Error>::into(make_te())),
        "
TestService::do_foo failed with ThriftError

Caused by:
    0: baz
    1: bar
    2: foo
"
    );
    snapshot!(
        format!("{:#?}", Into::<anyhow::Error>::into(make_te())),
        r#"
ThriftError(
    Error {
        context: "baz",
        source: Error {
            context: "bar",
            source: "foo",
        },
    },
)
"#
    );
}

fn make_te_stream() -> DoFooStreamStreamError {
    DoFooStreamStreamError::ThriftError(anyhow::anyhow!("foo").context("bar").context("baz"))
}

#[test]
fn test_thrift_error_stream() {
    let te = make_te_stream();

    snapshot!(
        format!("{}", te),
        "TestService::do_foo_stream failed with ThriftError"
    );
    snapshot!(
        format!("{:#}", te),
        "TestService::do_foo_stream failed with ThriftError: baz: bar: foo"
    );
    snapshot!(
        format!("{:?}", te),
        "
ThriftError(baz

Caused by:
    0: bar
    1: foo)
"
    );
    snapshot!(
        format!("{:#?}", te),
        r#"
ThriftError(
    Error {
        context: "baz",
        source: Error {
            context: "bar",
            source: "foo",
        },
    },
)
"#
    );

    snapshot!(
        format!("{}", Into::<anyhow::Error>::into(make_te_stream())),
        "TestService::do_foo_stream failed with ThriftError"
    );
    snapshot!(
        format!("{:#}", Into::<anyhow::Error>::into(make_te_stream())),
        "TestService::do_foo_stream failed with ThriftError: baz: bar: foo"
    );
    snapshot!(
        format!("{:?}", Into::<anyhow::Error>::into(make_te_stream())),
        "
TestService::do_foo_stream failed with ThriftError

Caused by:
    0: baz
    1: bar
    2: foo
"
    );
    snapshot!(
        format!("{:#?}", Into::<anyhow::Error>::into(make_te_stream())),
        r#"
ThriftError(
    Error {
        context: "baz",
        source: Error {
            context: "bar",
            source: "foo",
        },
    },
)
"#
    );
}

fn make_no_throw_error() -> DoNoThrowError {
    DoNoThrowError::ThriftError(
        anyhow::anyhow!("TransportError: failed to connect to host").context("some client failed"),
    )
}

#[test]
fn test_no_throw_error() {
    let no_throw_err = make_no_throw_error();

    snapshot!(format!("{}", no_throw_err), "some client failed");
    snapshot!(
        format!("{:#}", no_throw_err),
        "some client failed: TransportError: failed to connect to host"
    );
    snapshot!(
        format!("{:?}", no_throw_err),
        "
ThriftError(some client failed

Caused by:
    TransportError: failed to connect to host)
"
    );
    snapshot!(
        format!("{:#?}", no_throw_err),
        r#"
ThriftError(
    Error {
        context: "some client failed",
        source: "TransportError: failed to connect to host",
    },
)
"#
    );

    snapshot!(
        format!("{}", Into::<anyhow::Error>::into(make_no_throw_error())),
        "some client failed"
    );
    snapshot!(
        format!("{:#}", Into::<anyhow::Error>::into(make_no_throw_error())),
        "some client failed: TransportError: failed to connect to host"
    );
    snapshot!(
        format!("{:?}", Into::<anyhow::Error>::into(make_no_throw_error())),
        "
some client failed

Caused by:
    TransportError: failed to connect to host
"
    );
    snapshot!(
        format!("{:#?}", Into::<anyhow::Error>::into(make_no_throw_error())),
        r#"
ThriftError(
    Error {
        context: "some client failed",
        source: "TransportError: failed to connect to host",
    },
)
"#
    );
}
