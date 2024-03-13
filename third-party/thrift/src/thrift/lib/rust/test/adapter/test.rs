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
use std::num::NonZeroI64;

use adapters::CustomString;
use adapters::SortedVec;
use fbthrift::simplejson_protocol;
use thrift_test::consts;
use thrift_test::AdaptedListNewType;
use thrift_test::Asset;
use thrift_test::AssetType;
use thrift_test::Bar;
use thrift_test::Foo;
use thrift_test::TransitiveStructWrapper;
use thrift_test::WrappedAdaptedBytes;
use thrift_test::WrappedAdaptedString;
use thrift_test::WrappedAdaptedWrappedAdaptedBytes;

#[test]
fn test_foo_default() {
    let default_foos: Vec<Foo> = vec![
        Foo::default(),
        simplejson_protocol::deserialize("{}").unwrap(),
    ];

    for default_foo in default_foos {
        assert_eq!(default_foo.str_val, "");
        assert_eq!(default_foo.int_val, 0);
        assert_eq!(default_foo.str_val_adapted, CustomString("".to_string()));
        assert_eq!(
            default_foo.str_val_adapted_default,
            CustomString("hello".to_string())
        );
        assert_eq!(default_foo.str_val_adapted_optional, None);
        assert_eq!(default_foo.validated_int_val, NonZeroI64::new(1).unwrap());
        assert_eq!(
            default_foo.list_val,
            SortedVec(vec!["ispum".to_string(), "lorem".to_string()])
        );
        assert_eq!(default_foo.ident_field, "".to_string());
        assert_eq!(default_foo.typedef_str_val, CustomString("".to_string()));
        assert_eq!(
            default_foo.typedef_str_val_default,
            CustomString("Hello".to_string())
        );
        assert_eq!(default_foo.typedef_str_val_optional, None);
        assert_eq!(
            default_foo.pass_through_adapted_i64,
            NonZeroI64::new(42).unwrap()
        );
        assert_eq!(
            default_foo.pass_through_adapted_and_field_i64,
            NonZeroI64::new(2).unwrap()
        );
        assert_eq!(
            default_foo.adapted_int_list,
            vec![
                NonZeroI64::new(1).unwrap(),
                NonZeroI64::new(2).unwrap(),
                NonZeroI64::new(3).unwrap()
            ]
        );
        assert_eq!(
            default_foo.adapted_string_list,
            vec![
                CustomString("hello".to_string()),
                CustomString("world".to_string()),
            ]
        );
        assert_eq!(
            default_foo.nested_adapted_string_list,
            vec![vec![vec![
                CustomString("hello".to_string()),
                CustomString("world".to_string()),
            ]]]
        );
        assert_eq!(
            default_foo.field_adapted_adapted_string_list,
            vec![CustomString("zucc".to_string()),]
        );
        assert_eq!(
            default_foo.adapted_struct,
            Asset {
                type_: AssetType::Unknown,
                id: 0
            }
        );
        assert_eq!(
            default_foo.double_adapted_struct,
            Asset {
                type_: AssetType::Unknown,
                id: 0
            }
        );
        assert_eq!(
            default_foo.adapted_struct_list,
            vec![Asset {
                type_: AssetType::Laptop,
                id: 10
            }]
        );
        assert_eq!(
            default_foo.adapted_list_new_type,
            AdaptedListNewType(vec![
                CustomString("hi".to_string()),
                CustomString("there".to_string())
            ])
        );
        assert_eq!(default_foo.map_with_adapted_key_val, {
            let mut map = BTreeMap::new();
            map.insert(
                CustomString("what".to_string()),
                WrappedAdaptedWrappedAdaptedBytes(WrappedAdaptedBytes("are those?".into())),
            );

            map
        });
        assert_eq!(default_foo.map_with_adapted_wrapped_key_val, {
            let mut map = BTreeMap::new();
            map.insert(
                WrappedAdaptedString(CustomString("marco".to_string())),
                WrappedAdaptedWrappedAdaptedBytes(WrappedAdaptedBytes("polo".into())),
            );

            map
        });

        assert_eq!(
            r#"{
          "str_val": "",
          "int_val": 0,
          "str_val_adapted": "",
          "str_val_adapted_default": "hello",
          "validated_int_val": 1,
          "list_val": ["ispum", "lorem"],
          "field_checked": "",
          "ident_field": "",
          "typedef_str_val": "",
          "typedef_str_val_default": "Hello",
          "pass_through_adapted_i64": 42,
          "pass_through_adapted_and_field_i64": 2,
          "adapted_int_list": [1, 2, 3],
          "adapted_string_list": ["hello", "world"],
          "nested_adapted_string_list": [[["hello", "world"]]],
          "nested_adapted_int_map": {
            "hello": [[[1, 2, 3], [4, 5, 6]]],
            "world": [[[7, 8, 9]]]
          },
          "field_adapted_adapted_string_list": ["zucc"],
          "adapted_struct": {"type_": 0, "id": 0, "id1": 1, "id2": 2, "id3": 3, "comment": ""},
          "double_adapted_struct": {"type_": 0, "id": 0, "id1": 1, "id2": 2, "id3": 3, "comment": ""},
          "adapted_struct_list": [{"type_": 1, "id": 10, "id1": 1, "id2": 2, "id3": 3, "comment": ""}],
          "adapted_list_new_type": ["hi", "there"],
          "map_with_adapted_key_val": {"what": "YXJlIHRob3NlPw"},
          "map_with_adapted_wrapped_key_val": {"marco": "cG9sbw"}
        }"#
            .replace(['\n', ' '], ""),
            String::from_utf8(simplejson_protocol::serialize(default_foo).into()).unwrap()
        );
    }
}

