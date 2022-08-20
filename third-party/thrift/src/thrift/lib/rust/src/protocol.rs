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

use crate::binary_type::CopyFromBuf;
use crate::binary_type::Discard;
use crate::bufext::BufMutExt;
use crate::errors::ProtocolError;
use crate::framing::Framing;
use crate::framing::FramingDecoded;
use crate::framing::FramingEncoded;
use crate::framing::FramingEncodedFinal;
use crate::thrift_protocol::MessageType;
use crate::thrift_protocol::ProtocolID;
use crate::ttype::TType;
use crate::Result;

/// The maximum recursive depth the skip() function will traverse
pub const DEFAULT_RECURSION_DEPTH: i32 = 64;

/// Helper type alias to get the pre-finalization encoded type of a protocol frame.
pub type ProtocolEncoded<P> = FramingEncoded<<P as Protocol>::Frame>;

/// Helper type alias to get the final encoded type of a protocol frame
pub type ProtocolEncodedFinal<P> = FramingEncodedFinal<<P as Protocol>::Frame>;

/// Helper type alias to get the buffer type for a frame to be decoded.
pub type ProtocolDecoded<P> = FramingDecoded<<P as Protocol>::Frame>;

/// An instance of Protocol glues a Framing implementation to a serializer
/// (ProtocolWriter) and deserializer (ProtocolReader). It constructs, as
/// needed, a serializer to construct a frame with a given protocol, or a
/// deserializer from a frame into a stream of deserialized objects.
pub trait Protocol: 'static {
    /// Type of the framing implementation
    type Frame: Framing;

    /// Compute the size of a frame for a given protocol. This can be exact or too large, but
    /// must not be too small.
    type Sizer: ProtocolWriter<Final = usize>;

    /// Serialize into a buffer. The buffer is allocated with the size computed by Sizer, so
    /// it must be large enough.
    type Serializer: ProtocolWriter<Final = <<Self::Frame as Framing>::EncBuf as BufMutExt>::Final>;

    /// Set up a deserializer from a frame's buffer.
    type Deserializer: ProtocolReader;

    const PROTOCOL_ID: ProtocolID;

    fn serializer<SZ, SER>(sz: SZ, ser: SER) -> <Self::Serializer as ProtocolWriter>::Final
    where
        SZ: FnOnce(&mut Self::Sizer),
        SER: FnOnce(&mut Self::Serializer);

    fn deserializer(buf: <Self::Frame as Framing>::DecBuf) -> Self::Deserializer;

    fn into_buffer(_: Self::Deserializer) -> <Self::Frame as Framing>::DecBuf;
}

fn skip_inner<P: ProtocolReader + ?Sized>(
    p: &mut P,
    field_type: TType,
    max_depth: i32,
) -> Result<()> {
    if max_depth <= 0 {
        bail_err!(ProtocolError::SkipDepthExceeded)
    }

    match field_type {
        TType::Void => {}
        TType::Bool => {
            p.read_bool()?;
        }
        TType::Byte => {
            p.read_byte()?;
        }
        TType::I16 => {
            p.read_i16()?;
        }
        TType::I32 => {
            p.read_i32()?;
        }
        TType::I64 => {
            p.read_i64()?;
        }
        TType::Double => {
            p.read_double()?;
        }
        TType::Float => {
            p.read_float()?;
        }
        TType::Struct => {
            p.read_struct_begin(|_| ())?;
            loop {
                let fields = &[];
                let (_, type_id, _) = p.read_field_begin(|_| (), fields)?;
                if type_id == TType::Stop {
                    break;
                }
                skip_inner(p, type_id, max_depth - 1)?;
                p.read_field_end()?;
            }
            p.read_struct_end()?;
        }
        TType::Map => {
            let (key_type, value_type, len) = p.read_map_begin()?;
            if len != Some(0) {
                let mut idx = 0;
                loop {
                    let more = p.read_map_key_begin()?;
                    if !more {
                        break;
                    }
                    skip_inner(p, key_type, max_depth - 1)?;
                    p.read_map_value_begin()?;
                    skip_inner(p, value_type, max_depth)?;
                    p.read_map_value_end()?;

                    idx += 1;
                    if should_break(len, more, idx) {
                        break;
                    }
                }
            }
            p.read_map_end()?;
        }
        TType::Set => {
            let (elem_type, len) = p.read_set_begin()?;
            if len != Some(0) {
                let mut idx = 0;
                loop {
                    let more = p.read_set_value_begin()?;
                    if !more {
                        break;
                    }
                    skip_inner(p, elem_type, max_depth - 1)?;
                    p.read_set_value_end()?;

                    idx += 1;
                    if should_break(len, more, idx) {
                        break;
                    }
                }
            }
            p.read_set_end()?;
        }
        TType::List => {
            let (elem_type, len) = p.read_list_begin()?;
            if len != Some(0) {
                let mut idx = 0;
                loop {
                    let more = p.read_list_value_begin()?;
                    if !more {
                        break;
                    }
                    skip_inner(p, elem_type, max_depth - 1)?;
                    p.read_list_value_end()?;

                    idx += 1;
                    if should_break(len, more, idx) {
                        break;
                    }
                }
            }
            p.read_list_end()?;
        }
        TType::UTF8 => {
            p.read_string()?;
        }
        TType::UTF16 => {
            p.read_string()?;
        }
        TType::String => {
            p.read_binary::<Discard>()?;
        }
        TType::Stream => bail_err!(ProtocolError::StreamUnsupported),
        TType::Stop => bail_err!(ProtocolError::UnexpectedStopInSkip),
    };
    Ok(())
}

