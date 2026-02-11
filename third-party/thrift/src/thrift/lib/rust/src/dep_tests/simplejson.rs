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
use std::io::Cursor;

use anyhow::Result;
use approx::assert_relative_eq;
use fbthrift::Deserialize;
use fbthrift::simplejson_protocol::SimpleJsonProtocolDeserializer;
use fbthrift::simplejson_protocol::deserialize;
use fbthrift::simplejson_protocol::serialize;
use fbthrift_test_if::Basic;
use fbthrift_test_if::Containers;
use fbthrift_test_if::En;
use fbthrift_test_if::FloatingPoint;
use fbthrift_test_if::MainStruct;
use fbthrift_test_if::MainStructNoBinary;
use fbthrift_test_if::Small;
use fbthrift_test_if::SubStruct;
use fbthrift_test_if::Un;
use fbthrift_test_if::UnOne;
use proptest::prelude::*;
use rstest::*;

use crate::proptest::gen_main_struct;

#[test]
fn test_large_roundtrip() -> Result<()> {
    // Build the big struct
    let mut m = BTreeMap::new();
    m.insert("m1".to_string(), 1);
    m.insert("m2".to_string(), 2);

    let sub = SubStruct {
        optDef: Some("IAMOPT".to_owned()),
        ..Default::default()
    };
    let u = Un::un1(UnOne {
        one: 1,
        ..Default::default()
    });
    let e = En::TWO;

    let mut int_keys = BTreeMap::new();
    int_keys.insert(42, 43);
    int_keys.insert(44, 45);

    let r = MainStruct {
        foo: "foo".to_string(),
        m,
        bar: "test".to_string(),
        s: sub,
        l: vec![
            Small {
                num: 1,
                two: 2,
                ..Default::default()
            },
            Small {
                num: 2,
                two: 3,
                ..Default::default()
            },
        ],
        u,
        e,
        int_keys,
        opt: None,
        ..Default::default()
    };

    // serialize it and assert that it serializes correctly
    let s = String::from_utf8(serialize(&r).to_vec()).unwrap();

    let expected_string = r#"{
        "foo":"foo",
        "m":{"m1":1,"m2":2},
        "bar":"test",
        "s":{"optDef":"IAMOPT","req_def":"IAMREQ","bin":""},
        "l":[{"num":1,"two":2},{"num":2,"two":3}],
        "u":{"un1":{"one":1}},
        "e":2,
        "int_keys":{"42":43,"44":45}
    }"#
    .replace([' ', '\n'], "");
    assert_eq!(expected_string, s);

    // It at least needs to be valid json, the serialize then
    // deserialize and compare will come in the next diff
    let v: serde_json::Result<serde_json::Value> = serde_json::from_str(&s);
    assert!(v.is_ok());

    // Assert that deserialize builts the exact same struct
    assert_eq!(r, deserialize(s).unwrap());

    Ok(())
}

#[test]
fn test_struct_key() -> Result<()> {
    // See the `structKey` test in cpp_compat_test

    let mut h = std::collections::BTreeMap::new();
    h.insert(
        Small {
            ..Default::default()
        },
        1,
    );
    let sub = SubStruct {
        key_map: Some(h),
        // In rust we need to specify optionals with defaults as None
        // instead of relying on ..Default::default()
        optDef: None,
        ..Default::default()
    };

    let s = String::from_utf8(serialize(&sub).to_vec()).unwrap();
    let expected_string = r#"{
        "req_def":"IAMREQ",
        "key_map":{{"num":0,"two":0}:1},
        "bin":""
    }"#
    .replace([' ', '\n'], "");
    assert_eq!(expected_string, s);

    // It's definitely not JSON...
    let v: serde_json::Result<serde_json::Value> = serde_json::from_str(&s);
    assert!(v.is_err());

    // ...but it needs to deserialize
    assert_eq!(sub, deserialize(&s).unwrap());

    // ...though not to serde_json::Value.
    if let Ok(wat) = deserialize(&s) {
        let _: serde_json::Value = wat;
        panic!("map with struct keys is not supposed to deserialize to Value");
    }

    Ok(())
}

