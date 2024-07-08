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

use anyhow::bail;
use anyhow::Result;
use fbthrift::binary_protocol;
use fbthrift::compact_protocol;
use fbthrift::simplejson_protocol;
use standard::ByteBuffer;
use standard::StandardProtocol;
use type_rep::ProtocolUnion;

use crate::compression::decompress_any;
use crate::compression::is_compressed;
use crate::thrift_any_type::ensure_thrift_any_type;
use crate::AnyError;
use crate::GetThriftAnyType;

// For convenience
pub trait DeserializableFromAny:
    GetThriftAnyType
    + fbthrift::GetUri
    + fbthrift::GetTType
    + compact_protocol::DeserializeSlice
    + binary_protocol::DeserializeSlice
    + simplejson_protocol::DeserializeSlice
{
}
impl<T> DeserializableFromAny for T where
    T: GetThriftAnyType
        + fbthrift::GetUri
        + fbthrift::GetTType
        + compact_protocol::DeserializeSlice
        + binary_protocol::DeserializeSlice
        + simplejson_protocol::DeserializeSlice
{
}

pub fn is_type<T>(any: &any::Any) -> bool
where
    T: GetThriftAnyType,
{
    ensure_thrift_any_type::<T>(any).is_ok()
}

pub fn deserialize<T>(any: &any::Any) -> Result<T>
where
    T: DeserializableFromAny,
{
    ensure_thrift_any_type::<T>(any)?;
    match &any.protocol {
        ProtocolUnion::standard(standard_protocol) => {
            deserialize_standard(*standard_protocol, &any.data)
        }
        ProtocolUnion::custom(_) => {
            if is_compressed(any) {
                let decompressed_any = decompress_any(any)?;
                let standard_protocol = extract_standard_protocol(&decompressed_any)?;
                deserialize_standard(standard_protocol, &decompressed_any.data)
            } else {
                bail!(AnyError::UnsupportedThriftProtocol(any.protocol.clone()));
            }
        }
        _ => bail!(AnyError::UnsupportedThriftProtocol(any.protocol.clone())),
    }
}

fn extract_standard_protocol(any: &any::Any) -> Result<StandardProtocol> {
    if let ProtocolUnion::standard(standard_protocol) = any.protocol {
        return Ok(standard_protocol);
    }
    bail!(AnyError::StandardThriftProtocolExpectationViolated(
        any.protocol.clone()
    ))
}

fn deserialize_standard<T>(standard_protocol: StandardProtocol, data: &ByteBuffer) -> Result<T>
where
    T: DeserializableFromAny,
{
    match standard_protocol {
        StandardProtocol::Compact => compact_protocol::deserialize(data),
        StandardProtocol::Binary => binary_protocol::deserialize(data),
        StandardProtocol::SimpleJson => simplejson_protocol::deserialize(data),
        _ => bail!(AnyError::UnsupportedStandardThriftProtocol(
            standard_protocol
        )),
    }
}

#[cfg(test)]
mod tests {
    use any::Any;
    use anyhow::Result;
    use standard::StandardProtocol;
    use standard::TypeName;
    use standard::TypeUri;
    use type_::Protocol;
    use type_::Type;

    use super::deserialize;

    #[test]
    fn test_deserialize_0() -> Result<()> {
        use artifact_builder_override::ExperimentOverrideRequestContext;
        let val = Any {
            r#type: Type {
                name: TypeName::structType(TypeUri::typeHashPrefixSha2_256(
                    base64::decode("e3AbYkUAP8FICiPtbGYI6w").unwrap(),
                )),
                params: Vec::new(),
                ..Default::default()
            },
            protocol: Protocol::standard(StandardProtocol::Compact),
            data: base64::decode(
                "GAozNjAwNjUwNzQ0GA5jb2d3aGVlbF90ZXN0cxwcHLwoENBU+E6zAHkpmP0Y3JboyaMAABkMABwVBAAYlgMZHBgNY29nd2hlZWxfdGVzdBkcGBIvbG9ncy90aHJpZnRfbW9ja3MYDHRocmlmdF9tb2NrczbAmgwAABwYjAJ7ImFsbF9zaWRlcyI6IFsiYSJdLCAicnBtX21hcCI6IHt9LCAiYnVuZGxlX3BhY2thZ2VfbWFwIjogeyJyY2VzZXJ2aWNlIjogImY4NDZjZjczNGM2YjQ3NDI5OTdiNDBjNTA2MWI1NmJhIiwgInR1cHBlcndhcmUuaW1hZ2Uud2ViZm91bmRhdGlvbi5jOS5yY2VzZXJ2aWNlIjogImZkNDZmN2VmMzQ0ZDYxNTIzMTdhODJlMzE4ZjZkMjk4In0sICJzZmlkIjogInJjZXNlcnZpY2Uvd3d3X2dlbmVyaWMiLCAiYnVuZGxlX2lkIjogODE4OCwgIm1pbm9yX2J1bmRsZV9pZCI6IDF9HBwSEgAcIRISHBIWAAAAHAAcJAIAHBglY29nd2hlZWxfcmNlc2VydmljZV90ZXN0X3Rlc3RfaGFybmVzcwAcAAAcGwAAABgAGwAAABIbABwAAA",
            )?,
            ..Default::default()
        };

        let ctx = deserialize::<ExperimentOverrideRequestContext>(&val)?;
        assert_eq!(ctx.windtunnel_trial_id, "3600650744");
        assert_eq!(ctx.reservation_name, "cogwheel_tests");

        Ok(())
    }
}