#[test]
fn test_foo_deser() {
    let foo: Foo = simplejson_protocol::deserialize(
        r#"{
          "str_val": "rust",
          "int_val": 3,
          "str_val_adapted": "python",
          "str_val_adapted_default": "c++",
          "str_val_adapted_optional": "golang",
          "typedef_str_val": "Python",
          "typedef_str_val_default": "C++",
          "typedef_str_val_optional": "Golang",
          "validated_int_val": 42,
          "list_val": ["zzz", "hi", "there"],
          "pass_through_adapted_and_field_i64": 100
        }"#,
    )
    .unwrap();

    assert_eq!(foo.str_val, "rust");
    assert_eq!(foo.int_val, 3);
    assert_eq!(foo.str_val_adapted, CustomString("python".to_string()));
    assert_eq!(foo.str_val_adapted_default, CustomString("c++".to_string()));
    assert_eq!(
        foo.str_val_adapted_optional,
        Some(CustomString("golang".to_string()))
    );
    assert_eq!(foo.typedef_str_val, CustomString("Python".to_string()));
    assert_eq!(foo.typedef_str_val_default, CustomString("C++".to_string()));
    assert_eq!(
        foo.typedef_str_val_optional,
        Some(CustomString("Golang".to_string()))
    );
    assert_eq!(foo.validated_int_val, NonZeroI64::new(42).unwrap());
    assert_eq!(
        foo.list_val,
        SortedVec(vec![
            "hi".to_string(),
            "there".to_string(),
            "zzz".to_string()
        ])
    );
    assert_eq!(
        foo.pass_through_adapted_and_field_i64,
        NonZeroI64::new(100).unwrap()
    );
}

#[test]
fn test_foo_deser_fail() {
    let deser_result: Result<Foo, anyhow::Error> = simplejson_protocol::deserialize(
        r#"{
          "str_val": "rust",
          "int_val": 3,
          "str_val_adapted": "python",
          "str_val_adapted_default": "c++",
          "str_val_adapted_optional": "golang",
          "validated_int_val": 0,
          "list_val": ["zzz", "hi", "there"]
        }"#,
    );

    assert!(deser_result.is_err());
    assert_eq!(
        deser_result.unwrap_err().to_string(),
        "Given i64 is not non-zero: 0"
    );
}

