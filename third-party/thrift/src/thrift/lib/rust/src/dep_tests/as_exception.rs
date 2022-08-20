/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

use anyhow::Result;
use fbthrift_test_if::errors::test_service::AsTestException;
use fbthrift_test_if::errors::test_service::Method1Error;
use fbthrift_test_if::types::TestException;

#[test]
fn test_as_exception() -> Result<()> {
    let me = Method1Error::ex(TestException::default());
    assert_eq!(Some(&TestException::default()), me.as_test_exception());

    let me = Method1Error::ThriftError(anyhow::anyhow!(""));
    assert_eq!(None, me.as_test_exception());

    Ok(())
}

#[test]
fn test_as_exception_anyhow() -> Result<()> {
    let ae: anyhow::Error = Method1Error::ex(TestException::default()).into();
    assert_eq!(Some(&TestException::default()), ae.as_test_exception());

    let ae: anyhow::Error = Method1Error::ThriftError(anyhow::anyhow!("")).into();
    assert_eq!(None, ae.as_test_exception());

    Ok(())
}
