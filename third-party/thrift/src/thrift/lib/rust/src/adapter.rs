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

use std::collections::BTreeMap;
use std::collections::BTreeSet;
use std::fmt::Debug;
use std::marker::PhantomData;

use crate::metadata::ThriftAnnotations;

pub trait ThriftAdapter {
    /// Aka the "from" type.
    type StandardType;
    /// Aka the "to" type.
    type AdaptedType: Clone + Debug + PartialEq + Send + Sync;

    /// The error type thrown if `from_thrift` fails during deserialization.
    type Error: Into<anyhow::Error> + Debug;

    /// Converts an instance of `StandardType` to `AdaptedType` during deserialization.
    ///
    /// The `Err` returned by this method will be propagated as a deserialization error.
    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error>;

    // TODO(emersonford): should we support a moved param here?
    /// Converts from the given `AdaptedType` back to the given `StandardType` during
    /// serialization.
    ///
    /// This must be an infallible operation as `serialize` is infallible.
    ///
    /// WARNING: you should be particularly cautious when using `.unwrap()` or any other panic-able
    /// methods in this method. If this method panics, it will be at serialization time and not at
    /// the Thrift struct creation time, meaning it will be extremely difficult to debug what the
    /// true "source" of the panic.
    ///
    /// If your `AdaptedType` -> `StandardType` conversion is truly fallible, you probably shouldn't
    /// use an adapter to begin with.
    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType;

    /// Method called when this adapter is used on a Thrift struct's field. Provides information
    /// about the specific field ID in `field_id`. The type of the struct that owns this field is
    /// passed in as `T`.
    ///
    /// Defaults to calling `from_thrift`.
    fn from_thrift_field<T: ThriftAnnotations>(
        value: Self::StandardType,
        _field_id: i16,
    ) -> Result<Self::AdaptedType, Self::Error> {
        Self::from_thrift(value)
    }

    /// Method called when this adapter is used on a Thrift struct's field. Provides information
    /// about the specific field ID in `field_id`. The type of the struct that owns this field is
    /// passed in as `T`.
    ///
    /// Defaults to calling `to_thrift`.
    fn to_thrift_field<T: ThriftAnnotations>(
        value: &Self::AdaptedType,
        _field_id: i16,
    ) -> Self::StandardType {
        Self::to_thrift(value)
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
    fn from_thrift_default<T: ThriftAnnotations>(
        value: Self::StandardType,
        field_id: i16,
    ) -> Self::AdaptedType {
        Self::from_thrift_field::<T>(value, field_id).unwrap_or_else(|e| {
            panic!(
                "`from_thrift_field` must not return an `Err` on field ID {} for its default value, but it did: '{:?}'",
                field_id, e
            );
        })
    }
}

// NOTE: we define where bounds on the structs themselves here to improve error messaging during
// the Thrift compilation process.

/// Layers multiple [ThriftTypeAdapter] together. Used when multiple Thrift typedefs with adapters
/// are layered on each other.
pub struct LayeredThriftAdapter<Fst, Snd>
where
    Fst: ThriftAdapter,
    Snd: ThriftAdapter,
    Fst::StandardType: Into<Snd::AdaptedType>,
    Snd::AdaptedType: Into<Fst::StandardType>,
{
    _inner_first: PhantomData<Fst>,
    _inner_second: PhantomData<Snd>,
}

impl<Fst, Snd> ThriftAdapter for LayeredThriftAdapter<Fst, Snd>
where
    Fst: ThriftAdapter,
    Snd: ThriftAdapter,
    Fst::StandardType: Into<Snd::AdaptedType>,
    Snd::AdaptedType: Into<Fst::StandardType>,
{
    type StandardType = <Snd as ThriftAdapter>::StandardType;
    type AdaptedType = <Fst as ThriftAdapter>::AdaptedType;

    type Error = anyhow::Error;

    #[inline]
    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        <Fst as ThriftAdapter>::from_thrift(
            <Snd as ThriftAdapter>::from_thrift(value)
                .map_err(Into::<anyhow::Error>::into)?
                .into(),
        )
        .map_err(Into::<anyhow::Error>::into)
    }

