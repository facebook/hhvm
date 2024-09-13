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
use cxx::let_cxx_string;
use fbthrift::compact_protocol;
use test_structs::Basic;
use test_structs::SimpleUnion;

use crate::bridge::ffi;

#[test]
fn test_cpp_basic_to_any() -> Result<()> {
    let basic = Basic {
        x: 10,
        y: "hello_world".to_string(),
        z: vec!["a".to_string(), "b".to_string()],
        ..Default::default()
    };
    //
    // use C++ to convert from Basic to Any
    //
    let_cxx_string!(compact_basic = compact_protocol::serialize(&basic).to_vec());
    let compact_any = ffi::basic_to_any(&compact_basic);
    let any: any::Any = compact_protocol::deserialize(compact_any.as_ref().unwrap().as_bytes())?;
    //
    // deserialize the Any back to Basic
    //
    let basic2: Basic = thrift_any::deserialize(&any)?;
    //
    // verify roundtrip
    //
    assert_eq!(basic, basic2);
    Ok(())
}

#[test]
fn test_cpp_any_to_basic() -> Result<()> {
    let basic = Basic {
        x: 10,
        y: "hello_world".to_string(),
        z: vec!["a".to_string(), "b".to_string()],
        ..Default::default()
    };
    //
    // convert to Any
    //
    let any = thrift_any::serialize(&basic);
    //
    // use C++ to convert Any to Basic
    //
    let_cxx_string!(compact_any = compact_protocol::serialize(any).to_vec());
    let compact_basic = ffi::any_to_basic(&compact_any);
    let basic2: Basic = compact_protocol::deserialize(compact_basic.as_ref().unwrap().as_bytes())?;
    //
    // verify roundtrip
    //
    assert_eq!(basic, basic2);
    Ok(())
}

#[test]
fn test_cpp_simple_union_to_any() -> Result<()> {
    let simple_union = SimpleUnion::basic(Basic {
        x: 10,
        y: "hello_world".to_string(),
        z: vec!["a".to_string(), "b".to_string()],
        ..Default::default()
    });
    //
    // use C++ to convert from Basic to Any
    //
    let_cxx_string!(compact_simple_union = compact_protocol::serialize(&simple_union).to_vec());
    let compact_any = ffi::simple_union_to_any(&compact_simple_union);
    let any: any::Any = compact_protocol::deserialize(compact_any.as_ref().unwrap().as_bytes())?;
    //
    // deserialize the Any back to Basic
    //
    let simple_union2: SimpleUnion = thrift_any::deserialize(&any)?;
    //
    // verify roundtrip
    //
    assert_eq!(simple_union, simple_union2);
    Ok(())
}

#[test]
fn test_cpp_any_to_simple_union() -> Result<()> {
    let simple_union = SimpleUnion::basic(Basic {
        x: 42,
        y: "hello_world2".to_string(),
        z: vec!["d".to_string(), "f".to_string()],
        ..Default::default()
    });
    //
    // convert to Any
    //
    let any = thrift_any::serialize(&simple_union);
    //
    // use C++ to convert Any to Basic
    //
    let_cxx_string!(compact_any = compact_protocol::serialize(any).to_vec());
    let compact_simple_union = ffi::any_to_simple_union(&compact_any);
    let simple_union2: SimpleUnion =
        compact_protocol::deserialize(compact_simple_union.as_ref().unwrap().as_bytes())?;
    //
    // verify roundtrip
    //
    assert_eq!(simple_union, simple_union2);
    Ok(())
}

/*#[test]
fn test_compression() -> Result<()> {
    let basic = Basic {
        x: 10,
        y: "hello_world".to_string(),
        z: vec!["a".to_string(), "b".to_string()],
        ..Default::default()
    };

    {
        let_cxx_string!(compact_basic = compact_protocol::serialize(&basic).to_vec());
        let compact_any = ffi::basic_to_any(&compact_basic);
        let compressed_any_serialized = ffi::compress_any(&compact_any);
        let compressed_any: any::Any =
            compact_protocol::deserialize(compressed_any_serialized.as_ref().unwrap().as_bytes())?;

        let basic2: Basic = thrift_any::deserialize(&compressed_any)?;

        assert_eq!(basic, basic2);
    }

    {
        let any = thrift_any::serialize(&basic);
        let compressed_any = thrift_any::compress_any(&any)?;

        let basic2: Basic = thrift_any::deserialize(&compressed_any)?;
        assert_eq!(basic, basic2);

        let_cxx_string!(
            compact_compressed_any = compact_protocol::serialize(&compressed_any).to_vec()
        );
        let any_serialized = ffi::decompress_any(&compact_compressed_any);
        let compact_basic = ffi::any_to_basic(&any_serialized);
        let basic3: Basic =
            compact_protocol::deserialize(compact_basic.as_ref().unwrap().as_bytes())?;
        assert_eq!(basic, basic3);
    }

    Ok(())
}*/
