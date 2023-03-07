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
use std::collections::BTreeMap;
use std::collections::BTreeSet;
use std::fmt::Debug;
use std::marker::PhantomData;

pub trait ThriftAdapter {
    /// Aka the "from" type.
    type OriginalType;
    /// Aka the "to" type.
    type AdaptedType: Clone + Debug + PartialEq + Send + Sync;

    /// The error type thrown if `from_original` fails during deserialization.
    type Error: Into<anyhow::Error> + Debug;

    /// Converts an instance of `OriginalType` to `AdaptedType` during deserialization.
    ///
    /// The `Err` returned by this method will be propagated as a deserialization error.
    fn from_original(value: Self::OriginalType) -> Result<Self::AdaptedType, Self::Error>;

    // TODO(emersonford): should we support a moved param here?
    /// Converts from the given `AdaptedType` back to the given `OriginalType` during
    /// serialization.
    ///
    /// This must be an infallible operation as `serialize` is infallible.
    ///
    /// WARNING: you should be particularly cautious when using `.unwrap()` or any other panic-able
    /// methods in this method. If this method panics, it will be at serialization time and not at
    /// the Thrift struct creation time, meaning it will be extremely difficult to debug what the
    /// true "source" of the panic.
    ///
    /// If your `AdaptedType` -> `OriginalType` conversion is truly fallible, you probably shouldn't
    /// use an adapter to begin with.
    fn to_original(value: &Self::AdaptedType) -> Self::OriginalType;

    /// Method called when this adapter is used on a Thrift struct's field. Provides information
    /// about the specific field ID and the type ID of the parent struct this field is in.
    ///
    /// Defaults to calling `from_original`.
    fn from_thrift_field(
        value: Self::OriginalType,
        _field_id: i16,
        _strct: TypeId,
    ) -> Result<Self::AdaptedType, Self::Error> {
        Self::from_original(value)
    }

    /// Method called when this adapter is used on a Thrift struct's field. Provides information
    /// about the specific field ID and the type ID of the parent struct this field is in.
    ///
    /// Defaults to calling `to_original`.
    fn to_thrift_field(
        value: &Self::AdaptedType,
        _field_id: i16,
        _strct: TypeId,
    ) -> Self::OriginalType {
        Self::to_original(value)
    }

    /// Method called when the adapted type is not present in a field during deserialization or is
    /// populated with `..Default::default()`. The value passed here is the default original type
    /// value for the field. This can be used to record that the field was not present inside
    /// of your adapted type.
    ///
    /// **This method must be infallible, as it will be called when `Default::default()` is used
    /// (which is infallible).**
    ///
    /// WARNING: This defaults to calling `from_thrift_field` and **assumes `from_thrift_field`
    /// will not return an `Err` for the default value**.
    fn from_thrift_default(
        value: Self::OriginalType,
        field_id: i16,
        strct: TypeId,
    ) -> Self::AdaptedType {
        Self::from_thrift_field(value, field_id, strct).unwrap_or_else(|e| {
            panic!(
                "`from_thrift_field` must not return an `Err` on field ID {} for its default value, but it did: '{:?}'",
                field_id, e
            );
        })
    }
}

// NOTE: we define where bounds on the structs themselves here to improve error messaging during
// the Thrift compilation process.

//// Layers multiple [ThriftTypeAdapter] together. Used when multiple Thrift typedefs with adapters
//// are layered on each other.
pub struct LayeredThriftAdapter<Fst, Snd>
where
    Fst: ThriftAdapter,
    Snd: ThriftAdapter,
    Fst::OriginalType: Into<Snd::AdaptedType>,
    Snd::AdaptedType: Into<Fst::OriginalType>,
{
    _inner_first: PhantomData<Fst>,
    _inner_second: PhantomData<Snd>,
}

