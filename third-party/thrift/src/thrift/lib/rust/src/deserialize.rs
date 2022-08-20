/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
use std::collections::HashMap;
use std::collections::HashSet;
use std::hash::Hash;
use std::sync::Arc;

use bytes::Bytes;
use ordered_float::OrderedFloat;

use crate::protocol::should_break;
use crate::protocol::ProtocolReader;
use crate::Result;

// Read trait. Every type that needs to be deserialized will implement this trait.
pub trait Deserialize<P>
where
    P: ProtocolReader,
{
    fn read(p: &mut P) -> Result<Self>
    where
        Self: Sized;
}

impl<P, T> Deserialize<P> for Box<T>
where
    P: ProtocolReader,
    T: Deserialize<P>,
{
    #[inline]
    fn read(p: &mut P) -> Result<Self> {
        T::read(p).map(Box::new)
    }
}

impl<P, T> Deserialize<P> for Arc<T>
where
    P: ProtocolReader,
    T: Deserialize<P>,
{
    fn read(p: &mut P) -> Result<Self> {
        T::read(p).map(Arc::new)
    }
}

impl<P> Deserialize<P> for ()
where
    P: ProtocolReader,
{
    #[inline]
    fn read(_p: &mut P) -> Result<Self> {
        Ok(())
    }
}

impl<P> Deserialize<P> for bool
where
    P: ProtocolReader,
{
    #[inline]
    fn read(p: &mut P) -> Result<Self> {
        p.read_bool()
    }
}

impl<P> Deserialize<P> for i8
where
    P: ProtocolReader,
{
    #[inline]
    fn read(p: &mut P) -> Result<Self> {
        p.read_byte()
    }
}

impl<P> Deserialize<P> for i16
where
    P: ProtocolReader,
{
    #[inline]
    fn read(p: &mut P) -> Result<Self> {
        p.read_i16()
    }
}

impl<P> Deserialize<P> for i32
where
    P: ProtocolReader,
{
    #[inline]
    fn read(p: &mut P) -> Result<Self> {
        p.read_i32()
    }
}

impl<P> Deserialize<P> for i64
where
    P: ProtocolReader,
{
    #[inline]
    fn read(p: &mut P) -> Result<Self> {
        p.read_i64()
    }
}

impl<P> Deserialize<P> for f64
where
    P: ProtocolReader,
{
    #[inline]
    fn read(p: &mut P) -> Result<Self> {
        p.read_double()
    }
}

impl<P> Deserialize<P> for f32
where
    P: ProtocolReader,
{
    #[inline]
    fn read(p: &mut P) -> Result<Self> {
        p.read_float()
    }
}

impl<P> Deserialize<P> for OrderedFloat<f64>
where
    P: ProtocolReader,
{
    #[inline]
    fn read(p: &mut P) -> Result<Self> {
        p.read_double().map(OrderedFloat)
    }
}

impl<P> Deserialize<P> for OrderedFloat<f32>
where
    P: ProtocolReader,
{
    #[inline]
    fn read(p: &mut P) -> Result<Self> {
        p.read_float().map(OrderedFloat)
    }
}

impl<P> Deserialize<P> for String
where
    P: ProtocolReader,
{
    #[inline]
    fn read(p: &mut P) -> Result<Self> {
        p.read_string()
    }
}

impl<P> Deserialize<P> for Bytes
where
    P: ProtocolReader,
{
    #[inline]
    fn read(p: &mut P) -> Result<Self> {
        p.read_binary()
    }
}

impl<P> Deserialize<P> for Vec<u8>
where
    P: ProtocolReader,
{
    #[inline]
    fn read(p: &mut P) -> Result<Self> {
        p.read_binary()
    }
}

