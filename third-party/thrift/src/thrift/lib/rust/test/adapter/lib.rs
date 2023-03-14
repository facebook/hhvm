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

use std::fmt::Debug;
use std::marker::PhantomData;
use std::num::NonZeroI64;

use fbthrift::adapter::ThriftAdapter;

#[derive(Default, Debug, Clone, PartialEq)]
pub struct CustomString(pub String);

pub struct StringAdapter {}

impl ThriftAdapter for StringAdapter {
    type StandardType = String;
    type Error = std::convert::Infallible;

    type AdaptedType = CustomString;

    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        Ok(CustomString(value))
    }

    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        value.0.clone()
    }
}

pub struct NonZeroI64Adapter {}

impl ThriftAdapter for NonZeroI64Adapter {
    type StandardType = i64;
    type Error = anyhow::Error;

    type AdaptedType = NonZeroI64;

    fn from_thrift(value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        match NonZeroI64::new(value) {
            Some(v) => Ok(v),
            None => {
                anyhow::bail!("Given i64 is not non-zero: {}", value);
            }
        }
    }

    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        value.get()
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct SortedVec(pub Vec<String>);

pub struct ListAdapter {}

impl ThriftAdapter for ListAdapter {
    type StandardType = Vec<String>;
    type Error = std::convert::Infallible;

    type AdaptedType = SortedVec;

    fn from_thrift(mut value: Self::StandardType) -> Result<Self::AdaptedType, Self::Error> {
        value.sort();

        Ok(SortedVec(value))
    }

    fn to_thrift(value: &Self::AdaptedType) -> Self::StandardType {
        value.0.clone()
    }
}

pub struct IdentityAdapter<T>
where
    T: Clone + Debug + Send + Sync + PartialEq,
{
    inner: PhantomData<T>,
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
