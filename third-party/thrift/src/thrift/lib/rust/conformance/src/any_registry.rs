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
use anyhow::Context;
use anyhow::Result;
use fbthrift::binary_protocol;
use fbthrift::compact_protocol;
use fbthrift::simplejson_protocol;
use fbthrift::GetUri;
use itertools::Itertools;
use protocol::StandardProtocol;

use crate::universal_name::ensure_registered;
use crate::universal_name::get_universal_hash;
use crate::universal_name::get_universal_hash_prefix_sha_256;
use crate::universal_name::UniversalHashAlgorithm;

const UNIVERSAL_HASH_PREFIX_SHA_256_LEN: i8 = 16;

struct AnySerDeser {
    serialize: fn(Box<dyn std::any::Any>, StandardProtocol) -> Result<Vec<u8>>,
    deserialize: fn(&[u8], StandardProtocol) -> Result<Box<dyn std::any::Any>>,
}

pub struct AnyRegistry {
    uri_to_typeid: HashMap<&'static str, TypeId>,
    typeid_to_uri: HashMap<TypeId, &'static str>,
    hash_prefix_to_typeid: HashMap<Vec<u8>, TypeId>,
    alg_to_hashes: HashMap<UniversalHashAlgorithm, HashSet<Vec<u8>>>,
    typeid_to_serializers: HashMap<TypeId, AnySerDeser>,
}

pub trait SerializeRef = binary_protocol::SerializeRef
    + compact_protocol::SerializeRef
    + simplejson_protocol::SerializeRef;
pub trait DeserializeSlice = binary_protocol::DeserializeSlice
    + compact_protocol::DeserializeSlice
    + simplejson_protocol::DeserializeSlice;

impl std::fmt::Debug for AnyRegistry {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let mut map = f.debug_map();
        for uri in self.uri_to_typeid.keys().sorted() {
            let hp =
                get_universal_hash_prefix_sha_256(uri, UNIVERSAL_HASH_PREFIX_SHA_256_LEN).unwrap();
            map.entry(uri, &hex::encode(hp));
        }
        map.finish()
    }
}

impl AnyRegistry {
    pub fn new() -> Self {
        let mut alg_to_hashes = HashMap::new();
        alg_to_hashes.insert(UniversalHashAlgorithm::Sha2_256, HashSet::new());

        Self {
            uri_to_typeid: HashMap::new(),
            typeid_to_uri: HashMap::new(),
            hash_prefix_to_typeid: HashMap::new(),
            typeid_to_serializers: HashMap::new(),
            alg_to_hashes,
        }
    }