impl<Fst, Snd> ThriftAdapter for LayeredThriftAdapter<Fst, Snd>
where
    Fst: ThriftAdapter,
    Snd: ThriftAdapter,
    Fst::OriginalType: Into<Snd::AdaptedType>,
    Snd::AdaptedType: Into<Fst::OriginalType>,
{
    type OriginalType = <Snd as ThriftAdapter>::OriginalType;
    type AdaptedType = <Fst as ThriftAdapter>::AdaptedType;

    type Error = anyhow::Error;

    #[inline]
    fn from_original(value: Self::OriginalType) -> Result<Self::AdaptedType, Self::Error> {
        <Fst as ThriftAdapter>::from_original(
            <Snd as ThriftAdapter>::from_original(value)
                .map_err(Into::<anyhow::Error>::into)?
                .into(),
        )
        .map_err(Into::<anyhow::Error>::into)
    }

    #[inline]
    fn to_original(value: &Self::AdaptedType) -> Self::OriginalType {
        <Snd as ThriftAdapter>::to_original(&<Fst as ThriftAdapter>::to_original(value).into())
    }

    #[inline]
    fn from_thrift_field(
        value: Self::OriginalType,
        field_id: i16,
        strct: TypeId,
    ) -> Result<Self::AdaptedType, Self::Error> {
        <Fst as ThriftAdapter>::from_thrift_field(
            <Snd as ThriftAdapter>::from_thrift_field(value, field_id, strct)
                .map_err(Into::<anyhow::Error>::into)?
                .into(),
            field_id,
            strct,
        )
        .map_err(Into::<anyhow::Error>::into)
    }

    #[inline]
    fn to_thrift_field(
        value: &Self::AdaptedType,
        field_id: i16,
        strct: TypeId,
    ) -> Self::OriginalType {
        <Snd as ThriftAdapter>::to_thrift_field(
            &<Fst as ThriftAdapter>::to_thrift_field(value, field_id, strct).into(),
            field_id,
            strct,
        )
    }

    #[inline]
    fn from_thrift_default(
        value: Self::OriginalType,
        field_id: i16,
        strct: TypeId,
    ) -> Self::AdaptedType {
        <Fst as ThriftAdapter>::from_thrift_default(
            <Snd as ThriftAdapter>::from_thrift_default(value, field_id, strct).into(),
            field_id,
            strct,
        )
    }
}

/// Transforms the given adapter `A` into an adapter with the signature `Vec<OriginalType>`
/// -> `Vec<AdaptedType>`. Because Rust doesn't have HKT, we cannot make this "generic" over
/// multiple collection types.
pub struct ListMapAdapter<A>
where
    A: ThriftAdapter,
{
    _inner_adapter: PhantomData<A>,
}

impl<A> ThriftAdapter for ListMapAdapter<A>
where
    A: ThriftAdapter,
{
    type OriginalType = Vec<A::OriginalType>;
    type AdaptedType = Vec<A::AdaptedType>;

    type Error = A::Error;

    #[inline]
    fn from_original(value: Self::OriginalType) -> Result<Self::AdaptedType, Self::Error> {
        value
            .into_iter()
            .map(|elem| A::from_original(elem))
            .collect::<Result<Self::AdaptedType, Self::Error>>()
    }

    #[inline]
    fn to_original(value: &Self::AdaptedType) -> Self::OriginalType {
        value.iter().map(|elem| A::to_original(elem)).collect()
    }

    #[inline]
    fn from_thrift_field(
        value: Self::OriginalType,
        field_id: i16,
        strct: TypeId,
    ) -> Result<Self::AdaptedType, Self::Error> {
        value
            .into_iter()
            .map(|elem| A::from_thrift_field(elem, field_id, strct))
            .collect::<Result<Self::AdaptedType, Self::Error>>()
    }

    #[inline]
    fn to_thrift_field(
        value: &Self::AdaptedType,
        field_id: i16,
        strct: TypeId,
    ) -> Self::OriginalType {
        value
            .iter()
            .map(|elem| A::to_thrift_field(elem, field_id, strct))
            .collect()
    }

    #[inline]
    fn from_thrift_default(
        value: Self::OriginalType,
        field_id: i16,
        strct: TypeId,
    ) -> Self::AdaptedType {
        value
            .into_iter()
            .map(|elem| A::from_thrift_default(elem, field_id, strct))
            .collect()
    }
}

