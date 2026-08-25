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

use anyhow::Result;
use fbthrift::GetTypeNameType;
use fbthrift::TypeNameType;
use fbthrift::compact_protocol;
use standard::TypeName;
use standard::TypeUri;
use test_structs::Basic;
use test_structs::SimpleEnum;
use test_structs::SimpleUnion;

#[test]
fn test_basic() -> Result<()> {
    //
    // initialize a thrift struct to use during testing
    //
    let basic = Basic {
        x: 10,
        y: "hello_world".to_string(),
        z: vec!["a".to_string(), "b".to_string()],
        ..Default::default()
    };

    //
    // test a round trip
    //
    let any = thrift_any::serialize(&basic);
    let basic2 = thrift_any::deserialize(&any)?;
    assert_eq!(basic, basic2);

    // verify the serialized binary data:
    assert_eq!(any.data, compact_protocol::serialize(&basic).to_vec());
    // verify everything else in the AnyStruct:
    let mut any = any;
    any.data = vec![];
    let expected = "AnyStruct {\
                  \n    r#type: TypeStruct {\
                  \n        name: structType(\
                  \n            uri(\
                  \n                \"facebook.com/icsp/new_any/Basic\",\
                  \n            ),\
                  \n        ),\
                  \n        params: [],\
                  \n    },\
                  \n    protocol: standard(\
                  \n        StandardProtocol::Compact,\
                  \n    ),\
                  \n    data: [],\
                  \n}";
    assert_eq!(format!("{:#?}", any), expected);
    Ok(())
}

#[test]
fn test_basic_json() -> Result<()> {
    let basic = Basic {
        x: 20,
        y: "hello_world2".to_string(),
        z: vec!["d".to_string(), "e".to_string()],
        ..Default::default()
    };

    let any = thrift_any::serialize_json(&basic);
    let basic2 = thrift_any::deserialize(&any)?;
    assert_eq!(basic, basic2);
    Ok(())
}

#[test]
fn test_get_uri_struct_type() -> Result<()> {
    let basic = Basic {
        x: 10,
        y: "hello_world".to_string(),
        z: vec!["a".to_string(), "b".to_string()],
        ..Default::default()
    };

    let any = thrift_any::serialize_json(&basic);
    let uri = thrift_any::get_uri(&any).unwrap();
    assert_eq!("facebook.com/icsp/new_any/Basic".to_string(), uri);
    Ok(())
}

#[test]
fn test_get_uri_union_type() -> Result<()> {
    let simple_union = SimpleUnion::x(20);

    let any = thrift_any::serialize_json(&simple_union);
    let uri = thrift_any::get_uri(&any).unwrap();
    assert_eq!("facebook.com/icsp/new_any/SimpleUnion".to_string(), uri);
    Ok(())
}

#[test]
fn test_enum_compact_roundtrip() -> Result<()> {
    let value = SimpleEnum::VARIANT2;

    let any = thrift_any::serialize(&value);

    assert!(matches!(
        SimpleEnum::type_name_type(),
        TypeNameType::EnumType
    ));
    assert_eq!(
        any.r#type.name,
        TypeName::enumType(TypeUri::uri(
            "facebook.com/icsp/new_any/SimpleEnum".to_string()
        ))
    );
    assert_eq!(any.data, compact_protocol::serialize(value).to_vec());
    let uri = thrift_any::get_uri(&any).unwrap();
    assert_eq!("facebook.com/icsp/new_any/SimpleEnum".to_string(), uri);
    assert_eq!(thrift_any::deserialize::<SimpleEnum>(&any)?, value);
    Ok(())
}

#[test]
fn test_enum_json_roundtrip() -> Result<()> {
    let value = SimpleEnum::VARIANT1;

    let any = thrift_any::serialize_json(&value);

    assert_eq!(thrift_any::deserialize::<SimpleEnum>(&any)?, value);
    Ok(())
}

#[test]
fn test_enum_type_checking() {
    let mut any = thrift_any::serialize(&SimpleEnum::VARIANT1);

    assert!(thrift_any::is_type::<SimpleEnum>(&any));
    assert!(!thrift_any::is_type::<Basic>(&any));

    any.r#type.name = TypeName::enumType(TypeUri::uri(
        "facebook.com/icsp/new_any/OtherEnum".to_string(),
    ));
    assert!(!thrift_any::is_type::<SimpleEnum>(&any));
    assert!(thrift_any::deserialize::<SimpleEnum>(&any).is_err());
}
