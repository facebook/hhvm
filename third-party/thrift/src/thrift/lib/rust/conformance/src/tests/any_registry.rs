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
use fbthrift_conformance::AnyRegistry;
use maplit::btreemap;
use protocol::StandardProtocol;
use testset::struct_map_string_i32;

fn test_for_protocol(protocol: StandardProtocol) -> Result<()> {
    let mut registry = AnyRegistry::new();

    registry.register_type::<struct_map_string_i32>()?;

    let original = struct_map_string_i32 {
        field_1: btreemap! {"Answer to the Ultimate Question of Life, the Universe, and Everything.".to_owned() => 42},
        ..Default::default()
    };

    let any_obj = registry.store(&original, protocol)?;

    assert!(any_obj.typeHashPrefixSha2_256.is_some());
    assert_eq!(protocol, any_obj.protocol.unwrap());

    let loaded = registry.load(&any_obj)?;

    assert_eq!(original, loaded);

    Ok(())
}

#[test]
fn test_round_trip() -> Result<()> {
    let protocols = vec![
        StandardProtocol::Binary,
        StandardProtocol::Compact,
        StandardProtocol::SimpleJson,
    ];

    for protocol in protocols {
        test_for_protocol(protocol)?;
    }

    Ok(())
}
