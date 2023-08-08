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

    // Note that the default value for optional with default with
    // `deprecated_optional_with_default_is_some` flag is NOT the same
    // as in C++
    assert_eq!(sub.optDef.as_deref(), Some("IAMOPT"));

    assert_eq!(sub.req_def, "IAMREQ");
    assert!(sub.key_map.is_none());
    assert!(sub.bin.is_empty());
}