#[test]
fn test_foo_ser() {
    let foo = Foo {
        str_val: "rust".to_string(),
        int_val: 3,
        str_val_adapted: CustomString("python".to_string()),
        str_val_adapted_default: CustomString("c++".to_string()),
        str_val_adapted_optional: Some(CustomString("golang".to_string())),
        validated_int_val: NonZeroI64::new(42).unwrap(),
        list_val: SortedVec(vec![
            "hi".to_string(),
            "there".to_string(),
            "zzz".to_string(),
        ]),
        ident_field: "foobar".to_string(),
        typedef_str_val: CustomString("haskell".to_string()),
        typedef_str_val_default: CustomString("ruby".to_string()),
        typedef_str_val_optional: Some(CustomString("visualbasic".to_string())),
        pass_through_adapted_i64: NonZeroI64::new(13).unwrap(),
        pass_through_adapted_and_field_i64: NonZeroI64::new(14).unwrap(),
        adapted_int_list: vec![NonZeroI64::new(15).unwrap()],
        adapted_string_list: vec![CustomString("java".to_string())],
        nested_adapted_string_list: vec![vec![vec![CustomString("ada".to_string())]]],
        ..Default::default()
    };

    assert_eq!(
        r#"{
          "str_val": "rust",
          "int_val": 3,
          "str_val_adapted": "python",
          "str_val_adapted_default": "c++",
          "str_val_adapted_optional": "golang",
          "validated_int_val": 42,
          "list_val": ["hi", "there", "zzz"],
          "field_checked": "",
          "ident_field": "foobar",
          "typedef_str_val": "haskell",
          "typedef_str_val_default": "ruby",
          "typedef_str_val_optional": "visualbasic",
          "pass_through_adapted_i64": 13,
          "pass_through_adapted_and_field_i64": 14,
          "adapted_int_list": [15],
          "adapted_string_list": ["java"],
          "nested_adapted_string_list": [[["ada"]]],
          "nested_adapted_int_map": {
            "hello": [[[1, 2, 3], [4, 5, 6]]],
            "world": [[[7, 8, 9]]]
          },
          "field_adapted_adapted_string_list": ["zucc"],
          "adapted_struct": {"type_": 0, "id": 0, "id1": 1, "id2": 2, "id3": 3, "comment": ""},
          "double_adapted_struct": {"type_": 0, "id": 0, "id1": 1, "id2": 2, "id3": 3, "comment": ""},
          "adapted_struct_list": [{"type_": 1, "id": 10, "id1": 1, "id2": 2, "id3": 3, "comment": ""}],
          "adapted_list_new_type": ["hi", "there"],
          "map_with_adapted_key_val": {"what": "YXJlIHRob3NlPw"},
          "map_with_adapted_wrapped_key_val": {"marco": "cG9sbw"}
        }"#
        .replace(['\n', ' '], ""),
        std::string::String::from_utf8(simplejson_protocol::serialize(foo).into()).unwrap()
    );
}

#[test]
fn test_bar_default() {
    assert_eq!(Bar::UnknownField(-1), Bar::default());
}

