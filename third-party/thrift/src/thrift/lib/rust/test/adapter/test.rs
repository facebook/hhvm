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

use std::num::NonZeroI64;

use adapters::CustomString;
use adapters::SortedVec;
use fbthrift::simplejson_protocol;
use thrift_test::types::Bar;
use thrift_test::types::Foo;

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

        assert_eq!(
            r#"{
          "str_val": "",
          "int_val": 0,
          "str_val_adapted": "",
          "str_val_adapted_default": "hello",
          "validated_int_val": 1,
          "list_val": ["ispum", "lorem"],
          "field_checked": ""
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
          "validated_int_val": 42,
          "list_val": ["zzz", "hi", "there"]
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
    assert_eq!(foo.validated_int_val, NonZeroI64::new(42).unwrap());
    assert_eq!(
        foo.list_val,
        SortedVec(vec![
            "hi".to_string(),
            "there".to_string(),
            "zzz".to_string()
        ])
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
          "field_checked": ""
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