    pub fn register_type<T: 'static + GetUri + SerializeRef + DeserializeSlice>(
        &mut self,
    ) -> Result<bool> {
        let uri = T::uri();
        let hash_prefix =
            get_universal_hash_prefix_sha_256(uri, UNIVERSAL_HASH_PREFIX_SHA_256_LEN)?;
        let type_id = TypeId::of::<T>();

        if self.uri_to_typeid.contains_key(uri)
            || self.hash_prefix_to_typeid.contains_key(&hash_prefix)
        {
            return Ok(false);
        }
        self.uri_to_typeid.insert(uri, type_id);
        self.typeid_to_uri.insert(type_id, uri);
        self.hash_prefix_to_typeid.insert(hash_prefix, type_id);

        for (alg, hashes) in self.alg_to_hashes.iter_mut() {
            let hash = get_universal_hash(*alg, uri)?;
            hashes.insert(hash);
        }
        self.typeid_to_serializers.insert(
            type_id,
            AnySerDeser {
                serialize: |x, prot| {
                    x.downcast::<T>().map_or_else(
                        |_| bail!("bad any (\"{}\" expected)", T::uri()),
                        |t| serialize::<T>(&t, prot),
                    )
                },
                deserialize: |bytes, prot| {
                    deserialize::<T>(bytes, prot).map(|t| Box::new(t) as Box<dyn std::any::Any>)
                },
            },
        );
        Ok(true)
    }

    pub fn num_registered_types(&self) -> usize {
        self.uri_to_typeid.len()
    }

    pub fn has_type<T: 'static + GetUri>(&self, obj: &any::Any) -> Result<bool> {
        let type_uri = T::uri();
        let type_hash_prefix_sha2_256 =
            get_universal_hash_prefix_sha_256(type_uri, UNIVERSAL_HASH_PREFIX_SHA_256_LEN)?;
        if let Some(obj_hash_prefix_sha2_256) = &obj.typeHashPrefixSha2_256 {
            Ok(type_hash_prefix_sha2_256 == *obj_hash_prefix_sha2_256)
        } else if let Some(obj_uri) = obj.r#type.as_ref() {
            Ok(type_uri == obj_uri)
        } else {
            bail!("No type information found");
        }
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
        let hash_prefix =
            get_universal_hash_prefix_sha_256(uri, UNIVERSAL_HASH_PREFIX_SHA_256_LEN)?;
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

    pub fn load_(&self, val: &any::Any) -> Result<Box<dyn std::any::Any>> {
        let AnySerDeser { deserialize, .. } = self.serializers(val)?;
        deserialize(&val.data, val.protocol.unwrap_or(StandardProtocol::Compact))
    }

    pub fn store_(
        &self,
        val: Box<dyn std::any::Any>,
        protocol: StandardProtocol,
    ) -> Result<Vec<u8>> {
        let typeid = (*val).type_id();
        let AnySerDeser { serialize, .. } = self
            .typeid_to_serializers
            .get(&typeid)
            .context("serializers lookup failure")?;
        serialize(val, protocol)
    }

    fn serializers_given_uri(&self, uri: &str) -> Result<&AnySerDeser> {
        self.typeid_to_serializers
            .get(
                self.uri_to_typeid
                    .get(uri)
                    .context("typeid lookup failure")?,
            )
            .context("serializers lookup failure")
    }

    fn serializers_given_hash_prefix(&self, hash_prefix: &Vec<u8>) -> Result<&AnySerDeser> {
        self.typeid_to_serializers
            .get(
                self.hash_prefix_to_typeid
                    .get(hash_prefix)
                    .context("typeid lookup failure")?,
            )
            .context("serializers lookup failure")
    }

    fn serializers(&self, any: &any::Any) -> Result<&AnySerDeser> {
        match (any.r#type.as_ref(), any.typeHashPrefixSha2_256.as_ref()) {
            (Some(uri), _) => self.serializers_given_uri(uri),
            (_, Some(hash_prefix)) => self.serializers_given_hash_prefix(hash_prefix),
            (None, None) => Err(anyhow!("neither uri or hash prefix given ")),
        }
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
    use testset::union_map_string_i32;

    use super::*;

    #[test]
    fn test_debug() -> Result<()> {
        let mut registry = AnyRegistry::new();
        registry.register_type::<union_map_string_i32>().unwrap();
        registry.register_type::<struct_map_string_i32>().unwrap();
        let expected = r#"{
    "facebook.com/thrift/test/testset/struct_map_string_i32": "d4252c01f98d688ebb155e0a0a316763",
    "facebook.com/thrift/test/testset/union_map_string_i32": "35f8820d6ded47c7d1611dd76a002040",
}"#;
        assert_eq!(format!("{:#?}", &registry), expected);

        Ok(())
    }

    fn get_test_object() -> struct_map_string_i32 {
        struct_map_string_i32 {
            field_1: btreemap! {"Answer to the Ultimate Question of Life, the Universe, and Everything.".to_owned() => 42},
            ..Default::default()
        }
    }

    fn get_test_protocols() -> Vec<StandardProtocol> {
        vec![
            StandardProtocol::Binary,
            StandardProtocol::Compact,
            StandardProtocol::SimpleJson,
        ]
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
        for protocol in get_test_protocols() {
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

    fn test_store_load_for_protocol(protocol: StandardProtocol) -> Result<()> {
        let mut registry = AnyRegistry::new();
        registry.register_type::<struct_map_string_i32>()?;

        let original = get_test_object();
        let any_obj = registry.store(&original, protocol)?;
        assert!(any_obj.typeHashPrefixSha2_256.is_some());
        assert_eq!(protocol, any_obj.protocol.unwrap());

        let loaded = registry.load(&any_obj)?;
        assert_eq!(original, loaded);
        Ok(())
    }

    #[test]
    fn test_round_trip_through_store_load() -> Result<()> {
        for protocol in get_test_protocols() {
            test_store_load_for_protocol(protocol)?;
        }
        Ok(())
    }

    #[test]
    fn test_round_trip_through_any_via_uri() -> Result<()> {
        let mut any_registry = AnyRegistry::new();
        any_registry.register_type::<struct_map_string_i32>()?;

        let uri = struct_map_string_i32::uri();
        let AnySerDeser {
            serialize,
            deserialize,
        } = any_registry.serializers_given_uri(uri)?;

        let obj = get_test_object();
        for protocol in get_test_protocols() {
            let any: Box<dyn std::any::Any> = Box::new(obj.clone());
            let bytes = serialize(any, protocol)?;
            assert!(!bytes.is_empty());
            let val = *(deserialize(&bytes, protocol)?
                .downcast::<struct_map_string_i32>()
                .map_err(|_| anyhow::Error::msg("bad any cast")))?;
            assert_eq!(val, obj);
        }

        Ok(())
    }

    #[test]
    fn test_round_trip_through_any_via_hash_prefix() -> Result<()> {
        let mut any_registry = AnyRegistry::new();
        any_registry.register_type::<struct_map_string_i32>()?;

        let hash_prefix = get_universal_hash_prefix_sha_256(
            struct_map_string_i32::uri(),
            UNIVERSAL_HASH_PREFIX_SHA_256_LEN,
        )?;
        let AnySerDeser {
            serialize,
            deserialize,
        } = any_registry.serializers_given_hash_prefix(&hash_prefix)?;

        let obj = get_test_object();
        for protocol in get_test_protocols() {
            let any: Box<dyn std::any::Any> = Box::new(obj.clone());
            let bytes = serialize(any, protocol)?;
            assert!(!bytes.is_empty());
            let val = *(deserialize(&bytes, protocol)?
                .downcast::<struct_map_string_i32>()
                .map_err(|_| anyhow::Error::msg("bad any cast")))?;
            assert_eq!(val, obj);
        }

        Ok(())
    }
}
