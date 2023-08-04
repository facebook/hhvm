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
fn test_consts() {
    // NOTE: Keep in sync with `consts` test in cpp_compat_test

    assert_eq!(
        fbthrift_test_if::SubStruct::default(),
        *fbthrift_test_if::DEFAULT_SUBSTRUCT
    );

    assert_eq!(
        fbthrift_test_if::SubStruct {
            optDef: Some("CUSTOM_OPT_DEF".to_owned()),
            req_def: "CUSTOM_REQ_DEF".to_owned(),
            key_map: Some(
                [(
                    fbthrift_test_if::Small {
                        num: 4,
                        two: 2,
                        ..Default::default()
                    },
                    42
                ),]
                .into_iter()
                .collect()
            ),
            bin: b"0123456789".to_vec(),
            ..Default::default()
        },
        *fbthrift_test_if::CUSTOM_SUBSTRUCT
    );
}
