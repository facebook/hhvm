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

// If you want an adapter to refer to a Thrift struct defined in the Thrift file using the adapter,
// you have to use `rust_include_srcs` to avoid the target circular dependency.
use std::fmt::Debug;
use std::marker::PhantomData;

use anyhow::Context;
use fbthrift::adapter::ThriftAdapter;
use fbthrift::metadata::ThriftAnnotations;

#[derive(Clone, Debug, PartialEq)]
pub struct Wrapper<T: Clone + Debug + PartialEq + Sync + Send>(pub T);

pub struct FieldCheckerAdapter {}

impl ThriftAdapter for FieldCheckerAdapter {
    type StandardType = String;
    type Error = anyhow::Error;

    type AdaptedType = String;

    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        Ok(value)
    }

    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        value.clone()
    }

    fn from_thrift_field<T: ThriftAnnotations>(
        value: Self::StandardType,
        field_id: i16,
    ) -> Result<Self::AdaptedType, Self::Error> {
        assert_eq!(field_id, 8);
        assert_eq!(std::any::TypeId::of::<Foo>(), std::any::TypeId::of::<T>());

        Self::from_thrift(value)
    }

    fn to_thrift_field<T: ThriftAnnotations>(
        value: &Self::AdaptedType,
        field_id: i16,
    ) -> Self::StandardType {
        assert_eq!(field_id, 8);
        assert_eq!(std::any::TypeId::of::<Foo>(), std::any::TypeId::of::<T>());

        Self::to_thrift(value)
    }
}

pub struct IOBufIdentityAdapter {}

impl ThriftAdapter for IOBufIdentityAdapter {
    type StandardType = bytes::Bytes;
    type Error = std::convert::Infallible;

    type AdaptedType = bytes::Bytes;

    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        Ok(value)
    }

    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        value.clone()
    }
}

pub struct WrappedAdaptedBytesIdentityAdapter {}

impl ThriftAdapter for WrappedAdaptedBytesIdentityAdapter {
    type StandardType = WrappedAdaptedBytes;
    type Error = std::convert::Infallible;

    type AdaptedType = WrappedAdaptedBytes;

    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        Ok(value)
    }

    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        value.clone()
    }
}

pub struct AdaptedStringListIdentityAdapter {}

impl ThriftAdapter for AdaptedStringListIdentityAdapter {
    type StandardType = Vec<AdaptedString>;
    type Error = std::convert::Infallible;

    type AdaptedType = Vec<AdaptedString>;

    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        Ok(value)
    }

    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        value.clone()
    }
}

#[derive(Debug, Default, Clone, PartialEq)]
pub enum AssetType {
    #[default]
    Unknown,
    Laptop,
    Server,
}

#[derive(Debug, Default, Clone, PartialEq)]
pub struct Asset {
    pub type_: AssetType,
    pub id: u32,
}

pub struct AssetAdapter {}

impl ThriftAdapter for AssetAdapter {
    type StandardType = crate::unadapted::Asset;
    type Error = anyhow::Error;

    type AdaptedType = Asset;

    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        Ok(Asset {
            type_: match value.type_ {
                crate::ThriftAssetType::LAPTOP => AssetType::Laptop,
                crate::ThriftAssetType::SERVER => AssetType::Server,
                // Rust Thrift will NOT use 0 as the default enum value by default, but instead
                // will use i32::MIN.
                _ => AssetType::Unknown,
            },
            id: value.id.try_into().context("invalid i64 for id")?,
        })
    }

    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        crate::unadapted::Asset {
            type_: match value.type_ {
                AssetType::Unknown => crate::ThriftAssetType::UNKNOWN,
                AssetType::Laptop => crate::ThriftAssetType::LAPTOP,
                AssetType::Server => crate::ThriftAssetType::SERVER,
            },
            id: value.id.into(),
            ..Default::default()
        }
    }
}

pub struct AdaptedAssetIdentityAdapter;

impl ThriftAdapter for AdaptedAssetIdentityAdapter {
    type StandardType = Asset;
    type Error = std::convert::Infallible;

    type AdaptedType = Asset;

    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        Ok(value)
    }

    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        value.clone()
    }
}

pub struct TransitiveTestAdapter<T>(PhantomData<T>);

impl<T> ThriftAdapter for TransitiveTestAdapter<T>
where
    T: Clone + Debug + PartialEq + Send + Sync,
{
    type StandardType = T;
    type AdaptedType = Wrapper<T>;

    type Error = std::convert::Infallible;

    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        Ok(Wrapper(value))
    }

    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        value.0.clone()
    }
}