    #[inline]
    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        <Snd as ThriftAdapter>::to_thrift(&<Fst as ThriftAdapter>::to_thrift(value).into())
    }

    #[inline]
    fn from_thrift_field<T: ThriftAnnotations>(
        value: Self::StandardType,
        field_id: i16,
    ) -> Result<Self::AdaptedType, Self::Error> {
        <Fst as ThriftAdapter>::from_thrift_field::<T>(
            <Snd as ThriftAdapter>::from_thrift_field::<T>(value, field_id)
                .map_err(Into::<anyhow::Error>::into)?
                .into(),
            field_id,
        )
        .map_err(Into::<anyhow::Error>::into)
    }

    #[inline]
    fn to_thrift_field<T: ThriftAnnotations>(
        value: &Self::AdaptedType,
        field_id: i16,
    ) -> Self::StandardType {
        <Snd as ThriftAdapter>::to_thrift_field::<T>(
            &<Fst as ThriftAdapter>::to_thrift_field::<T>(value, field_id).into(),
            field_id,
        )
    }

    #[inline]
    fn from_thrift_default<T: ThriftAnnotations>(
        value: Self::StandardType,
        field_id: i16,
    ) -> Self::AdaptedType {
        <Fst as ThriftAdapter>::from_thrift_default::<T>(
            <Snd as ThriftAdapter>::from_thrift_default::<T>(value, field_id).into(),
            field_id,
        )
    }
}

/// Transforms the given adapter `A` into an adapter with the signature `Vec<StandardType>`
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
    type StandardType = Vec<A::StandardType>;
    type AdaptedType = Vec<A::AdaptedType>;

    type Error = A::Error;

    #[inline]
    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        value
            .into_iter()
            .map(|elem| A::from_thrift(elem))
            .collect::<Result<Self::AdaptedType, Self::Error>>()
    }

    #[inline]
    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        value.iter().map(|elem| A::to_thrift(elem)).collect()
    }

    #[inline]
    fn from_thrift_field<T: ThriftAnnotations>(
        value: Self::StandardType,
        field_id: i16,
    ) -> Result<Self::AdaptedType, Self::Error> {
        value
            .into_iter()
            .map(|elem| A::from_thrift_field::<T>(elem, field_id))
            .collect::<Result<Self::AdaptedType, Self::Error>>()
    }

    #[inline]
    fn to_thrift_field<T: ThriftAnnotations>(
        value: &Self::AdaptedType,
        field_id: i16,
    ) -> Self::StandardType {
        value
            .iter()
            .map(|elem| A::to_thrift_field::<T>(elem, field_id))
            .collect()
    }

    #[inline]
    fn from_thrift_default<T: ThriftAnnotations>(
        value: Self::StandardType,
        field_id: i16,
    ) -> Self::AdaptedType {
        value
            .into_iter()
            .map(|elem| A::from_thrift_default::<T>(elem, field_id))
            .collect()
    }
}

/// Transforms the given adapter `A` into an adapter with the signature `BTreeSet<StandardType>`
/// -> `BTreeSet<AdaptedType>`. Because Rust doesn't have HKT, we cannot make this "generic" over
/// multiple collection types.
pub struct SetMapAdapter<A>
where
    A: ThriftAdapter,
    A::StandardType: Ord + PartialEq,
    A::AdaptedType: Ord + PartialEq,
{
    _inner_adapter: PhantomData<A>,
}

