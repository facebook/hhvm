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

use any::Any;
use anyhow::bail;
use anyhow::Result;
use standard::TypeName;
use standard::TypeUri;

use crate::AnyError;
use crate::ThriftAnyType;

pub trait GetThriftAnyType: fbthrift::GetUri {
    fn get_thrift_any_type() -> ThriftAnyType;
}

impl<T: fbthrift::GetTypeNameType + fbthrift::GetUri> GetThriftAnyType for T {
    fn get_thrift_any_type() -> ThriftAnyType {
        match <T as fbthrift::GetTypeNameType>::type_name_type() {
            fbthrift::TypeNameType::StructType => make_thrift_any_type_struct::<T>(),
            fbthrift::TypeNameType::UnionType => make_thrift_any_type_union::<T>(),
        }
    }
}

pub fn make_thrift_any_type_struct<T: fbthrift::GetUri>() -> ThriftAnyType {
    ThriftAnyType {
        name: TypeName::structType(TypeUri::uri(T::uri().to_string())),
        ..Default::default()
    }
}

pub fn make_thrift_any_type_union<T: fbthrift::GetUri>() -> ThriftAnyType {
    ThriftAnyType {
        name: TypeName::unionType(TypeUri::uri(T::uri().to_string())),
        ..Default::default()
    }
}

fn check_uri_consistency(uri: &TypeUri, other: &TypeUri) -> Result<()> {
    fn get_hash_prefix(s: &str) -> Result<Vec<u8>> {
        universal_name::get_universal_hash_prefix_sha_256(
            s,
            universal_name::UNIVERSAL_HASH_PREFIX_SHA_256_LEN,
        )
    }
    if !(match (uri, other) {
        (TypeUri::uri(s), TypeUri::uri(t)) => s == t,
        (TypeUri::typeHashPrefixSha2_256(s), TypeUri::typeHashPrefixSha2_256(t)) => s == t,
        (TypeUri::typeHashPrefixSha2_256(s), TypeUri::uri(t)) => s == &get_hash_prefix(t)?,
        (TypeUri::uri(t), TypeUri::typeHashPrefixSha2_256(s)) => s == &get_hash_prefix(t)?,
        (TypeUri::scopedName(s), TypeUri::scopedName(t)) => s == t,
        (_, _) => true,
    }) {
        bail!(crate::AnyError::AnyTypeExpectationViolated);
    }
    Ok(())
}

pub fn ensure_thrift_any_type<T>(any: &Any) -> Result<()>
where
    T: GetThriftAnyType,
{
    let t_type = <T as GetThriftAnyType>::get_thrift_any_type();
    match &any.r#type.name {
        TypeName::structType(uri) => match &t_type.name {
            TypeName::structType(other_uri) => check_uri_consistency(uri, other_uri)?,
            _ => bail!(AnyError::AnyTypeExpectationViolated),
        },
        TypeName::unionType(uri) => match &t_type.name {
            TypeName::unionType(other) => check_uri_consistency(uri, other)?,
            _ => bail!(AnyError::AnyTypeExpectationViolated),
        },
        _ => (), // Avoiding being overly prescriptive.
    }

    Ok(())
}

#[cfg(test)]
mod tests {

    use any::Any;
    use anyhow::Result;
    use standard::TypeName;
    use standard::TypeUri;
    use type_::Type;

    use super::ensure_thrift_any_type;

    #[test]
    fn test_ensure_thrift_any_type_0() -> Result<()> {
        use artifact_builder_override::ExperimentOverrideRequestContext;
        let val = Any {
            r#type: Type {
                name: TypeName::structType(TypeUri::typeHashPrefixSha2_256(
                    base64::decode("e3AbYkUAP8FICiPtbGYI6w").unwrap(),
                )),
                ..Default::default()
            },
            ..Default::default()
        };
        ensure_thrift_any_type::<ExperimentOverrideRequestContext>(&val)
    }
}
