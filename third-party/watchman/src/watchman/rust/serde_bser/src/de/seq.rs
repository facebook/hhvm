/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use serde::de;

use super::Deserializer;
use super::read::DeRead;
use super::reentrant::ReentrantGuard;
use crate::errors::*;

pub struct SeqAccess<'a, R> {
    de: &'a mut Deserializer<R>,
    remaining: usize,
}

impl<'a, 'de, R> SeqAccess<'a, R>
where
    R: 'a + DeRead<'de>,
{
    /// Create a new `MapAccess`.
    ///
    /// `_guard` makes sure the caller is accounting for the recursion limit.
    pub fn new(de: &'a mut Deserializer<R>, nitems: usize, _guard: &ReentrantGuard) -> Self {
        SeqAccess {
            de,
            remaining: nitems,
        }
    }
}

impl<'a, 'de, R> de::SeqAccess<'de> for SeqAccess<'a, R>
where
    R: 'a + DeRead<'de>,
{
    type Error = Error;

    fn next_element_seed<T>(&mut self, seed: T) -> Result<Option<T::Value>>
    where
        T: de::DeserializeSeed<'de>,
    {
        if self.remaining == 0 {
            Ok(None)
        } else {
            self.remaining -= 1;
            let value = seed.deserialize(&mut *self.de)?;
            Ok(Some(value))
        }
    }
}
