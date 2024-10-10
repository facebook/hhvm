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

#[test]
fn test_defaults() {
    // NOTE: Keep in sync with `defaults` test in cpp_compat_test

    let sub = fbthrift_test_if::SubStruct::default();

    assert!(sub.optDef.is_none());
    assert_eq!(sub.req_def, "IAMREQ");
    assert!(sub.key_map.is_none());
    assert!(sub.bin.is_empty());
}

#[test]
fn test_deprecated_defaults() {
    // No C++ equivalent here. This is incompatible legacy behavior available as opt-in.

    let sub = test_deprecated_optional_with_default_is_some_if::Struct::default();

    assert_eq!(sub.optDef.as_deref(), Some("IAMOPTWITHDEF"));
}

#[test]
fn test_competing_defaults() {
    let test: test_competing_defaults_if::Outer =
        fbthrift::simplejson_protocol::deserialize("{\"inner\":{}}")
            .expect("Failed to deserialize JSON");

    // Note: in C++ this would be "default_override_in_outer"
    // Historically, Rust behavior is different. This might change in the future,
    // but having this test here documents the current behavior.
    assert_eq!(test.inner.value, "default_in_inner");
}