/// Trait for emitting a frame formatted in a given protocol
pub trait ProtocolWriter {
    type Final;

    fn write_message_begin(&mut self, name: &str, type_id: MessageType, seqid: u32);
    fn write_message_end(&mut self);
    fn write_struct_begin(&mut self, name: &str);
    fn write_struct_end(&mut self);
    fn write_field_begin(&mut self, name: &str, type_id: TType, id: i16);
    fn write_field_end(&mut self);
    fn write_field_stop(&mut self);
    fn write_map_begin(&mut self, key_type: TType, value_type: TType, size: usize);
    fn write_map_key_begin(&mut self);
    fn write_map_value_begin(&mut self);
    fn write_map_end(&mut self);
    fn write_list_begin(&mut self, elem_type: TType, size: usize);
    fn write_list_value_begin(&mut self);
    fn write_list_end(&mut self);
    fn write_set_begin(&mut self, elem_type: TType, size: usize);
    fn write_set_value_begin(&mut self);
    fn write_set_end(&mut self);
    fn write_bool(&mut self, value: bool);
    fn write_byte(&mut self, value: i8);
    fn write_i16(&mut self, value: i16);
    fn write_i32(&mut self, value: i32);
    fn write_i64(&mut self, value: i64);
    fn write_double(&mut self, value: f64);
    fn write_float(&mut self, value: f32);
    fn write_string(&mut self, value: &str);
    fn write_binary(&mut self, value: &[u8]);

    fn finish(self) -> Self::Final;
}

/// Trait for decoding a frame in a given protocol
pub trait ProtocolReader {
    fn read_message_begin<F, T>(&mut self, method: F) -> Result<(T, MessageType, u32)>
    where
        F: FnOnce(&[u8]) -> T;
    fn read_message_end(&mut self) -> Result<()>;
    fn read_struct_begin<F, T>(&mut self, strukt: F) -> Result<T>
    where
        F: FnOnce(&[u8]) -> T;
    fn read_struct_end(&mut self) -> Result<()>;
    fn read_field_begin<F, T>(&mut self, field: F, fields: &[Field]) -> Result<(T, TType, i16)>
    where
        F: FnOnce(&[u8]) -> T;
    fn read_field_end(&mut self) -> Result<()>;
    fn read_map_begin(&mut self) -> Result<(TType, TType, Option<usize>)>;
    fn read_map_key_begin(&mut self) -> Result<bool>;
    fn read_map_value_begin(&mut self) -> Result<()>;
    fn read_map_value_end(&mut self) -> Result<()>;
    fn read_map_end(&mut self) -> Result<()>;
    fn read_list_begin(&mut self) -> Result<(TType, Option<usize>)>;
    fn read_list_value_begin(&mut self) -> Result<bool>;
    fn read_list_value_end(&mut self) -> Result<()>;
    fn read_list_end(&mut self) -> Result<()>;
    fn read_set_begin(&mut self) -> Result<(TType, Option<usize>)>;
    fn read_set_value_begin(&mut self) -> Result<bool>;
    fn read_set_value_end(&mut self) -> Result<()>;
    fn read_set_end(&mut self) -> Result<()>;
    fn read_bool(&mut self) -> Result<bool>;
    fn read_byte(&mut self) -> Result<i8>;
    fn read_i16(&mut self) -> Result<i16>;
    fn read_i32(&mut self) -> Result<i32>;
    fn read_i64(&mut self) -> Result<i64>;
    fn read_double(&mut self) -> Result<f64>;
    fn read_float(&mut self) -> Result<f32>;
    fn read_string(&mut self) -> Result<String>;
    fn read_binary<V: CopyFromBuf>(&mut self) -> Result<V>;

    /// Skip over the next data element from the provided input Protocol object
    fn skip(&mut self, field_type: TType) -> Result<()> {
        skip_inner(self, field_type, DEFAULT_RECURSION_DEPTH)
    }
}

pub fn should_break(len: Option<usize>, more: bool, idx: usize) -> bool {
    match (len, more) {
        (Some(real_length), _) => idx >= real_length,
        (None, true) => false,
        (None, false) => true,
    }
}

/// Protocol::serializer wants the same serializer closure passed twice so
/// that it can specialize it for two types: once to compute the size, and a second time to
/// actually serialize the content. This macro helps by taking the factory type and
/// applying the serializer expression twice.
#[macro_export]
macro_rules! serialize {
    ($protocol:ty, $serializer:expr) => {
        <$protocol as $crate::Protocol>::serializer($serializer, $serializer)
    };
}

pub struct Field {
    pub name: &'static str,
    pub ttype: TType,
    pub id: i16,
}

impl Field {
    pub const fn new(name: &'static str, ttype: TType, id: i16) -> Self {
        Field { name, ttype, id }
    }
}