/// Transforms the given adapter `A` into an adapter with the signature `BTreeSet<OriginalType>`
/// -> `BTreeSet<AdaptedType>`. Because Rust doesn't have HKT, we cannot make this "generic" over
/// multiple collection types.
pub struct SetMapAdapter<A>
where
    A: ThriftAdapter,
    A::OriginalType: Ord + PartialEq,
    A::AdaptedType: Ord + PartialEq,
{
    _inner_adapter: PhantomData<A>,
}

impl<A> ThriftAdapter for SetMapAdapter<A>
where
    A: ThriftAdapter,
    A::OriginalType: Ord + PartialEq,
    A::AdaptedType: Ord + PartialEq,
{
    type OriginalType = BTreeSet<A::OriginalType>;
    type AdaptedType = BTreeSet<A::AdaptedType>;

    type Error = A::Error;

    #[inline]
    fn from_original(value: Self::OriginalType) -> Result<Self::AdaptedType, Self::Error> {
        value
            .into_iter()
            .map(|elem| A::from_original(elem))
            .collect::<Result<Self::AdaptedType, Self::Error>>()
    }

    #[inline]
    fn to_original(value: &Self::AdaptedType) -> Self::OriginalType {
        value.iter().map(|elem| A::to_original(elem)).collect()
    }

    #[inline]
    fn from_thrift_field(
        value: Self::OriginalType,
        field_id: i16,
        strct: TypeId,
    ) -> Result<Self::AdaptedType, Self::Error> {
        value
            .into_iter()
            .map(|elem| A::from_thrift_field(elem, field_id, strct))
            .collect::<Result<Self::AdaptedType, Self::Error>>()
    }

    #[inline]
    fn to_thrift_field(
        value: &Self::AdaptedType,
        field_id: i16,
        strct: TypeId,
    ) -> Self::OriginalType {
        value
            .iter()
            .map(|elem| A::to_thrift_field(elem, field_id, strct))
            .collect()
    }

    #[inline]
    fn from_thrift_default(
        value: Self::OriginalType,
        field_id: i16,
        strct: TypeId,
    ) -> Self::AdaptedType {
        value
            .into_iter()
            .map(|elem| A::from_thrift_default(elem, field_id, strct))
            .collect()
    }
}

/// Transforms the given adapter `KA` and `KV` into an adapter with the signature
/// `BTreeMap<KA::OriginalType, KV::OriginalType>` -> `BTreeMap<KA::AdaptedType, KV::AdaptedType>`.
/// Because Rust doesn't have HKT, we cannot make this "generic" over multiple collection types.
pub struct MapMapAdapter<KA, KV>
where
    KA: ThriftAdapter,
    KV: ThriftAdapter,
    KA::OriginalType: Ord + PartialEq,
    KA::AdaptedType: Ord + PartialEq,
{
    _inner_key_adapter: PhantomData<KA>,
    _inner_val_adapter: PhantomData<KV>,
}