#[test]
fn test_bar_deser() {
    let bar: Bar = simplejson_protocol::deserialize(
        r#"{
          "list_val": ["world", "hello"]
        }"#,
    )
    .unwrap();

    assert_eq!(
        Bar::list_val(SortedVec(vec!["hello".to_string(), "world".to_string()])),
        bar
    );

    let bar: Bar = simplejson_protocol::deserialize(
        r#"{
          "str_val": "zucc"
        }"#,
    )
    .unwrap();

    assert_eq!(Bar::str_val(CustomString("zucc".to_string())), bar);

    let bar: Bar = simplejson_protocol::deserialize(
        r#"{
          "str_val_not_adapted": "zucc"
        }"#,
    )
    .unwrap();

    assert_eq!(Bar::str_val_not_adapted("zucc".to_string()), bar);

    let bar: Bar = simplejson_protocol::deserialize(
        r#"{
          "validated_int_val": 42
        }"#,
    )
    .unwrap();

    assert_eq!(Bar::validated_int_val(NonZeroI64::new(42).unwrap()), bar);

    let bar: Bar = simplejson_protocol::deserialize(r#"{}"#).unwrap();

    assert_eq!(Bar::UnknownField(-1), bar);

    let bar: Bar = simplejson_protocol::deserialize(
        r#"{
          "does_not_exist": 42
        }"#,
    )
    .unwrap();

    assert_eq!(Bar::UnknownField(-1), bar);
}

#[test]
fn test_bar_deser_failure() {
    let deser_result: Result<Bar, anyhow::Error> = simplejson_protocol::deserialize(
        r#"{
          "validated_int_val": 0,
        }"#,
    );

    assert!(deser_result.is_err());
    assert_eq!(
        deser_result.unwrap_err().to_string(),
        "Given i64 is not non-zero: 0"
    );
}

#[test]
fn test_bar_ser() {
    assert_eq!(
        String::from_utf8(
            simplejson_protocol::serialize(Bar::list_val(SortedVec(vec![
                "hello".to_string(),
                "world".to_string()
            ])))
            .into()
        )
        .unwrap(),
        r#"{
          "list_val": ["hello", "world"]
        }"#
        .replace(['\n', ' '], "")
    );

    assert_eq!(
        String::from_utf8(
            simplejson_protocol::serialize(Bar::str_val_not_adapted("zucc".to_string())).into()
        )
        .unwrap(),
        r#"{
          "str_val_not_adapted": "zucc"
        }"#
        .replace(['\n', ' '], "")
    );
}

#[test]
fn test_newtype() {
    let wrapped = WrappedAdaptedWrappedAdaptedBytes(WrappedAdaptedBytes("hello world".into()));

    assert_eq!(
        String::from_utf8(simplejson_protocol::serialize(wrapped.clone()).into()).unwrap(),
        r#""aGVsbG8gd29ybGQ""#
    );

    // The '=' is just a padding and can be omitted if desired.
    let deser: WrappedAdaptedWrappedAdaptedBytes =
        simplejson_protocol::deserialize(r#""aGVsbG8gd29ybGQ=""#).unwrap();

    assert_eq!(wrapped, deser);
}

#[test]
fn test_consts() {
    assert_eq!(*consts::adapted_int, NonZeroI64::new(5).unwrap());
    assert_eq!(
        *consts::pass_through_adapted_int,
        NonZeroI64::new(6).unwrap()
    );
    assert_eq!(
        *consts::adapted_bytes_const,
        WrappedAdaptedWrappedAdaptedBytes(WrappedAdaptedBytes("some_bytes".into()))
    );
    assert_eq!(
        *consts::adapted_list_const,
        AdaptedListNewType(vec![
            CustomString("hello".to_string()),
            CustomString("world".to_string())
        ])
    );
    assert_eq!(
        *consts::adapted_struct_const,
        Asset {
            type_: AssetType::Server,
            id: 42,
        }
    );
}

#[test]
fn test_annotations() {
    let deser: TransitiveStructWrapper =
        simplejson_protocol::deserialize(r#"{"test_field": "boo", "test_field_2": "spooky"}"#)
            .unwrap();

    assert_eq!(deser.0.0.test_field.0, "boo");
    assert_eq!(deser.0.0.test_field_2.0, "spooky");

    assert_eq!(
        simplejson_protocol::serialize(deser),
        r#"{"test_field":"boo","test_field_2":"spooky"}"#
    );
}