#[test]
fn test_weird_text() -> Result<()> {
    // See the `weirdText` test in cpp_compat_test

    let mut sub = SubStruct {
        optDef: Some("stuff\twith\nescape\\characters'...\"lots{of}fun</xml>".to_string()),
        bin: "1234".as_bytes().to_vec(),
        ..Default::default()
    };

    let s = String::from_utf8(serialize(&sub).to_vec()).unwrap();
    let expected_string = r#"{
        "optDef":"stuff\twith\nescape\\characters'...\"lots{of}fun</xml>",
        "req_def":"IAMREQ",
        "bin":"MTIzNA"
    }"#
    .replace([' ', '\n'], "");
    assert_eq!(expected_string, s);
    // Make sure its equal
    assert_eq!(sub, deserialize(s).unwrap());

    // Unicode escaping
    sub.optDef = Some("UNICODE\u{1F60A}UH OH".to_string());

    let s = String::from_utf8(serialize(&sub).to_vec()).unwrap();
    let expected_string = r#"{
        "optDef":"UNICODE😊UH OH",
        "req_def":"IAMREQ",
        "bin":"MTIzNA"
    }"#
    // Double-space to deal with "tabs"
    .replace("  ", "")
    .replace('\n', "");
    assert_eq!(expected_string, s);
    // Make sure its equal
    assert_eq!(sub, deserialize(s).unwrap());

    Ok(())
}

#[test]
fn test_skip_complex() -> Result<()> {
    // See the `skipComplex` test in cpp_compat_test

    let sub = SubStruct {
        optDef: Some("thing".to_string()),
        bin: "1234".as_bytes().to_vec(),
        ..Default::default()
    };

    let input = r#"{
        "optDef":"thing",
        "req_def":"IAMREQ",
        "bin":"MTIzNA",
        "extra":[1,{"thing":"thing2"}],
        "extra_map":{"thing":null,"thing2":2},
        "extra_bool":true
    }"#
    .replace([' ', '\n'], "");
    // Make sure everything is skipped properly
    assert_eq!(sub, deserialize(input).unwrap());

    Ok(())
}

#[test]
fn test_need_commas() -> Result<()> {
    // See the `needCommas` test in cpp_compat_test

    // Note the missing commas

    let input = r#"{
        "num":1
        "two":2
    }"#
    .replace([' ', '\n'], "");
    assert!(deserialize::<Small, _, _>(input).is_err());

    // even when skipping
    let input2 = r#"{
        "num":1,
        "two":2,
        "extra_map":{"thing":null,"thing2":2}
        "extra_bool":true
    }"#
    .replace([' ', '\n'], "");
    assert!(deserialize::<Small, _, _>(input2).is_err());

    Ok(())
}

#[test]
fn test_need_commas_containers() -> Result<()> {
    // See the `needCommasContainers` test in cpp_compat_test

    let goodinput = r#"{
        "m":{"m1":"m1","m2":"m2"}
    }"#;
    // Note the missing comma
    let badinput = r#"{
        "m":{"m1":"m1""m2":"m2"}
    }"#;
    assert!(deserialize::<Containers, _, _>(goodinput).is_ok());
    assert!(deserialize::<Containers, _, _>(badinput).is_err());

    let goodinput2 = r#"{
        "l":["l1","l2"]
    }"#;
    // Note the missing comma
    let badinput2 = r#"{
        "l":["l1""l2"]
    }"#;
    assert!(deserialize::<Containers, _, _>(goodinput2).is_ok());
    assert!(deserialize::<Containers, _, _>(badinput2).is_err());

    Ok(())
}

#[test]
fn test_null_stuff_deser() -> Result<()> {
    // See the `nullStuffDeser` test in cpp_compat_test

    let sub = SubStruct {
        bin: "1234".as_bytes().to_vec(),
        ..Default::default()
    };

    let inputs = &[
        r#"{                 "req_def": "IAMREQ", "bin": "MTIzNA" }"#,
        r#"{ "optDef": null, "req_def": "IAMREQ", "bin": "MTIzNA" }"#,
    ];
    for input in inputs {
        // Make sure everything is skipped properly
        let res = deserialize(*input);
        assert_eq!(
            Some(&sub),
            res.as_ref().ok(),
            "INPUT={} RESULT={:?}",
            input,
            res
        );
    }

    Ok(())
}

