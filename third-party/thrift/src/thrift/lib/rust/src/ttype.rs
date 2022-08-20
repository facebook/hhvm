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

use bytes::Bytes;
use ordered_float::OrderedFloat;

use crate::errors::ProtocolError;
use crate::Result;

/// Will be able to add missing types once #![feature(associated_consts)] lands
#[derive(PartialEq, Copy, Clone, Debug)]
#[repr(u8)]
pub enum TType {
    Stop = 0,
    Void = 1,
    Bool = 2,
    Byte = 3,
    // I08    = 3,
    Double = 4,
    I16 = 6,
    I32 = 8,
    I64 = 10,
    String = 11,
    // UTF7   = 11,
    Struct = 12,
    Map = 13,
    Set = 14,
    List = 15,
    UTF8 = 16,
    UTF16 = 17,
    Stream = 18,
    Float = 19,
}

impl TryFrom<i8> for TType {
    type Error = anyhow::Error;

    fn try_from(val: i8) -> Result<Self> {
        let ret = match val {
            0 => TType::Stop,
            1 => TType::Void,
            2 => TType::Bool,
            3 => TType::Byte,
            4 => TType::Double,
            6 => TType::I16,
            8 => TType::I32,
            10 => TType::I64,
            11 => TType::String,
            12 => TType::Struct,
            13 => TType::Map,
            14 => TType::Set,
            15 => TType::List,
            16 => TType::UTF8,
            17 => TType::UTF16,
            18 => TType::Stream,
            19 => TType::Float,
            _ => bail_err!(ProtocolError::InvalidTypeTag),
        };
        Ok(ret)
    }
}

impl From<TType> for String {
    fn from(t: TType) -> String {
        let tmp: &str = t.into();
        tmp.to_owned()
    }
}

impl From<TType> for &'static str {
    fn from(t: TType) -> &'static str {
        match t {
            TType::Stop => "STOP",
            TType::Void => "VOID",
            TType::Bool => "BOOL",
            TType::Byte => "BYTE",
            // TType::I08 => "I08",
            TType::Double => "DOUBLE",
            TType::I16 => "I16",
            TType::I32 => "I32",
            TType::I64 => "I64",
            TType::String => "STRING",
            // TType::UTF7 => "UTF7",
            TType::Struct => "STRUCT",
            TType::Map => "MAP",
            TType::Set => "SET",
            TType::List => "LIST",
            TType::UTF8 => "UTF8",
            TType::UTF16 => "UTF16",
            TType::Stream => "STREAM",
            TType::Float => "FLOAT",
        }
    }
}

// Get the ttype for a given type
pub trait GetTType {
    const TTYPE: TType;
}

impl GetTType for () {
    const TTYPE: TType = TType::Void;
}

impl GetTType for bool {
    const TTYPE: TType = TType::Bool;
}

impl GetTType for i8 {
    const TTYPE: TType = TType::Byte;
}

impl GetTType for i16 {
    const TTYPE: TType = TType::I16;
}

impl GetTType for i32 {
    const TTYPE: TType = TType::I32;
}

impl GetTType for i64 {
    const TTYPE: TType = TType::I64;
}

impl GetTType for f64 {
    const TTYPE: TType = TType::Double;
}

impl GetTType for f32 {
    const TTYPE: TType = TType::Float;
}

impl GetTType for OrderedFloat<f64> {
    const TTYPE: TType = TType::Double;
}

impl GetTType for OrderedFloat<f32> {
    const TTYPE: TType = TType::Float;
}

impl GetTType for String {
    const TTYPE: TType = TType::String;
}

impl GetTType for Bytes {
    const TTYPE: TType = TType::String;
}

// This only narrowly avoids a collision with Vec<T>, because
// it requires T: GetTType, but u8 does not impl it (only i8).
impl GetTType for Vec<u8> {
    const TTYPE: TType = TType::String;
}

impl<T> GetTType for BTreeSet<T>
where
    T: GetTType + Ord,
{
    const TTYPE: TType = TType::Set;
}

impl<T, S> GetTType for HashSet<T, S>
where
    T: GetTType + Hash + Eq,
    S: std::hash::BuildHasher,
{
    const TTYPE: TType = TType::Set;
}

impl<K, V> GetTType for BTreeMap<K, V>
where
    K: GetTType + Ord,
    V: GetTType,
{
    const TTYPE: TType = TType::Map;
}

impl<K, V, S> GetTType for HashMap<K, V, S>
where
    K: GetTType + Hash + Eq,
    V: GetTType,
    S: std::hash::BuildHasher,
{
    const TTYPE: TType = TType::Map;
}

impl<T> GetTType for Vec<T>
where
    T: GetTType,
{
    const TTYPE: TType = TType::List;
}