impl<A> ThriftAdapter for SetMapAdapter<A>
where
    A: ThriftAdapter,
    A::StandardType: Ord + PartialEq,
    A::AdaptedType: Ord + PartialEq,
{
    type StandardType = BTreeSet<A::StandardType>;
    type AdaptedType = BTreeSet<A::AdaptedType>;

    type Error = A::Error;

    #[inline]
    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        value
            .into_iter()
            .map(|elem| A::from_thrift(elem))
            .collect::<Result<Self::AdaptedType, Self::Error>>()
    }

    #[inline]
    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        value.iter().map(|elem| A::to_thrift(elem)).collect()
    }

    #[inline]
    fn from_thrift_field<T: ThriftAnnotations>(
        value: Self::StandardType,
        field_id: i16,
    ) -> Result<Self::AdaptedType, Self::Error> {
        value
            .into_iter()
            .map(|elem| A::from_thrift_field::<T>(elem, field_id))
            .collect::<Result<Self::AdaptedType, Self::Error>>()
    }

    #[inline]
    fn to_thrift_field<T: ThriftAnnotations>(
        value: &Self::AdaptedType,
        field_id: i16,
    ) -> Self::StandardType {
        value
            .iter()
            .map(|elem| A::to_thrift_field::<T>(elem, field_id))
            .collect()
    }

    #[inline]
    fn from_thrift_default<T: ThriftAnnotations>(
        value: Self::StandardType,
        field_id: i16,
    ) -> Self::AdaptedType {
        value
            .into_iter()
            .map(|elem| A::from_thrift_default::<T>(elem, field_id))
            .collect()
    }
}

/// Transforms the given adapter `KA` and `KV` into an adapter with the signature
/// `BTreeMap<KA::StandardType, KV::StandardType>` -> `BTreeMap<KA::AdaptedType, KV::AdaptedType>`.
/// Because Rust doesn't have HKT, we cannot make this "generic" over multiple collection types.
pub struct MapMapAdapter<KA, KV>
where
    KA: ThriftAdapter,
    KV: ThriftAdapter,
    KA::StandardType: Ord + PartialEq,
    KA::AdaptedType: Ord + PartialEq,
{
    _inner_key_adapter: PhantomData<KA>,
    _inner_val_adapter: PhantomData<KV>,
}

