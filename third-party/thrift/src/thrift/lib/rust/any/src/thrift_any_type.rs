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
use crate::AnyTypeExpectationViolated;
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
        bail!(AnyError::from(
            AnyTypeExpectationViolated::TypeUrisInconsistent((uri.clone(), other.clone()))
        ));
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
            _ => bail!(AnyError::from(
                AnyTypeExpectationViolated::TypeNamesInconsistent((
                    any.r#type.name.clone(),
                    t_type.name
                ))
            )),
        },
        TypeName::unionType(uri) => match &t_type.name {
            TypeName::unionType(other) => check_uri_consistency(uri, other)?,
            _ => bail!(AnyError::from(
                AnyTypeExpectationViolated::TypeNamesInconsistent((
                    any.r#type.name.clone(),
                    t_type.name
                ))
            )),
        },
        _ => (), // Avoiding being overly prescriptive.
    }

    Ok(())
}

#[cfg(test)]
mod tests {

    use any::Any;
    use anyhow::anyhow;
    use anyhow::Result;
    use base64::alphabet::STANDARD;
    use base64::engine::general_purpose::GeneralPurpose;
    use base64::engine::general_purpose::GeneralPurposeConfig;
    use base64::engine::DecodePaddingMode;
    use base64::Engine;
    use standard::TypeName;
    use standard::TypeUri;
    use type_::Type;

    use super::check_uri_consistency;
    use super::ensure_thrift_any_type;
    use crate::AnyError;
    use crate::AnyTypeExpectationViolated;

    // Bring back the pre 0.20 bevahiour and allow either padded or un-padded base64 strings at decode time.
    const STANDARD_INDIFFERENT: GeneralPurpose = GeneralPurpose::new(
        &STANDARD,
        GeneralPurposeConfig::new().with_decode_padding_mode(DecodePaddingMode::Indifferent),
    );

    #[test]
    fn test_ensure_thrift_any_type_0() -> Result<()> {
        use artifact_builder_override::ExperimentOverrideRequestContext;
        let val = Any {
            r#type: Type {
                name: TypeName::structType(TypeUri::typeHashPrefixSha2_256(
                    STANDARD_INDIFFERENT
                        .decode("e3AbYkUAP8FICiPtbGYI6w")
                        .unwrap(),
                )),
                ..Default::default()
            },
            ..Default::default()
        };
        ensure_thrift_any_type::<ExperimentOverrideRequestContext>(&val)
    }

    #[test]
    fn test_ensure_thrift_any_type_1() -> Result<()> {
        fn test_failed() -> Result<()> {
            Err(anyhow!("expected type uri inconsistency error"))
        }
        let l = TypeUri::uri("facebook.com/icsp/domains/container/ContainerBuildState".to_owned());
        let r = TypeUri::uri(
            "facebook.com/icsp/domains/container/ContainerBuildInstanceState".to_owned(),
        );
        match check_uri_consistency(&l, &r) {
            Err(err) => match err.downcast_ref::<AnyError>() {
                Some(AnyError::AnyTypeExpectationViolated(
                    AnyTypeExpectationViolated::TypeUrisInconsistent(_),
                )) => Ok(()),
                _ => test_failed(),
            },
            _ => test_failed(),
        }
    }

    #[test]
    fn test_ensure_thrift_any_type_2() -> Result<()> {
        fn test_failed() -> Result<()> {
            Err(anyhow!("expected type name inconsistency error"))
        }
        use artifact_builder_override::ExperimentOverrideRequestContext;
        let val = Any {
            r#type: Type {
                name: TypeName::unionType(TypeUri::uri("foo/bar/baz".to_owned())),
                ..Default::default()
            },
            ..Default::default()
        };
        match ensure_thrift_any_type::<ExperimentOverrideRequestContext>(&val) {
            Err(err) => match err.downcast_ref::<AnyError>() {
                Some(AnyError::AnyTypeExpectationViolated(
                    AnyTypeExpectationViolated::TypeNamesInconsistent(_),
                )) => Ok(()),
                _ => test_failed(),
            },
            _ => test_failed(),
        }
    }
}
