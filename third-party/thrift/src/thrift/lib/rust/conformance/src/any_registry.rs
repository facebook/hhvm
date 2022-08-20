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

use std::any::TypeId;
use std::collections::HashMap;
use std::collections::HashSet;

use anyhow::anyhow;
use anyhow::bail;
use anyhow::ensure;
use anyhow::Result;
use fbthrift::binary_protocol;
use fbthrift::compact_protocol;
use fbthrift::simplejson_protocol;
use fbthrift::GetUri;
use protocol::StandardProtocol;

use crate::universal_name::ensure_registered;
use crate::universal_name::get_universal_hash;
use crate::universal_name::get_universal_hash_prefix;
use crate::universal_name::UniversalHashAlgorithm;

pub struct AnyRegistry {
    uri_to_typeid: HashMap<&'static str, TypeId>,
    typeid_to_uri: HashMap<TypeId, &'static str>,
    alg_to_hashes: HashMap<UniversalHashAlgorithm, HashSet<Vec<u8>>>,
}

pub trait SerializeRef = binary_protocol::SerializeRef
    + compact_protocol::SerializeRef
    + simplejson_protocol::SerializeRef;
pub trait DeserializeSlice = binary_protocol::DeserializeSlice
    + compact_protocol::DeserializeSlice
    + simplejson_protocol::DeserializeSlice;

impl AnyRegistry {
    pub fn new() -> Self {
        let mut alg_to_hashes = HashMap::new();
        alg_to_hashes.insert(UniversalHashAlgorithm::Sha2_256, HashSet::new());

        Self {
            uri_to_typeid: HashMap::new(),
            typeid_to_uri: HashMap::new(),
            alg_to_hashes,
        }
    }

    pub fn register_type<T: 'static + GetUri>(&mut self) -> Result<bool> {
        let uri = T::uri();
        let type_id = TypeId::of::<T>();
        if self.uri_to_typeid.contains_key(uri) || self.typeid_to_uri.contains_key(&type_id) {
            return Ok(false);
        }
        self.uri_to_typeid.insert(uri, type_id);
        self.typeid_to_uri.insert(type_id, uri);
        for (alg, hashes) in self.alg_to_hashes.iter_mut() {
            let hash = get_universal_hash(*alg, uri)?;
            hashes.insert(hash);
        }
        Ok(true)
    }

    pub fn store<T: 'static + SerializeRef>(
        &mut self,
        obj: &T,
        protocol: StandardProtocol,
    ) -> Result<any::Any> {
        let type_id = TypeId::of::<T>();
        let uri = self
            .typeid_to_uri
            .get(&type_id)
            .ok_or_else(|| anyhow!("Type {:?} not registered", type_id))?;
        let hash = get_universal_hash(UniversalHashAlgorithm::Sha2_256, uri)?;
        let hash_prefix = get_universal_hash_prefix(&hash, 16);
        Ok(any::Any {
            r#type: Some(uri.to_string()),
            typeHashPrefixSha2_256: Some(hash_prefix),
            protocol: Some(protocol),
            data: serialize(obj, protocol)?,
            ..Default::default()
        })
    }

    pub fn load<T: DeserializeSlice>(&self, obj: &any::Any) -> Result<T> {
        if let Some(type_hash_prefix_sha2_256) = &obj.typeHashPrefixSha2_256 {
            let hashes = self
                .alg_to_hashes
                .get(&UniversalHashAlgorithm::Sha2_256)
                .ok_or_else(|| anyhow!("Hash algorithm Sha2_256 not supported"))?;
            ensure_registered(hashes, type_hash_prefix_sha2_256)?;
        } else if let Some(uri) = obj.r#type.as_ref() {
            ensure!(
                self.uri_to_typeid.contains_key(uri.as_str()),
                "Type '{}' not registered",
                uri
            );
        } else {
            bail!("No type information found");
        }
        deserialize(&obj.data, obj.protocol.unwrap_or(StandardProtocol::Compact))
    }
}

fn serialize<T: SerializeRef>(obj: &T, protocol: StandardProtocol) -> Result<Vec<u8>> {
    match protocol {
        StandardProtocol::Binary => Ok(binary_protocol::serialize(obj).to_vec()),
        StandardProtocol::Compact => Ok(compact_protocol::serialize(obj).to_vec()),
        StandardProtocol::SimpleJson => Ok(simplejson_protocol::serialize(obj).to_vec()),
        unsupported => Err(anyhow!("Unsupported protocol: {:?}", unsupported)),
    }
}

fn deserialize<T: DeserializeSlice>(data: &[u8], protocol: StandardProtocol) -> Result<T> {
    match protocol {
        StandardProtocol::Binary => binary_protocol::deserialize(data),
        StandardProtocol::Compact => compact_protocol::deserialize(data),
        StandardProtocol::SimpleJson => simplejson_protocol::deserialize(data),
        unsupported => Err(anyhow!("Unsupported protocol: {:?}", unsupported)),
    }
}

#[cfg(test)]
mod tests {
    use anyhow::Result;
    use maplit::btreemap;
    use testset::struct_map_string_i32;

    use super::*;

    fn get_test_object() -> struct_map_string_i32 {
        struct_map_string_i32 {
            field_1: btreemap! {"Answer to the Ultimate Question of Life, the Universe, and Everything.".to_owned() => 42},
            ..Default::default()
        }
    }

    fn test_serialize_round_trip_for_protocol(protocol: StandardProtocol) -> Result<()> {
        let original = get_test_object();

        let serialized = serialize(&original, protocol)?;

        let deserialized = deserialize(&serialized, protocol)?;

        assert_eq!(original, deserialized);

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
            test_serialize_round_trip_for_protocol(protocol)?;
        }

        Ok(())
    }

    #[test]
    fn test_unsupported_protocol() {
        let test_object = get_test_object();

        assert!(serialize(&test_object, StandardProtocol::Custom).is_err());
        assert!(serialize(&test_object, StandardProtocol::Json).is_err());

        assert!(deserialize::<struct_map_string_i32>(b"", StandardProtocol::Custom).is_err());
        assert!(deserialize::<struct_map_string_i32>(b"", StandardProtocol::Json).is_err());
    }
}