#[test]
fn test_deprecated_null_stuff_deser() -> Result<()> {
    // No C++ equivalent here. This is incompatible legacy behavior available as opt-in.

    // Note that the deserialization behavior with `deprecated_optional_with_default_is_some`
    // is not consistent with `Default::default`. If optional-with-default is
    // not present in the stream (or if it is null), then it is `None` in the
    // resulting struct. This behavior is the same as in C++ and as in Rust
    // without the deprecated flag, but not as in `default` or in constants.

    let sub = test_deprecated_optional_with_default_is_some_if::Struct {
        optDef: None,
        marker: true,
        ..Default::default()
    };

    let inputs = &[
        r#"{                 "marker": true }"#,
        r#"{ "optDef": null, "marker": true }"#,
    ];
    for input in inputs {
        // Make sure everything is skipped properly
        let res = deserialize(*input);
        assert_eq!(
            Some(&sub),
            res.as_ref().ok(),
            "INPUT={} RESULT={:?}",
            input,
            res
        );
    }

    Ok(())
}

#[test]
fn infinite_spaces() -> Result<()> {
    let mut m = BTreeMap::new();
    m.insert("m1".to_string(), 1);
    m.insert("m2".to_string(), 2);

    let sub = SubStruct::default();
    let u = Un::un1(UnOne {
        one: 1,
        ..Default::default()
    });
    let e = En::TWO;

    let mut int_keys = BTreeMap::new();
    int_keys.insert(42, 43);
    int_keys.insert(44, 45);

    let r = MainStruct {
        foo: "foo".to_string(),
        m,
        bar: " test ".to_string(),
        s: sub,
        l: vec![
            Small {
                num: 1,
                two: 2,
                ..Default::default()
            },
            Small {
                num: 2,
                ..Default::default()
            },
        ],
        u,
        e,
        int_keys,
        opt: None,
        ..Default::default()
    };

    let input = r#"{
         "foo"  :  "foo" ,
          "m" : { "m1" :  1   , "m2" : 2 }  ,
        "bar":" test ",
        "s":{  "req_def":  "IAMREQ","bin": ""  },
        "l":[{"num":1,"two":2},{"num"  :2 ," two" : 3 } ],
        "u":{"un1":{"one":  1  } },
        "e":  2  ,
        "int_keys"  :{"42"   :  43,  "44":45}
    }"#;

    // Assert that deserialize builts the exact same struct
    assert_eq!(r, deserialize(input).unwrap());

    Ok(())
}

#[test]
fn test_bool() -> Result<()> {
    let b = Basic {
        b: true,
        b2: false,
        ..Default::default()
    };
    // serialize it and assert that it serializes correctly
    let s = String::from_utf8(serialize(&b).to_vec()).unwrap();

    // Assert that deserialize builts the exact same struct
    assert_eq!(b, deserialize(s).unwrap());

    Ok(())
}

#[test]
fn test_serde_compat() -> Result<()> {
    // Build the big struct
    let mut m = BTreeMap::new();
    m.insert("m1".to_string(), 1);
    m.insert("m2".to_string(), 2);

    let u = Un::un1(UnOne {
        one: 1,
        ..Default::default()
    });
    let e = En::TWO;

    let mut int_keys = BTreeMap::new();
    int_keys.insert(42, 43);
    int_keys.insert(44, 45);

    let r = MainStructNoBinary {
        foo: "foo".to_string(),
        m,
        bar: "test".to_string(),
        l: vec![
            Small {
                num: 1,
                two: 2,
                ..Default::default()
            },
            Small {
                num: 2,
                two: 3,
                ..Default::default()
            },
        ],
        u,
        e,
        int_keys,
        opt: None,
        ..Default::default()
    };

    let fbthrift_s = String::from_utf8(serialize(&r).to_vec()).unwrap();
    // We aren't going to get full compat, but at least make it so fbthrift
    // can deserialize what serde has written out
    let serde_s = serde_json::to_string(&r).unwrap();

    // but passing between them should work
    assert_eq!(r, serde_json::from_str(&fbthrift_s).unwrap());
    assert_eq!(r, deserialize(&serde_s).unwrap());

    Ok(())
}