impl<KA, KV> ThriftAdapter for MapMapAdapter<KA, KV>
where
    KA: ThriftAdapter,
    KV: ThriftAdapter,
    KA::OriginalType: Ord + PartialEq,
    KA::AdaptedType: Ord + PartialEq,
{
    type OriginalType = BTreeMap<KA::OriginalType, KV::OriginalType>;
    type AdaptedType = BTreeMap<KA::AdaptedType, KV::AdaptedType>;

    type Error = anyhow::Error;

    #[inline]
    fn from_original(value: Self::OriginalType) -> Result<Self::AdaptedType, Self::Error> {
        value
            .into_iter()
            .map(|(key, val)| {
                Ok((
                    KA::from_original(key).map_err(Into::<anyhow::Error>::into)?,
                    KV::from_original(val).map_err(Into::<anyhow::Error>::into)?,
                ))
            })
            .collect::<Result<Self::AdaptedType, Self::Error>>()
    }

    #[inline]
    fn to_original(value: &Self::AdaptedType) -> Self::OriginalType {
        value
            .iter()
            .map(|(key, val)| (KA::to_original(key), KV::to_original(val)))
            .collect::<Self::OriginalType>()
    }

    #[inline]
    fn from_thrift_field(
        value: Self::OriginalType,
        field_id: i16,
        strct: TypeId,
    ) -> Result<Self::AdaptedType, Self::Error> {
        value
            .into_iter()
            .map(|(key, val)| {
                Ok((
                    KA::from_thrift_field(key, field_id, strct)
                        .map_err(Into::<anyhow::Error>::into)?,
                    KV::from_thrift_field(val, field_id, strct)
                        .map_err(Into::<anyhow::Error>::into)?,
                ))
            })
            .collect::<Result<Self::AdaptedType, Self::Error>>()
    }

    #[inline]
    fn to_thrift_field(
        value: &Self::AdaptedType,
        field_id: i16,
        strct: TypeId,
    ) -> Self::OriginalType {
        value
            .iter()
            .map(|(key, val)| {
                (
                    KA::to_thrift_field(key, field_id, strct),
                    KV::to_thrift_field(val, field_id, strct),
                )
            })
            .collect::<Self::OriginalType>()
    }

    #[inline]
    fn from_thrift_default(
        value: Self::OriginalType,
        field_id: i16,
        strct: TypeId,
    ) -> Self::AdaptedType {
        value
            .into_iter()
            .map(|(key, val)| {
                (
                    KA::from_thrift_default(key, field_id, strct),
                    KV::from_thrift_default(val, field_id, strct),
                )
            })
            .collect::<Self::AdaptedType>()
    }
}

/// No-op adapter. Used if the key of a map is an adapted type, but the value isn't, or vice versa.
pub struct IdentityAdapter<T>
where
    T: Clone + Debug + Send + Sync + PartialEq,
{
    _inner: PhantomData<T>,
}

