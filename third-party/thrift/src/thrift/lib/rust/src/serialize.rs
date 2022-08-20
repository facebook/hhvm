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

use crate::protocol::ProtocolWriter;
use crate::ttype::GetTType;

// Write trait. Every type that needs to be serialized will implement this trait.
pub trait Serialize<P>
where
    P: ProtocolWriter,
{
    fn write(&self, p: &mut P);
}

impl<P, T> Serialize<P> for &T
where
    P: ProtocolWriter,
    T: ?Sized + Serialize<P>,
{
    fn write(&self, p: &mut P) {
        (**self).write(p);
    }
}

impl<P, T> Serialize<P> for Box<T>
where
    P: ProtocolWriter,
    T: Serialize<P>,
{
    #[inline]
    fn write(&self, p: &mut P) {
        self.as_ref().write(p)
    }
}

impl<P, T> Serialize<P> for Arc<T>
where
    P: ProtocolWriter,
    T: Serialize<P>,
{
    fn write(&self, p: &mut P) {
        (**self).write(p);
    }
}

impl<P> Serialize<P> for ()
where
    P: ProtocolWriter,
{
    #[inline]
    fn write(&self, _p: &mut P) {}
}

impl<P> Serialize<P> for bool
where
    P: ProtocolWriter,
{
    #[inline]
    fn write(&self, p: &mut P) {
        p.write_bool(*self)
    }
}

impl<P> Serialize<P> for i8
where
    P: ProtocolWriter,
{
    #[inline]
    fn write(&self, p: &mut P) {
        p.write_byte(*self)
    }
}

impl<P> Serialize<P> for i16
where
    P: ProtocolWriter,
{
    #[inline]
    fn write(&self, p: &mut P) {
        p.write_i16(*self)
    }
}

impl<P> Serialize<P> for i32
where
    P: ProtocolWriter,
{
    #[inline]
    fn write(&self, p: &mut P) {
        p.write_i32(*self)
    }
}

impl<P> Serialize<P> for i64
where
    P: ProtocolWriter,
{
    #[inline]
    fn write(&self, p: &mut P) {
        p.write_i64(*self)
    }
}

impl<P> Serialize<P> for f64
where
    P: ProtocolWriter,
{
    #[inline]
    fn write(&self, p: &mut P) {
        p.write_double(*self)
    }
}

impl<P> Serialize<P> for f32
where
    P: ProtocolWriter,
{
    #[inline]
    fn write(&self, p: &mut P) {
        p.write_float(*self)
    }
}

impl<P> Serialize<P> for OrderedFloat<f64>
where
    P: ProtocolWriter,
{
    #[inline]
    fn write(&self, p: &mut P) {
        p.write_double(self.0)
    }
}

impl<P> Serialize<P> for OrderedFloat<f32>
where
    P: ProtocolWriter,
{
    #[inline]
    fn write(&self, p: &mut P) {
        p.write_float(self.0)
    }
}

impl<P> Serialize<P> for String
where
    P: ProtocolWriter,
{
    #[inline]
    fn write(&self, p: &mut P) {
        p.write_string(self.as_str())
    }
}

impl<P> Serialize<P> for str
where
    P: ProtocolWriter,
{
    #[inline]
    fn write(&self, p: &mut P) {
        p.write_string(self)
    }
}

impl<P> Serialize<P> for Bytes
where
    P: ProtocolWriter,
{
    #[inline]
    fn write(&self, p: &mut P) {
        p.write_binary(self.as_ref())
    }
}

impl<P> Serialize<P> for Vec<u8>
where
    P: ProtocolWriter,
{
    #[inline]
    fn write(&self, p: &mut P) {
        p.write_binary(self.as_ref())
    }
}

impl<P> Serialize<P> for [u8]
where
    P: ProtocolWriter,
{
    #[inline]
    fn write(&self, p: &mut P) {
        p.write_binary(self)
    }
}

impl<P, T> Serialize<P> for BTreeSet<T>
where
    P: ProtocolWriter,
    T: GetTType + Ord,
    T: Serialize<P>,
{
    fn write(&self, p: &mut P) {
        p.write_set_begin(T::TTYPE, self.len());
        for item in self.iter() {
            p.write_set_value_begin();
            item.write(p);
        }
        p.write_set_end();
    }
}

impl<P, T, S> Serialize<P> for HashSet<T, S>
where
    P: ProtocolWriter,
    T: GetTType + Hash + Eq,
    T: Serialize<P>,
    S: std::hash::BuildHasher,
{
    fn write(&self, p: &mut P) {
        p.write_set_begin(T::TTYPE, self.len());
        for item in self.iter() {
            p.write_set_value_begin();
            item.write(p);
        }
        p.write_set_end();
    }
}

impl<P, K, V> Serialize<P> for BTreeMap<K, V>
where
    P: ProtocolWriter,
    K: GetTType + Ord,
    K: Serialize<P>,
    V: GetTType,
    V: Serialize<P>,
{
    fn write(&self, p: &mut P) {
        p.write_map_begin(K::TTYPE, V::TTYPE, self.len());
        for (k, v) in self.iter() {
            p.write_map_key_begin();
            k.write(p);
            p.write_map_value_begin();
            v.write(p);
        }
        p.write_map_end();
    }
}

impl<P, K, V, S> Serialize<P> for HashMap<K, V, S>
where
    P: ProtocolWriter,
    K: GetTType + Hash + Eq,
    K: Serialize<P>,
    V: GetTType,
    V: Serialize<P>,
    S: std::hash::BuildHasher,
{
    fn write(&self, p: &mut P) {
        p.write_map_begin(K::TTYPE, V::TTYPE, self.len());
        for (k, v) in self.iter() {
            p.write_map_key_begin();
            k.write(p);
            p.write_map_value_begin();
            v.write(p);
        }
        p.write_map_end();
    }
}

impl<P, T> Serialize<P> for Vec<T>
where
    P: ProtocolWriter,
    T: GetTType,
    T: Serialize<P>,
{
    /// Vec<T> is Thrift List type
    fn write(&self, p: &mut P) {
        p.write_list_begin(T::TTYPE, self.len());
        for item in self.iter() {
            p.write_list_value_begin();
            item.write(p);
        }
        p.write_list_end();
    }
}

impl<P, T> Serialize<P> for [T]
where
    P: ProtocolWriter,
    T: GetTType,
    T: Serialize<P>,
{
    /// \[T\] is Thrift List type
    fn write(&self, p: &mut P) {
        p.write_list_begin(T::TTYPE, self.len());
        for item in self.iter() {
            p.write_list_value_begin();
            item.write(p);
        }
        p.write_list_end();
    }
}
