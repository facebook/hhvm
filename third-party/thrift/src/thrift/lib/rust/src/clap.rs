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

use std::ffi::OsStr;
use std::marker::PhantomData;
use std::str::FromStr;

use clap::Arg;
use clap::Command;
use clap::builder::PossibleValue;
use clap::builder::PossibleValuesParser;
use clap::builder::TypedValueParser;
use clap::error::Error;

use crate::ThriftEnum;

pub struct ThriftEnumValueParser<T>(PhantomData<T>);

impl<T> ThriftEnumValueParser<T> {
    pub fn new() -> Self {
        ThriftEnumValueParser(PhantomData)
    }
}

impl<T> Clone for ThriftEnumValueParser<T> {
    fn clone(&self) -> Self {
        ThriftEnumValueParser(PhantomData)
    }
}

impl<T> TypedValueParser for ThriftEnumValueParser<T>
where
    T: ThriftEnum + FromStr + Send + Sync + Clone + 'static,
{
    type Value = T;

    fn parse_ref(
        &self,
        cmd: &Command,
        arg: Option<&Arg>,
        value: &OsStr,
    ) -> Result<Self::Value, Error> {
        if let Some(value) = value.to_str() {
            let ignore_case = arg.is_some_and(|a| a.is_ignore_case_set());
            if ignore_case {
                let value_snake_case = value.replace('-', "_");
                for (variant, name) in T::enumerate() {
                    if unicase::eq(*name, &value_snake_case) {
                        return Ok(variant.clone());
                    }
                }
            } else {
                if let Ok(variant) = T::from_str(value) {
                    return Ok(variant);
                }
            }
        }

        Err(PossibleValuesParser::new(self.possible_values().unwrap())
            .parse_ref(cmd, arg, value)
            .unwrap_err())
    }

    fn possible_values(&self) -> Option<Box<dyn Iterator<Item = PossibleValue>>> {
        Some(Box::new(T::variants().iter().map(PossibleValue::from)))
    }
}
