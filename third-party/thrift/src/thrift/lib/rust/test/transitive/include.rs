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

pub use ::get_struct_name::GetStructName;
pub use ::get_struct_name_derive::GetStructName;
use fbthrift::adapter::ThriftAdapter;

pub struct WrapperAdapter<T>(PhantomData<T>);

#[derive(Clone, Debug, PartialEq)]
pub struct Wrapper<T: Clone + Debug + PartialEq + Sync + Send>(pub T);

impl<T> ThriftAdapter for WrapperAdapter<T>
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
}
