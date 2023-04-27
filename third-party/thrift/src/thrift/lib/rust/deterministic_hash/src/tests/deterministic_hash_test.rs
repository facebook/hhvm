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

use fbthrift::builtin_types::BTreeMap;
use fbthrift::builtin_types::BTreeSet;
use fbthrift_deterministic_hash::deterministic_hash;
use fbthrift_deterministic_hash::Sha256Hasher;
use maplit::btreemap;
use teststructs::ComplexStruct;
use teststructs::ComplexStructSha256Hash;
use teststructs::SimpleStruct1;
use teststructs::SimpleStructSha256Hash;

#[test]
fn hash_test_complex_structure() {
    let complex_struct = ComplexStruct {
        l: vec!["a".to_string(), "ab".to_string(), "abc".to_string()],
        s: BTreeSet::from(["a".to_string(), "ab".to_string(), "abc".to_string()]),
        m: btreemap! {
            "aa".to_owned() => "bb".to_owned(),
            "cc".to_owned() => "dd".to_owned(),
            "ee".to_owned() => "ff".to_owned(),
        },
        ml: btreemap! {
            "aa".to_owned() => vec![1, 2, 3],
            "bb".to_owned() => vec![],
            "cc".to_owned() => vec![3, 2],
            "ee".to_owned() => vec![1, 2, 3],
            "ff".to_owned() => vec![],
        },
        mm: btreemap! {
            "aa".to_owned() => BTreeMap::from([(1, 2)]),
            "bb".to_owned() => BTreeMap::from([]),
            "cc".to_owned() => BTreeMap::from([(2, 3), (3, 2)]),
            "ee".to_owned() => BTreeMap::from([(3, 4)]),
            "ff".to_owned() => BTreeMap::from([]),
        },
        ..Default::default()
    };
    let result =
        deterministic_hash(&complex_struct, Sha256Hasher::default).expect("expected no error");
    assert_eq!(
        result.as_ref(),
        ComplexStructSha256Hash
            .iter()
            .map(|val| *val as u8)
            .collect::<Vec<u8>>()
    );
}

#[test]
fn hash_test_simple_structure() {
    let simple_struct = SimpleStruct1 {
        byte_: 8,
        i_16: 12,
        i_32: 1,
        i_64: 2,
        f_32: 1.0,
        f_64: 2.0,
        str_with_value: "abc".to_string(),
        str_empty: "".to_string(),
        bool_true: true,
        bool_false: false,
        binary_: vec![1u8, 2, 3],
        ..Default::default()
    };
    let result =
        deterministic_hash(&simple_struct, Sha256Hasher::default).expect("expected no error");
    assert_eq!(
        result.as_ref(),
        SimpleStructSha256Hash
            .iter()
            .map(|val| *val as u8)
            .collect::<Vec<u8>>()
    );
}
