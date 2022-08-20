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
use fbthrift::simplejson_protocol::deserialize;
use fbthrift::simplejson_protocol::serialize;
use fbthrift::simplejson_protocol::SimpleJsonProtocolDeserializer;
use fbthrift::Deserialize;
use fbthrift_test_if::Basic;
use fbthrift_test_if::Containers;
use fbthrift_test_if::En;
use fbthrift_test_if::MainStruct;
use fbthrift_test_if::MainStructNoBinary;
use fbthrift_test_if::Small;
use fbthrift_test_if::SubStruct;
use fbthrift_test_if::Un;
use fbthrift_test_if::UnOne;
use proptest::prelude::*;

use crate::proptest::gen_main_struct;

#[test]
fn test_large_roundtrip() -> Result<()> {
    // Build the big struct
    let mut m = BTreeMap::new();
    m.insert("m1".to_string(), 1);
    m.insert("m2".to_string(), 2);

    let sub = SubStruct {
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

    // Note that default optionals are there
    // but non-default optionals are not there
    // That is an artifact on how the serialize trait works
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
    .replace(" ", "")
    .replace("\n", "");
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
    .replace(" ", "")
    .replace("\n", "");
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
    .replace(" ", "")
    .replace("\n", "");
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
    .replace("\n", "");
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
    .replace(" ", "")
    .replace("\n", "");
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
    .replace(" ", "")
    .replace("\n", "");
    assert!(deserialize::<Small, _, _>(input).is_err());

    // even when skipping
    let input2 = r#"{
        "num":1,
        "two":2,
        "extra_map":{"thing":null,"thing2":2}
        "extra_bool":true
    }"#
    .replace(" ", "")
    .replace("\n", "");
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
fn test_null_stuff() -> Result<()> {
    // See the `nullStuff` test in cpp_compat_test

    let sub = SubStruct {
        optDef: None,
        bin: "1234".as_bytes().to_vec(),
        ..Default::default()
    };

    let input = r#"{
        "optDef":null,
        "req_def":"IAMREQ",
        "bin":"MTIzNA"
    }"#
    .replace(" ", "")
    .replace("\n", "");
    // Make sure everything is skipped properly
    assert_eq!(sub, deserialize(input).unwrap());

    Ok(())
}

#[test]
fn infinite_spaces() -> Result<()> {
    let mut m = BTreeMap::new();
    m.insert("m1".to_string(), 1);
    m.insert("m2".to_string(), 2);

    let sub = SubStruct {
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
        "s":{"optDef":  "IAMOPT"  ,"req_def":  "IAMREQ","bin": ""  },
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
    // can deserialize
    // what serde has written out
    let serde_s = serde_json::to_string(&r).unwrap();

    // but passing between them should work
    assert_eq!(r, serde_json::from_str(&fbthrift_s).unwrap());
    assert_eq!(r, deserialize(&serde_s).unwrap());
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
    assert_eq!(b1, Basic::read(&mut deserializer)?);
    assert_eq!(b2, Basic::read(&mut deserializer)?);

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
    let s2 = String::from_utf8(serialize(&b2).to_vec()).unwrap();
    let to_check = format!("{} {}", s1, s2);

    let mut deserializer = SimpleJsonProtocolDeserializer::new(Cursor::new(
        // 6 should cover the } and a `true` value
        &to_check.as_bytes()[..to_check.as_bytes().len() - 6],
    ));
    // Assert that deserialize builts the exact same struct
    assert_eq!(b1, Basic::read(&mut deserializer)?);
    assert!(Basic::read(&mut deserializer).is_err());

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
    let s2 = String::from_utf8(serialize(&explicit_unknown).to_vec()).unwrap();
    let expected_string = "{}";
    assert_eq!(expected_string, s2);

    // Deserializes to the default -1 case, this matches the other
    // protocols behavior
    assert_eq!(u, deserialize(s2).unwrap());

    // backwards compat test
    let old_output = r#"{
        "UnknownField":-1
    }"#
    .replace(" ", "")
    .replace("\n", "");

    assert_eq!(u, deserialize(old_output).unwrap());

    Ok(())
}

proptest! {
#[test]
fn test_prop_serialize_deserialize(s in gen_main_struct()) {
    let processed = deserialize(serialize(&s)).unwrap();
    prop_assert_eq!(s, processed);
}
}