pub struct AnnotationTestAdapter;

impl ThriftAdapter for AnnotationTestAdapter {
    type StandardType = String;
    type AdaptedType = Wrapper<String>;

    type Error = std::convert::Infallible;

    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        Ok(Wrapper(value))
    }

    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        value.0.clone()
    }

    fn from_thrift_field<T: ThriftAnnotations>(
        value: Self::StandardType,
        field_id: i16,
    ) -> Result<Self::AdaptedType, Self::Error> {
        assert_eq!(field_id, 1);
        assert_eq!(
            std::any::TypeId::of::<crate::unadapted::TransitiveStruct>(),
            std::any::TypeId::of::<T>()
        );

        assert_eq!(
            T::get_structured_annotation::<crate::unadapted::TransitiveAdapterAnnotation>()
                .unwrap()
                .payload,
            "hello_world"
        );
        assert_eq!(
            T::get_structured_annotation::<crate::FirstAnnotation>()
                .unwrap()
                .uri,
            "thrift/test"
        );

        Self::from_thrift(value)
    }

    fn to_thrift_field<T: ThriftAnnotations>(
        value: &Self::AdaptedType,
        field_id: i16,
    ) -> Self::StandardType {
        assert_eq!(field_id, 1);
        assert_eq!(
            std::any::TypeId::of::<crate::unadapted::TransitiveStruct>(),
            std::any::TypeId::of::<T>()
        );

        assert_eq!(
            T::get_structured_annotation::<crate::unadapted::TransitiveAdapterAnnotation>()
                .unwrap()
                .payload,
            "hello_world"
        );
        assert_eq!(
            T::get_structured_annotation::<crate::FirstAnnotation>()
                .unwrap()
                .uri,
            "thrift/test"
        );

        Self::to_thrift(value)
    }
}

pub struct TransitiveAnnotationTestAdapter<T>(PhantomData<T>);

impl<T> ThriftAdapter for TransitiveAnnotationTestAdapter<T>
where
    T: Clone + Debug + PartialEq + Sync + Send,
{
    type StandardType = T;
    type AdaptedType = Wrapper<T>;

    type Error = std::convert::Infallible;

    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        Ok(Wrapper(value))
    }

    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        value.0.clone()
    }

    fn from_thrift_field<S: ThriftAnnotations>(
        value: Self::StandardType,
        field_id: i16,
    ) -> Result<Self::AdaptedType, Self::Error> {
        assert_eq!(field_id, 2);
        assert_eq!(
            std::any::TypeId::of::<crate::unadapted::TransitiveStruct>(),
            std::any::TypeId::of::<S>()
        );

        assert_eq!(
            S::get_structured_annotation::<crate::unadapted::TransitiveAdapterAnnotation>()
                .unwrap()
                .payload,
            "hello_world"
        );
        assert_eq!(
            S::get_structured_annotation::<crate::FirstAnnotation>()
                .unwrap()
                .uri,
            "thrift/test"
        );

        assert_eq!(
            S::get_field_structured_annotation::<
                crate::unadapted::TransitiveFieldAdapterAnnotation,
            >(field_id)
            .unwrap()
            .payload,
            "foobar"
        );
        assert_eq!(
            S::get_field_structured_annotation::<crate::FirstAnnotation>(field_id)
                .unwrap()
                .uri,
            "thrift/transitive_field_test"
        );

        Self::from_thrift(value)
    }

    fn to_thrift_field<S: ThriftAnnotations>(
        value: &Self::AdaptedType,
        field_id: i16,
    ) -> Self::StandardType {
        assert_eq!(field_id, 2);
        assert_eq!(
            std::any::TypeId::of::<crate::unadapted::TransitiveStruct>(),
            std::any::TypeId::of::<S>()
        );

        assert_eq!(
            S::get_structured_annotation::<crate::unadapted::TransitiveAdapterAnnotation>()
                .unwrap()
                .payload,
            "hello_world"
        );
        assert_eq!(
            S::get_structured_annotation::<crate::FirstAnnotation>()
                .unwrap()
                .uri,
            "thrift/test"
        );

        assert_eq!(
            S::get_field_structured_annotation::<
                crate::unadapted::TransitiveFieldAdapterAnnotation,
            >(field_id)
            .unwrap()
            .payload,
            "foobar"
        );
        assert_eq!(
            S::get_field_structured_annotation::<crate::FirstAnnotation>(field_id)
                .unwrap()
                .uri,
            "thrift/transitive_field_test"
        );

        Self::to_thrift(value)
    }
}
