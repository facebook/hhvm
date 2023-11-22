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

use std::collections::BTreeMap;
use std::collections::BTreeSet;
use std::collections::HashMap;
use std::collections::HashSet;
use std::fmt::Debug;
use std::io::Cursor;

use bytes::Bytes;
use fbthrift::serialize;
use fbthrift::CompactProtocol;
use fbthrift::Deserialize;
use fbthrift::Protocol;
use fbthrift::Serialize;
use fbthrift::ThriftEnum;
use indexmap::IndexMap;
use indexmap::IndexSet;
use interface::TestBytesShared;
use interface::TestEnum;
use interface::TestEnumEmpty;
use interface::TestSkipV1;
use interface::TestSkipV2;
use interface::TypedefNondefaultTypes_1;
use interface::TypedefNondefaultTypes_2;
use interface::TypedefNondefaultTypes_3;
use interface::WrapBinary;
use interface::WrapString;
use smallvec::SmallVec;

#[test]
fn test_typedef_nondefault_types_1() {
    assert_round_trip(TypedefNondefaultTypes_1 {
        defaultset: make_btreeset(),
        btreeset: make_btreeset(),
        hashset: make_hashset(),
        indexset_a: make_indexset(),
        indexset_b: make_indexset(),
        indexset_c: IndexSet::new(),
        defaultmap: make_btreemap(),
        btreemap: make_btreemap(),
        hashmap: make_hashmap(),
        indexmap_a: make_indexmap(),
        indexmap_b: make_indexmap(),
        indexmap_c: IndexMap::new(),
        bin_smallvec: SmallVec::from(&b"smallvec"[..]),
        bin_bytes: Bytes::from(&b"bytes"[..]),
        ..Default::default()
    });
}

#[test]
fn test_typedef_non_default_types_2() {
    assert_round_trip(TypedefNondefaultTypes_2 {
        defaultset: make_btreeset(),
        btreeset: make_btreeset(),
        hashset: make_hashset(),
        indexset_a: make_indexset(),
        indexset_b: make_indexset(),
        indexset_c: IndexSet::new(),
        defaultmap: make_btreemap(),
        btreemap: make_btreemap(),
        hashmap: make_hashmap(),
        indexmap_a: make_indexmap(),
        indexmap_b: make_indexmap(),
        indexmap_c: IndexMap::new(),
        bin_smallvec: SmallVec::from(&b"smallvec"[..]),
        bin_bytes: Bytes::from(&b"bytes"[..]),
        ..Default::default()
    });
}

#[test]
fn test_typedef_non_default_types_3() {
    assert_round_trip(TypedefNondefaultTypes_3 {
        bytes: SmallVec::from(&b"smallvec"[..]),
        dict: make_indexmap(),
        ..Default::default()
    });
}

fn make_btreeset() -> BTreeSet<String> {
    let mut set = BTreeSet::new();
    set.insert("btreeset".to_owned());
    set
}

fn make_hashset() -> HashSet<String> {
    let mut set = HashSet::new();
    set.insert("hashset".to_owned());
    set
}

fn make_indexset() -> IndexSet<String> {
    let mut set = IndexSet::new();
    set.insert("indexset".to_owned());
    set
}

fn make_btreemap() -> BTreeMap<String, String> {
    let mut map = BTreeMap::new();
    map.insert("btreemap".to_owned(), String::new());
    map
}

fn make_hashmap() -> HashMap<String, String> {
    let mut map = HashMap::new();
    map.insert("hashmap".to_owned(), String::new());
    map
}

fn make_indexmap() -> IndexMap<String, String> {
    let mut map = IndexMap::new();
    map.insert("indexmap".to_owned(), String::new());
    map
}

fn assert_round_trip<T>(value: T)
where
    T: Serialize<<CompactProtocol as Protocol>::Sizer>
        + Serialize<<CompactProtocol as Protocol>::Serializer>
        + Deserialize<<CompactProtocol as Protocol>::Deserializer>
        + PartialEq
        + Debug,
{
    let bytes = serialize!(CompactProtocol, |w| Serialize::write(&value, w));
    let mut deserializer = <CompactProtocol>::deserializer(Cursor::new(bytes));
    let back = Deserialize::read(&mut deserializer).unwrap();
    assert_eq!(value, back);
}