impl<KA, KV> ThriftAdapter for MapMapAdapter<KA, KV>
where
    KA: ThriftAdapter,
    KV: ThriftAdapter,
    KA::StandardType: Ord + PartialEq,
    KA::AdaptedType: Ord + PartialEq,
{
    type StandardType = BTreeMap<KA::StandardType, KV::StandardType>;
    type AdaptedType = BTreeMap<KA::AdaptedType, KV::AdaptedType>;

    type Error = anyhow::Error;

    #[inline]
    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        value
            .into_iter()
            .map(|(key, val)| {
                Ok((
                    KA::from_thrift(key).map_err(Into::<anyhow::Error>::into)?,
                    KV::from_thrift(val).map_err(Into::<anyhow::Error>::into)?,
                ))
            })
            .collect::<Result<Self::AdaptedType, Self::Error>>()
    }

    #[inline]
    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        value
            .iter()
            .map(|(key, val)| (KA::to_thrift(key), KV::to_thrift(val)))
            .collect::<Self::StandardType>()
    }

    #[inline]
    fn from_thrift_field<T: ThriftAnnotations>(
        value: Self::StandardType,
        field_id: i16,
    ) -> Result<Self::AdaptedType, Self::Error> {
        value
            .into_iter()
            .map(|(key, val)| {
                Ok((
                    KA::from_thrift_field::<T>(key, field_id)
                        .map_err(Into::<anyhow::Error>::into)?,
                    KV::from_thrift_field::<T>(val, field_id)
                        .map_err(Into::<anyhow::Error>::into)?,
                ))
            })
            .collect::<Result<Self::AdaptedType, Self::Error>>()
    }

    #[inline]
    fn to_thrift_field<T: ThriftAnnotations>(
        value: &Self::AdaptedType,
        field_id: i16,
    ) -> Self::StandardType {
        value
            .iter()
            .map(|(key, val)| {
                (
                    KA::to_thrift_field::<T>(key, field_id),
                    KV::to_thrift_field::<T>(val, field_id),
                )
            })
            .collect::<Self::StandardType>()
    }

    #[inline]
    fn from_thrift_default<T: ThriftAnnotations>(
        value: Self::StandardType,
        field_id: i16,
    ) -> Self::AdaptedType {
        value
            .into_iter()
            .map(|(key, val)| {
                (
                    KA::from_thrift_default::<T>(key, field_id),
                    KV::from_thrift_default::<T>(val, field_id),
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
    type StandardType = T;
    type AdaptedType = T;

    type Error = std::convert::Infallible;

    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        Ok(value)
    }

    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        value.clone()
    }
}

/// Convenience trait for newtype adapters.
///
/// This trait simplifies construction of adapters that newtype the standard
/// type. On top of the standard bounds, the newtype (adapted type) needs to
/// also have a [`From`] and [`Into`] implementation for the standard thrift
/// type.
///
/// # Examples
///
/// ```
/// use fbthrift::adapter::NewTypeAdapter;
///
/// #[derive(Clone, Debug)]
/// struct MyInt(i64);
///
/// impl From<MyInt> for i64 {
///     fn from(v: MyInt) -> Self { v.0 }
/// }
///
/// impl From<i64> for MyInt {
///     fn from(v: i64) -> Self { Self(v) }
/// }
///
/// struct MyIntAdapter;
///
/// impl NewTypeAdapter for MyIntAdapter {
///     type StandardType = i64;
///     type AdaptedType = MyInt;
/// }
/// ```
pub trait NewTypeAdapter {
    /// The thrift type that the adapter interprets.
    type StandardType;
    /// The Rust type to adapt to.
    type AdaptedType;
}

impl<Adapter, StandardType, AdaptedType> ThriftAdapter for Adapter
where
    Adapter: NewTypeAdapter<StandardType = StandardType, AdaptedType = AdaptedType>,
    AdaptedType: From<StandardType> + Into<StandardType> + Clone + Debug + Send + Sync + PartialEq,
{
    type StandardType = <Self as NewTypeAdapter>::StandardType;
    type AdaptedType = <Self as NewTypeAdapter>::AdaptedType;

    type Error = <Self::AdaptedType as TryFrom<Self::StandardType>>::Error;

    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        value.clone().into()
    }

    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        Self::AdaptedType::try_from(value)
    }
}

#[cfg(test)]
mod tests {
    use std::any::TypeId;

    use super::*;

    // Represents the following flow
    // String (Final Adapted Type) <- BoolToStringAdapter -> Bool <- I64ToBoolAdapter -> i64
    // <- I8ToI64Adapter -> i8 (Original Thrift type)
    struct DummyParentStruct {}

    impl ThriftAnnotations for DummyParentStruct {}

    struct BoolToStringAdapter {}

    impl ThriftAdapter for BoolToStringAdapter {
        type StandardType = bool;
        type AdaptedType = String;

        type Error = anyhow::Error;

        fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
            Ok(value.to_string())
        }

        fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
            value == "true"
        }

        fn from_thrift_field<T: ThriftAnnotations>(
            value: Self::StandardType,
            field_id: i16,
        ) -> Result<Self::AdaptedType, Self::Error> {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), TypeId::of::<T>());

            Self::from_thrift(value)
        }

        fn to_thrift_field<T: ThriftAnnotations>(
            value: &Self::AdaptedType,
            field_id: i16,
        ) -> Self::StandardType {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), TypeId::of::<T>());

            Self::to_thrift(value)
        }

        fn from_thrift_default<T: ThriftAnnotations>(
            value: Self::StandardType,
            field_id: i16,
        ) -> Self::AdaptedType {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), TypeId::of::<T>());

            Self::from_thrift_field::<T>(value, field_id).unwrap()
        }
    }

    struct I64ToBoolAdapter {}

    impl ThriftAdapter for I64ToBoolAdapter {
        type StandardType = i64;
        type AdaptedType = bool;

        type Error = anyhow::Error;

        fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
            match value {
                0 => Ok(false),
                1 => Ok(true),
                other => anyhow::bail!("invalid num {}", other),
            }
        }

        fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
            if *value { 1 } else { 0 }
        }

        fn from_thrift_field<T: ThriftAnnotations>(
            value: Self::StandardType,
            field_id: i16,
        ) -> Result<Self::AdaptedType, Self::Error> {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), TypeId::of::<T>());

            Self::from_thrift(value)
        }

        fn to_thrift_field<T: ThriftAnnotations>(
            value: &Self::AdaptedType,
            field_id: i16,
        ) -> Self::StandardType {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), TypeId::of::<T>());

            Self::to_thrift(value)
        }

        fn from_thrift_default<T: ThriftAnnotations>(
            value: Self::StandardType,
            field_id: i16,
        ) -> Self::AdaptedType {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), TypeId::of::<T>());

            Self::from_thrift_field::<T>(value, field_id).unwrap()
        }
    }

    struct I8ToI64Adapter {}

    impl ThriftAdapter for I8ToI64Adapter {
        type StandardType = i8;
        type AdaptedType = i64;

        type Error = anyhow::Error;

        fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
            Ok(value.into())
        }

        fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
            (*value).try_into().unwrap()
        }

        fn from_thrift_field<T: ThriftAnnotations>(
            value: Self::StandardType,
            field_id: i16,
        ) -> Result<Self::AdaptedType, Self::Error> {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), TypeId::of::<T>());

            Self::from_thrift(value)
        }

        fn to_thrift_field<T: ThriftAnnotations>(
            value: &Self::AdaptedType,
            field_id: i16,
        ) -> Self::StandardType {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), TypeId::of::<T>());

            Self::to_thrift(value)
        }

        fn from_thrift_default<T: ThriftAnnotations>(
            value: Self::StandardType,
            field_id: i16,
        ) -> Self::AdaptedType {
            assert_eq!(field_id, 42);
            assert_eq!(TypeId::of::<DummyParentStruct>(), TypeId::of::<T>());

            Self::from_thrift_field::<T>(value, field_id).unwrap()
        }
    }

    type TestLayeredAdapter = LayeredThriftAdapter<
        BoolToStringAdapter,
        LayeredThriftAdapter<I64ToBoolAdapter, I8ToI64Adapter>,
    >;

    #[test]
    fn test_from_thrift() {
        assert_eq!(TestLayeredAdapter::from_thrift(0_i8).unwrap(), "false");
        assert_eq!(TestLayeredAdapter::from_thrift(1_i8).unwrap(), "true");
    }

    #[test]
    fn test_from_thrift_error() {
        let res = TestLayeredAdapter::from_thrift(3_i8);

        assert!(res.is_err());
        assert_eq!(res.unwrap_err().to_string(), "invalid num 3");
    }

    #[test]
    fn test_to_thrift() {
        assert_eq!(TestLayeredAdapter::to_thrift(&"false".to_string()), 0_i8);
        assert_eq!(TestLayeredAdapter::to_thrift(&"true".to_string()), 1_i8);
    }

    #[test]
    fn test_thrift_field() {
        assert_eq!(
            TestLayeredAdapter::from_thrift_field::<DummyParentStruct>(0_i8, 42).unwrap(),
            "false"
        );
        assert_eq!(
            TestLayeredAdapter::from_thrift_default::<DummyParentStruct>(0_i8, 42),
            "false"
        );
        assert_eq!(
            TestLayeredAdapter::to_thrift_field::<DummyParentStruct>(&"false".to_string(), 42),
            0_i8
        );
    }
}