impl<P, T> Deserialize<P> for BTreeSet<T>
where
    P: ProtocolReader,
    T: Deserialize<P> + Ord,
{
    fn read(p: &mut P) -> Result<Self> {
        let (_elem_ty, len) = p.read_set_begin()?;
        let mut bset = BTreeSet::new();

        if let Some(0) = len {
            return Ok(bset);
        }

        let mut idx = 0;
        loop {
            let more = p.read_set_value_begin()?;
            if !more {
                break;
            }
            let item = Deserialize::read(p)?;
            p.read_set_value_end()?;
            bset.insert(item);

            idx += 1;
            if should_break(len, more, idx) {
                break;
            }
        }
        p.read_set_end()?;
        Ok(bset)
    }
}

impl<P, T, S> Deserialize<P> for HashSet<T, S>
where
    P: ProtocolReader,
    T: Deserialize<P> + Hash + Eq,
    S: std::hash::BuildHasher + Default,
{
    fn read(p: &mut P) -> Result<Self> {
        let (_elem_ty, len) = p.read_set_begin()?;
        let mut hset =
            HashSet::with_capacity_and_hasher(len.unwrap_or_default(), Default::default());

        if let Some(0) = len {
            return Ok(hset);
        }

        let mut idx = 0;
        loop {
            let more = p.read_set_value_begin()?;
            if !more {
                break;
            }
            let item = Deserialize::read(p)?;
            p.read_set_value_end()?;
            hset.insert(item);

            idx += 1;
            if should_break(len, more, idx) {
                break;
            }
        }
        p.read_set_end()?;
        Ok(hset)
    }
}

impl<P, K, V> Deserialize<P> for BTreeMap<K, V>
where
    P: ProtocolReader,
    K: Deserialize<P> + Ord,
    V: Deserialize<P>,
{
    fn read(p: &mut P) -> Result<Self> {
        let (_key_ty, _val_ty, len) = p.read_map_begin()?;
        let mut btree = BTreeMap::new();

        if let Some(0) = len {
            return Ok(btree);
        }

        let mut idx = 0;
        loop {
            let more = p.read_map_key_begin()?;
            if !more {
                break;
            }
            let key = Deserialize::read(p)?;
            p.read_map_value_begin()?;
            let val = Deserialize::read(p)?;
            p.read_map_value_end()?;
            btree.insert(key, val);

            idx += 1;
            if should_break(len, more, idx) {
                break;
            }
        }
        p.read_map_end()?;
        Ok(btree)
    }
}

impl<P, K, V, S> Deserialize<P> for HashMap<K, V, S>
where
    P: ProtocolReader,
    K: Deserialize<P> + Hash + Eq,
    V: Deserialize<P>,
    S: std::hash::BuildHasher + Default,
{
    fn read(p: &mut P) -> Result<Self> {
        let (_key_ty, _val_ty, len) = p.read_map_begin()?;
        let mut hmap =
            HashMap::with_capacity_and_hasher(len.unwrap_or_default(), Default::default());

        if let Some(0) = len {
            return Ok(hmap);
        }

        let mut idx = 0;
        loop {
            let more = p.read_map_key_begin()?;
            if !more {
                break;
            }
            let key = Deserialize::read(p)?;
            p.read_map_value_begin()?;
            let val = Deserialize::read(p)?;
            p.read_map_value_end()?;
            hmap.insert(key, val);

            idx += 1;
            if should_break(len, more, idx) {
                break;
            }
        }
        p.read_map_end()?;
        Ok(hmap)
    }
}

impl<P, T> Deserialize<P> for Vec<T>
where
    P: ProtocolReader,
    T: Deserialize<P> + crate::ttype::GetTType, // GetTType just to exclude Vec<u8>
{
    /// Vec<T> is Thrift List type
    fn read(p: &mut P) -> Result<Self> {
        let (_elem_ty, len) = p.read_list_begin()?;
        let mut list = Vec::with_capacity(len.unwrap_or_default() as usize);

        if let Some(0) = len {
            return Ok(list);
        }

        let mut idx = 0;
        loop {
            let more = p.read_list_value_begin()?;
            if !more {
                break;
            }
            let item = Deserialize::read(p)?;
            p.read_list_value_end()?;
            list.push(item);

            idx += 1;
            if should_break(len, more, idx) {
                break;
            }
        }
        p.read_list_end()?;
        Ok(list)
    }
}
