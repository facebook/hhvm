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

use anyhow::Context;
use anyhow::Result;
use anyhow::anyhow;
use anyhow::bail;
use fbthrift::GetUri;
use fbthrift::binary_protocol;
use fbthrift::compact_protocol;
use fbthrift::simplejson_protocol;
use itertools::Itertools;
use protocol::StandardProtocol;
use universal_name::get_universal_hash_prefix_sha_256;

pub struct AnyRegistry {
    uri_to_typeid: HashMap<&'static str, TypeId>,
    hash_prefix_to_typeid: HashMap<Vec<u8>, TypeId>,
    typeid_to_serializers: HashMap<TypeId, AnySerDeser>,
}

pub trait SerializeRef = binary_protocol::SerializeRef
    + compact_protocol::SerializeRef
    + simplejson_protocol::SerializeRef;
pub trait DeserializeSlice = binary_protocol::DeserializeSlice
    + compact_protocol::DeserializeSlice
    + simplejson_protocol::DeserializeSlice;

struct AnySerDeser {
    serialize: fn(Box<dyn std::any::Any>, StandardProtocol) -> Result<Vec<u8>>,
    deserialize: fn(&[u8], StandardProtocol) -> Result<Box<dyn std::any::Any>>,
}

impl std::fmt::Debug for AnyRegistry {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let mut map = f.debug_map();
        for uri in self.uri_to_typeid.keys().sorted() {
            map.entry(uri, &hex::encode(hash_prefix(uri).unwrap()));
        }
        map.finish()
    }
}

impl AnyRegistry {
    pub fn new() -> Self {
        Self {
            uri_to_typeid: HashMap::new(),
            hash_prefix_to_typeid: HashMap::new(),
            typeid_to_serializers: HashMap::new(),
        }
    }

    pub fn register_type<T: 'static + GetUri + SerializeRef + DeserializeSlice>(
        &mut self,
    ) -> Result<bool> {
        let uri = T::uri();
        if self.uri_to_typeid.contains_key(uri) {
            return Ok(false);
        }
        let hash = hash_prefix(uri)?;
        let type_id = TypeId::of::<T>();

        self.uri_to_typeid.insert(uri, type_id);
        self.hash_prefix_to_typeid.insert(hash, type_id);
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

    pub fn load(&self, val: &any::Any) -> Result<Box<dyn std::any::Any>> {
        let AnySerDeser { deserialize, .. } = self.serializers(val)?;
        deserialize(&val.data, val.protocol.unwrap_or(StandardProtocol::Compact))
    }

    pub fn store(
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
}

pub const UNIVERSAL_HASH_PREFIX_SHA_256_LEN: i8 = 16;

#[inline]
fn hash_prefix(uri: &str) -> Result<Vec<u8>> {
    get_universal_hash_prefix_sha_256(uri, UNIVERSAL_HASH_PREFIX_SHA_256_LEN)
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

    #[test]
    fn test_debug() -> Result<()> {
        let mut registry = AnyRegistry::new();
        registry.register_type::<union_map_string_i32>().unwrap();
        registry.register_type::<struct_map_string_i32>().unwrap();
        let expected = "{\n    \
            \"facebook.com/thrift/test/testset/struct_map_string_i32\": \"d4252c01f98d688ebb155e0a0a316763\",\n    \
            \"facebook.com/thrift/test/testset/union_map_string_i32\": \"35f8820d6ded47c7d1611dd76a002040\",\n\
            }";
        assert_eq!(format!("{:#?}", &registry), expected);
        Ok(())
    }

    #[test]
    fn test_round_trip() -> Result<()> {
        fn test_serialize_round_trip_for_protocol(protocol: StandardProtocol) -> Result<()> {
            let original = get_test_object();
            let serialized = serialize(&original, protocol)?;
            let deserialized = deserialize(&serialized, protocol)?;
            assert_eq!(original, deserialized);
            Ok(())
        }
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
            let any = Box::new(obj.clone()) as Box<dyn std::any::Any>;
            let bytes = serialize(any, protocol)?;
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

        let hash_prefix = hash_prefix(struct_map_string_i32::uri())?;
        let AnySerDeser {
            serialize,
            deserialize,
        } = any_registry.serializers_given_hash_prefix(&hash_prefix)?;

        let obj = get_test_object();
        for protocol in get_test_protocols() {
            let any = Box::new(obj.clone()) as Box<dyn std::any::Any>;
            let bytes = serialize(any, protocol)?;
            let val = *(deserialize(&bytes, protocol)?
                .downcast::<struct_map_string_i32>()
                .map_err(|_| anyhow::Error::msg("bad any cast")))?;
            assert_eq!(val, obj);
        }

        Ok(())
    }

    #[test]
    fn test_load_store() -> Result<()> {
        let mut registry = AnyRegistry::new();
        registry.register_type::<struct_map_string_i32>()?;

        let uri = Some(struct_map_string_i32::uri().to_owned());
        let compact = "1b018546416e7377657220746f2074686520556c74696d617465205175657374696f6e206f66204c6966652c2074686520556e6976657273652c20616e642045766572797468696e672e5400";
        let binary = "0d00010b080000000100000046416e7377657220746f2074686520556c74696d617465205175657374696f6e206f66204c6966652c2074686520556e6976657273652c20616e642045766572797468696e672e0000002a00";
        let json = "{\"field_1\":{\"Answer to the Ultimate Question of Life, the Universe, and Everything.\":42}}";

        let val = any::Any {
            r#type: uri.clone(),
            protocol: Some(StandardProtocol::Compact),
            data: hex::decode(compact)?,
            ..Default::default()
        };
        let any = registry.load(&val)?;
        let val = any
            .downcast::<struct_map_string_i32>()
            .map_err(|_| anyhow::Error::msg("bad any cast"))?;
        assert_eq!(*val, get_test_object());
        assert_eq!(
            compact,
            hex::encode(registry.store(val as Box<dyn std::any::Any>, StandardProtocol::Compact)?)
        );

        let val = any::Any {
            r#type: uri.clone(),
            protocol: Some(StandardProtocol::Binary),
            data: hex::decode(binary)?,
            ..Default::default()
        };
        let any = registry.load(&val)?;
        let val = any
            .downcast::<struct_map_string_i32>()
            .map_err(|_| anyhow::Error::msg("bad any cast"))?;
        assert_eq!(*val, get_test_object());
        assert_eq!(
            binary,
            hex::encode(registry.store(val as Box<dyn std::any::Any>, StandardProtocol::Binary)?)
        );

        let val = any::Any {
            r#type: uri.clone(),
            protocol: Some(StandardProtocol::SimpleJson),
            data: json.as_bytes().to_vec(),
            ..Default::default()
        };
        let any = registry.load(&val)?;
        let val = any
            .downcast::<struct_map_string_i32>()
            .map_err(|_| anyhow::Error::msg("bad any cast"))?;
        assert_eq!(*val, get_test_object());
        assert_eq!(
            json,
            std::str::from_utf8(
                &registry.store(val as Box<dyn std::any::Any>, StandardProtocol::SimpleJson)?
            )?
        );

        Ok(())
    }
}