#[test]
fn test_serde_compat_empty_union() -> Result<()> {
    // Test the empty union scenario specifically, as it is a bit of
    // a special case
    let empty = Un::default();

    // Historically, serde rep of an empty union has been different to simplejson
    let empty_serde = serde_json::to_string(&empty).unwrap();
    assert_eq!(r#"{"UnknownField":-1}"#, empty_serde);
    let empty_simplejson = String::from_utf8(serialize(&empty).to_vec()).unwrap();
    assert_eq!("{}", empty_simplejson);

    // Mixed round-trip serde-to-simplejson should still work fine though
    assert_eq!(empty, deserialize(&empty_serde).unwrap());

    // Historically, mixed round-trip simplejson-to-serde fails
    // assert_eq!(empty, serde_json::from_str(&empty_simplejson).unwrap());

    Ok(())
}

#[test]
fn test_serde_compat_deser_errors() -> Result<()> {
    // Historically, serde deserialization for unions is stricter than simplejson.
    // It returns an error on mising or unknown variants
    assert!(serde_json::from_str::<Un>("{}").is_err());
    assert!(serde_json::from_str::<Un>(r#"{"NotAVariant":{}}"#).is_err());

    // On the other hand, serde deserialization for structs is similar to simplejson.
    // It tolerates missing or unknown fields
    assert!(serde_json::from_str::<UnOne>("{}").is_ok());
    assert!(serde_json::from_str::<UnOne>(r#"{"one":1,"NotAField":2}"#).is_ok());

    Ok(())
}

#[test]
fn test_multiple_deser() -> Result<()> {
    // Tests that we don't too eagerly advance the buffer
    let b1 = Basic {
        b: true,
        b2: false,
        ..Default::default()
    };
    let b2 = Basic {
        b: true,
        b2: true,
        ..Default::default()
    };
    // serialize it and assert that it serializes correctly
    let s1 = String::from_utf8(serialize(&b1).to_vec()).unwrap();
    let s2 = String::from_utf8(serialize(&b2).to_vec()).unwrap();
    let to_check = format!("{} {}", s1, s2);

    let mut deserializer = SimpleJsonProtocolDeserializer::new(Cursor::new(to_check.as_bytes()));
    // Assert that deserialize builts the exact same struct
    assert_eq!(b1, Basic::rs_thrift_read(&mut deserializer)?);
    assert_eq!(b2, Basic::rs_thrift_read(&mut deserializer)?);

    Ok(())
}

#[test]
fn test_not_enough() -> Result<()> {
    // Tests that we can deserialize until
    // we run out, and don't panic
    let b1 = Basic {
        b: true,
        b2: false,
        ..Default::default()
    };
    let b2 = Basic {
        b: true,
        b2: true,
        ..Default::default()
    };
    // serialize it and assert that it serializes correctly
    let s1 = String::from_utf8(serialize(&b1).to_vec()).unwrap();
    let s2 = String::from_utf8(serialize(b2).to_vec()).unwrap();
    let to_check = format!("{} {}", s1, s2);

    let mut deserializer = SimpleJsonProtocolDeserializer::new(Cursor::new(
        // 6 should cover the } and a `true` value
        &to_check[..to_check.len() - 6],
    ));
    // Assert that deserialize builts the exact same struct
    assert_eq!(b1, Basic::rs_thrift_read(&mut deserializer)?);
    assert!(Basic::rs_thrift_read(&mut deserializer).is_err());

    Ok(())
}

#[test]
fn test_unknown_union() -> Result<()> {
    // See unknownUnion

    // Build the empty union
    let u = Un::default();

    let s = String::from_utf8(serialize(&u).to_vec()).unwrap();
    let expected_string = "{}";
    assert_eq!(expected_string, s);

    // Assert that deserialize builts the exact same struct
    assert_eq!(u, deserialize(s).unwrap());

    // ...
    // extra weirdness
    // Build an explicit unknown
    let explicit_unknown = Un::UnknownField(100);
    let s2 = String::from_utf8(serialize(explicit_unknown).to_vec()).unwrap();
    let expected_string = "{}";
    assert_eq!(expected_string, s2);

    // Deserializes to the default -1 case, this matches the other
    // protocols behavior
    assert_eq!(u, deserialize(s2).unwrap());

    // backwards compat test
    let old_output = r#"{
        "UnknownField":-1
    }"#
    .replace([' ', '\n'], "");

    assert_eq!(u, deserialize(old_output).unwrap());

    Ok(())
}

#[rstest]
#[case(r#"{ "one": "1 }"#)]
#[case(r#"{ "one": "1 ,", ":null}"#)]
fn test_invalid_json(#[case] json: &str) {
    let res: Result<UnOne> = deserialize(json);
    assert!(res.is_err(), "Expected error, got {:?}", res);
}

#[rstest]
#[case::zero(0.0, r#"{"f32":0.0,"f64":0.0}"#)]
#[case::negative_zero(-0.0, r#"{"f32":-0.0,"f64":0.0}"#)]
#[case::one(1.0, r#"{"f32":1.0,"f64":0.0}"#)]
#[case::min_positive(f32::MIN_POSITIVE, r#"{"f32":1.1754944e-38,"f64":0.0}"#)]
#[case::negative_one(-1.0, r#"{"f32":-1.0,"f64":0.0}"#)]
#[case::min(f32::MIN, r#"{"f32":-3.4028235e+38,"f64":0.0}"#)]
#[case::max(f32::MAX, r#"{"f32":3.4028235e+38,"f64":0.0}"#)]
#[case::infinity(f32::INFINITY, r#"{"f32":"Infinity","f64":0.0}"#)]
#[case::negative_infinity(f32::NEG_INFINITY, r#"{"f32":"-Infinity","f64":0.0}"#)]
#[case::nan(f32::NAN, r#"{"f32":"NaN","f64":0.0}"#)]
#[case::negative_nan(-f32::NAN, r#"{"f32":"NaN","f64":0.0}"#)]
fn test_f32_round_trip(#[case] value: f32, #[case] expected_json: &str) -> Result<()> {
    let fp = FloatingPoint {
        f32: value,
        ..Default::default()
    };

    let s = String::from_utf8(serialize(&fp).to_vec()).unwrap();
    assert_eq!(s, expected_json);
    let d: FloatingPoint = deserialize(s).unwrap();

    if value.is_nan() {
        assert!(d.f32.is_nan(), "value: {}", d.f32);
    } else {
        assert_relative_eq!(d.f32, fp.f32);
    }

    Ok(())
}

#[rstest]
#[case::nan(r#"{ "f32": "NaN" }"#, f32::NAN)]
#[case::negative_nan(r#"{ "f32": "-NaN" }"#, -f32::NAN)]
#[case::infinity(r#"{ "f32": "Infinity" }"#, f32::INFINITY)]
#[case::negative_infinity(r#"{ "f32": "-Infinity" }"#, f32::NEG_INFINITY)]
#[case::ryu_nan(r#"{ "f32": "5.1042355e38" }"#, f32::NAN)]
#[case::ryu_negative_nan(r#"{ "f32": "-5.1042355e38" }"#, -f32::NAN)]
#[case::ryu_infinity(r#"{ "f32": "3.4028237e38" }"#, f32::INFINITY)]
#[case::ryu_negative_infinity(r#"{ "f32": "-3.4028237e38" }"#, f32::NEG_INFINITY)]
#[case::f64_infinity(r#"{ "f32": "1.797693134862316e308" }"#, f32::NAN)]
#[case::f64_negative_infinity(r#"{ "f32": "-1.797693134862316e308" }"#, f32::NAN)]
fn test_f32_deserialize(#[case] json: &str, #[case] expected_value: f32) -> Result<()> {
    let d: FloatingPoint = deserialize(json).unwrap();
    if expected_value.is_nan() {
        assert!(d.f32.is_nan(), "value: {}", d.f32);
    } else {
        assert_relative_eq!(d.f32, expected_value);
    }

    Ok(())
}

#[rstest]
#[case::zero(0.0, r#"{"f32":0.0,"f64":0.0}"#)]
#[case::negative_zero(-0.0, r#"{"f32":0.0,"f64":-0.0}"#)]
#[case::one(1.0, r#"{"f32":0.0,"f64":1.0}"#)]
#[case::negative_one(-1.0, r#"{"f32":0.0,"f64":-1.0}"#)]
#[case::min_positive(f64::MIN_POSITIVE, r#"{"f32":0.0,"f64":2.2250738585072014e-308}"#)]
#[case::f32_nan(5.1042355e38, r#"{"f32":0.0,"f64":5.1042355e+38}"#)]
#[case::f32_negative_nan(-5.1042355e38, r#"{"f32":0.0,"f64":-5.1042355e+38}"#)]
#[case::f32_infinity(3.4028237e38, r#"{"f32":0.0,"f64":3.4028237e+38}"#)]
#[case::f32_negative_infinity(-3.4028237e38, r#"{"f32":0.0,"f64":-3.4028237e+38}"#)]
#[case::min(f64::MIN, r#"{"f32":0.0,"f64":-1.7976931348623157e+308}"#)]
#[case::max(f64::MAX, r#"{"f32":0.0,"f64":1.7976931348623157e+308}"#)]
#[case::infinity(f64::INFINITY, r#"{"f32":0.0,"f64":"Infinity"}"#)]
#[case::negative_infinity(f64::NEG_INFINITY, r#"{"f32":0.0,"f64":"-Infinity"}"#)]
#[case::nan(f64::NAN, r#"{"f32":0.0,"f64":"NaN"}"#)]
#[case::negative_nan(-f64::NAN, r#"{"f32":0.0,"f64":"NaN"}"#)]
fn test_f64_round_trip(#[case] value: f64, #[case] expected_json: &str) -> Result<()> {
    let fp = FloatingPoint {
        f64: value,
        ..Default::default()
    };

    let s = String::from_utf8(serialize(&fp).to_vec()).unwrap();
    assert_eq!(s, expected_json);
    let d: FloatingPoint = deserialize(s).unwrap();

    if value.is_nan() {
        assert!(d.f64.is_nan(), "value: {}", d.f64);
    } else {
        assert_relative_eq!(d.f64, fp.f64);
    }

    Ok(())
}

#[rstest]
#[case::nan(r#"{ "f64": "NaN" }"#, f64::NAN)]
#[case::negative_nan(r#"{ "f64": "-NaN" }"#, -f64::NAN)]
#[case::infinity(r#"{ "f64": "Infinity" }"#, f64::INFINITY)]
#[case::negative_infinity(r#"{ "f64": "-Infinity" }"#, f64::NEG_INFINITY)]
#[case::ryu_nan(r#"{ "f64": "2.696539702293474e308" }"#, f64::NAN)]
#[case::ryu_negative_nan(r#"{ "f64": "-2.696539702293474e308" }"#, -f64::NAN)]
#[case::ryu_infinity(r#"{ "f64": "1.797693134862316e308" }"#, f64::INFINITY)]
#[case::ryu_negative_infinity(r#"{ "f64": "-1.797693134862316e308" }"#, f64::NEG_INFINITY)]
fn test_f64_deserialize(#[case] json: &str, #[case] expected_value: f64) -> Result<()> {
    let d: FloatingPoint = deserialize(json).unwrap();

    if expected_value.is_nan() {
        assert!(d.f64.is_nan(), "value: {}", d.f64);
    } else {
        assert_relative_eq!(d.f64, expected_value);
    }

    Ok(())
}

#[rstest]
#[case::unqoted_nan_f32(r#"{"f32":NaN}"#)]
#[case::unqoted_nan_f64(r#"{"f64":NaN}"#)]
#[case::unqoted_negative_nan_f32(r#"{"f32":-NaN}"#)]
#[case::unqoted_negative_nan_f64(r#"{"f64":-NaN}"#)]
#[case::unqoted_infinity_f32(r#"{"f32":Infinity}"#)]
#[case::unqoted_infinity_f64(r#"{"f64":Infinity}"#)]
#[case::unqoted_negative_infinity_f32(r#"{"f32":-Infinity}"#)]
#[case::unqoted_negative_infinity_f64(r#"{"f64":-Infinity}"#)]
fn test_invalid_floating_point(#[case] json: &str) {
    let res: Result<FloatingPoint> = deserialize(json);
    assert!(res.is_err(), "Expected error, got {:?}", res);
}

#[test]
fn test_map_end_detection() -> Result<()> {
    let c = Containers {
        m: [("k1".into(), "v1".into()), ("k2".into(), "v2".into())]
            .into_iter()
            .collect(),
        ..Default::default()
    };
    let s = serialize(&c);
    assert_eq!(c, deserialize(s).unwrap());
    Ok(())
}

#[test]
fn test_single_entry_map() -> Result<()> {
    let c = Containers {
        m: [("only".into(), "one".into())].into_iter().collect(),
        ..Default::default()
    };
    let s = serialize(&c);
    assert_eq!(c, deserialize(s).unwrap());
    Ok(())
}

#[test]
fn test_empty_map() -> Result<()> {
    let c = Containers {
        m: Default::default(),
        ..Default::default()
    };
    let s = serialize(&c);
    assert_eq!(c, deserialize(s).unwrap());
    Ok(())
}

#[test]
fn test_list_end_detection() -> Result<()> {
    let c = Containers {
        l: vec!["a".into(), "b".into(), "c".into()],
        ..Default::default()
    };
    let s = serialize(&c);
    assert_eq!(c, deserialize(s).unwrap());
    Ok(())
}

#[test]
fn test_single_entry_list() -> Result<()> {
    let c = Containers {
        l: vec!["only".into()],
        ..Default::default()
    };
    let s = serialize(&c);
    assert_eq!(c, deserialize(s).unwrap());
    Ok(())
}

#[test]
fn test_empty_list() -> Result<()> {
    let c = Containers {
        l: Default::default(),
        ..Default::default()
    };
    let s = serialize(&c);
    assert_eq!(c, deserialize(s).unwrap());
    Ok(())
}

#[test]
fn test_struct_field_end_detection() -> Result<()> {
    let s = Small {
        num: 42,
        two: 100,
        ..Default::default()
    };
    let serialized = serialize(&s);
    assert_eq!(s, deserialize(serialized).unwrap());
    Ok(())
}

#[test]
fn test_nested_containers_end_detection() -> Result<()> {
    let mut m = BTreeMap::new();
    m.insert("a".to_string(), 1);
    m.insert("b".to_string(), 2);
    let r = MainStruct {
        m,
        l: vec![
            Small {
                num: 1,
                two: 2,
                ..Default::default()
            },
            Small {
                num: 3,
                two: 4,
                ..Default::default()
            },
        ],
        int_keys: [(10, 20), (30, 40)].into_iter().collect(),
        ..Default::default()
    };
    let serialized = serialize(&r);
    assert_eq!(r, deserialize(serialized).unwrap());
    Ok(())
}

#[test]
fn test_map_with_whitespace_before_end() -> Result<()> {
    let input = r#"{"m":{"k1":"v1" , "k2":"v2"  }}"#;
    let c: Containers = deserialize(input).unwrap();
    assert_eq!(c.m.len(), 2);
    assert_eq!(c.m.get("k1"), Some(&"v1".to_string()));
    assert_eq!(c.m.get("k2"), Some(&"v2".to_string()));
    Ok(())
}

#[test]
fn test_list_with_whitespace_before_end() -> Result<()> {
    let input = r#"{"l":["a" , "b"  ]}"#;
    let c: Containers = deserialize(input).unwrap();
    assert_eq!(c.l, vec!["a".to_string(), "b".to_string()]);
    Ok(())
}

#[test]
fn test_bool_peek_based_parsing() -> Result<()> {
    let b = Basic {
        b: true,
        b2: false,
        ..Default::default()
    };
    let s = serialize(&b);
    assert_eq!(b, deserialize(s).unwrap());

    let b2 = Basic {
        b: false,
        b2: true,
        ..Default::default()
    };
    let s2 = serialize(&b2);
    assert_eq!(b2, deserialize(s2).unwrap());
    Ok(())
}

#[test]
fn test_bool_with_whitespace() -> Result<()> {
    let input = r#"{"b":  true  ,"b2":  false  }"#;
    let b: Basic = deserialize(input).unwrap();
    assert!(b.b);
    assert!(!b.b2);
    Ok(())
}

#[test]
fn test_bool_invalid_value() {
    let input = r#"{"b": 1}"#;
    assert!(deserialize::<Basic, _, _>(input).is_err());
}

#[test]
fn test_missing_comma_between_map_entries() {
    let input = r#"{"m":{"k1":"v1""k2":"v2"}}"#;
    assert!(deserialize::<Containers, _, _>(input).is_err());
}

#[test]
fn test_missing_comma_between_list_entries() {
    let input = r#"{"l":["a""b"]}"#;
    assert!(deserialize::<Containers, _, _>(input).is_err());
}

#[test]
fn test_trailing_comma_in_map() {
    let input = r#"{"m":{"k1":"v1",}}"#;
    assert!(deserialize::<Containers, _, _>(input).is_err());
}

#[test]
fn test_trailing_comma_in_list() {
    let input = r#"{"l":["a",]}"#;
    assert!(deserialize::<Containers, _, _>(input).is_err());
}

proptest! {
#[test]
fn test_prop_serialize_deserialize(s in gen_main_struct()) {
    let processed = deserialize(serialize(&s)).unwrap();
    prop_assert_eq!(s, processed);
}
}