#[test]
fn test_variants_fn_enum() {
    let expected: &'static [&'static str] = &["FOO", "BAR", "BAZ"];
    assert_eq!(TestEnum::variants(), expected);
    let expected_empty: &'static [&'static str] = &[];
    assert_eq!(TestEnumEmpty::variants(), expected_empty);
}

#[test]
fn test_variant_values_fn_enum() {
    let expected = &[TestEnum::FOO, TestEnum::BAR, TestEnum::BAZ];
    assert_eq!(TestEnum::variant_values(), expected);
    let expected_empty: &'static [TestEnumEmpty] = &[];
    assert_eq!(TestEnumEmpty::variant_values(), expected_empty);
}

#[test]
fn test_enumerate_fn_enum() {
    let expected = &[
        (TestEnum::FOO, "FOO"),
        (TestEnum::BAR, "BAR"),
        (TestEnum::BAZ, "BAZ"),
    ];
    assert_eq!(TestEnum::enumerate(), expected);
    let expected_empty: &'static [(TestEnumEmpty, &str)] = &[];
    assert_eq!(TestEnumEmpty::enumerate(), expected_empty);
}

#[test]
fn test_deserialize_skip_seq() {
    let v2 = TestSkipV2::default();
    let bytes = serialize!(CompactProtocol, |w| Serialize::write(&v2, w));
    let mut deserializer = <CompactProtocol>::deserializer(Cursor::new(bytes));
    let v1: TestSkipV1 = Deserialize::read(&mut deserializer).unwrap();
    assert_eq!(v1, TestSkipV1::default());
}

#[test]
fn test_bytes_shared() {
    // Test that when using `Bytes` for `binary` types and for the buffer the
    // data is deserialized from, the deserialized structs share data with the
    // buffer.  Do this by deserializing the same `Bytes`-backed buffer twice
    // and checking both deserialized copies have the same address.  This
    // should be somewhere within the serialized buffer.
    let original = TestBytesShared {
        b: Bytes::from(&b"data"[..]),
        ..Default::default()
    };
    let bytes = serialize!(CompactProtocol, |w| Serialize::write(&original, w));
    let mut deserializer1 = <CompactProtocol>::deserializer(Cursor::new(bytes.clone()));
    let shared1: TestBytesShared = Deserialize::read(&mut deserializer1).unwrap();
    let mut deserializer2 = <CompactProtocol>::deserializer(Cursor::new(bytes));
    let shared2: TestBytesShared = Deserialize::read(&mut deserializer2).unwrap();
    assert_eq!(shared1.b.as_ptr() as usize, shared2.b.as_ptr() as usize);
}

#[test]
fn test_nonutf8_string() {
    // Serializing `binary` and deserializing to `string` is okay, as long as
    // the data is valid UTF-8. Thrift intentionally uses the same on-wire
    // format for these two types.
    let data = b"...".to_vec();
    let value = WrapBinary { data };
    let repr = serialize!(CompactProtocol, |w| Serialize::write(&value, w));
    let mut deserializer = <CompactProtocol>::deserializer(Cursor::new(repr));
    let value2 = WrapString::read(&mut deserializer).unwrap();
    assert_eq!("...", value2.data);

    // Same thing with non-UTF-8 data is not okay. It happens to work in C++
    // because their only string type holds arbitrary bytes, but not Rust or
    // other languages which enforce a string encoding.
    let data = b"..\xff".to_vec();
    let value = WrapBinary { data };
    let repr = serialize!(CompactProtocol, |w| Serialize::write(&value, w));
    let mut deserializer = <CompactProtocol>::deserializer(Cursor::new(repr));
    let error = WrapString::read(&mut deserializer).unwrap_err();
    let expected = "deserializing `string` from Thrift compact protocol got invalid utf-8, you need to use `binary` instead: invalid utf-8 sequence of 1 bytes from index 2";
    assert_eq!(expected, error.to_string());
}