impl<T> ThriftAdapter for IdentityAdapter<T>
where
    T: Clone + Debug + Send + Sync + PartialEq,
{
    type OriginalType = T;
    type AdaptedType = T;

    type Error = std::convert::Infallible;

    fn from_original(value: Self::OriginalType) -> Result<Self::AdaptedType, Self::Error> {
        Ok(value)
    }

    fn to_original(value: &Self::AdaptedType) -> Self::OriginalType {
        value.clone()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // Represents the following flow
    // String (Final Adapted Type) <- BoolToStringAdapter -> Bool <- I64ToBoolAdapter -> i64
    // <- I8ToI64Adapter -> i8 (Original Thrift type)
    struct DummyParentStruct {}

    struct BoolToStringAdapter {}

    impl ThriftAdapter for BoolToStringAdapter {
        type OriginalType = bool;
        type AdaptedType = String;

        type Error = anyhow::Error;

        fn from_original(value: Self::OriginalType) -> Result<Self::AdaptedType, Self::Error> {
            Ok(value.to_string())
        }

        fn to_original(value: &Self::AdaptedType) -> Self::OriginalType {
            value == "true"
        }

        fn from_thrift_field(
            value: Self::OriginalType,
            field_id: i16,
            strct: TypeId,
        ) -> Result<Self::AdaptedType, Self::Error> {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), strct);

            Self::from_original(value)
        }

        fn to_thrift_field(
            value: &Self::AdaptedType,
            field_id: i16,
            strct: TypeId,
        ) -> Self::OriginalType {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), strct);

            Self::to_original(value)
        }

        fn from_thrift_default(
            value: Self::OriginalType,
            field_id: i16,
            strct: TypeId,
        ) -> Self::AdaptedType {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), strct);

            Self::from_thrift_field(value, field_id, strct).unwrap()
        }
    }

    struct I64ToBoolAdapter {}

    impl ThriftAdapter for I64ToBoolAdapter {
        type OriginalType = i64;
        type AdaptedType = bool;

        type Error = anyhow::Error;

        fn from_original(value: Self::OriginalType) -> Result<Self::AdaptedType, Self::Error> {
            match value {
                0 => Ok(false),
                1 => Ok(true),
                other => anyhow::bail!("invalid num {}", other),
            }
        }

        fn to_original(value: &Self::AdaptedType) -> Self::OriginalType {
            if *value { 1 } else { 0 }
        }

        fn from_thrift_field(
            value: Self::OriginalType,
            field_id: i16,
            strct: TypeId,
        ) -> Result<Self::AdaptedType, Self::Error> {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), strct);

            Self::from_original(value)
        }

        fn to_thrift_field(
            value: &Self::AdaptedType,
            field_id: i16,
            strct: TypeId,
        ) -> Self::OriginalType {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), strct);

            Self::to_original(value)
        }

        fn from_thrift_default(
            value: Self::OriginalType,
            field_id: i16,
            strct: TypeId,
        ) -> Self::AdaptedType {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), strct);

            Self::from_thrift_field(value, field_id, strct).unwrap()
        }
    }

    struct I8ToI64Adapter {}

    impl ThriftAdapter for I8ToI64Adapter {
        type OriginalType = i8;
        type AdaptedType = i64;

        type Error = anyhow::Error;

        fn from_original(value: Self::OriginalType) -> Result<Self::AdaptedType, Self::Error> {
            Ok(value.into())
        }

        fn to_original(value: &Self::AdaptedType) -> Self::OriginalType {
            (*value).try_into().unwrap()
        }

        fn from_thrift_field(
            value: Self::OriginalType,
            field_id: i16,
            strct: TypeId,
        ) -> Result<Self::AdaptedType, Self::Error> {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), strct);

            Self::from_original(value)
        }

        fn to_thrift_field(
            value: &Self::AdaptedType,
            field_id: i16,
            strct: TypeId,
        ) -> Self::OriginalType {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), strct);

            Self::to_original(value)
        }

        fn from_thrift_default(
            value: Self::OriginalType,
            field_id: i16,
            strct: TypeId,
        ) -> Self::AdaptedType {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), strct);

            Self::from_thrift_field(value, field_id, strct).unwrap()
        }
    }

    type TestLayeredAdapter = LayeredThriftAdapter<
        BoolToStringAdapter,
        LayeredThriftAdapter<I64ToBoolAdapter, I8ToI64Adapter>,
    >;

    #[test]
    fn test_from_thrift() {
        assert_eq!(TestLayeredAdapter::from_original(0_i8).unwrap(), "false");
        assert_eq!(TestLayeredAdapter::from_original(1_i8).unwrap(), "true");
    }

    #[test]
    fn test_from_thrift_error() {
        let res = TestLayeredAdapter::from_original(3_i8);

        assert!(res.is_err());
        assert_eq!(res.unwrap_err().to_string(), "invalid num 3");
    }

    #[test]
    fn test_to_original() {
        assert_eq!(TestLayeredAdapter::to_original(&"false".to_string()), 0_i8);
        assert_eq!(TestLayeredAdapter::to_original(&"true".to_string()), 1_i8);
    }

    #[test]
    fn test_thrift_field() {
        assert_eq!(
            TestLayeredAdapter::from_thrift_field(0_i8, 42, TypeId::of::<DummyParentStruct>())
                .unwrap(),
            "false"
        );
        assert_eq!(
            TestLayeredAdapter::from_thrift_default(0_i8, 42, TypeId::of::<DummyParentStruct>()),
            "false"
        );
        assert_eq!(
            TestLayeredAdapter::to_thrift_field(
                &"false".to_string(),
                42,
                TypeId::of::<DummyParentStruct>()
            ),
            0_i8
        );
    }
}
