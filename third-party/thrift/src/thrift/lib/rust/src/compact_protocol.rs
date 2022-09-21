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

use std::io::Cursor;

use anyhow::anyhow;
use anyhow::Result;
use bufsize::SizeCounter;
use bytes::Bytes;
use bytes::BytesMut;
use ghost::phantom;

use crate::binary_type::CopyFromBuf;
use crate::bufext::BufExt;
use crate::bufext::BufMutExt;
use crate::bufext::DeserializeSource;
use crate::deserialize::Deserialize;
use crate::errors::ProtocolError;
use crate::framing::Framing;
use crate::protocol::Field;
use crate::protocol::Protocol;
use crate::protocol::ProtocolReader;
use crate::protocol::ProtocolWriter;
use crate::serialize::Serialize;
use crate::thrift_protocol::MessageType;
use crate::thrift_protocol::ProtocolID;
use crate::ttype::TType;
use crate::varint;

const COMPACT_PROTOCOL_VERSION: u8 = 0x02;
const PROTOCOL_ID: u8 = 0x82;
const TYPE_MASK: u8 = 0xE0;
const TYPE_SHIFT_AMOUNT: usize = 5;
const VERSION_MASK: u8 = 0x1f;

#[derive(Copy, Clone, Eq, PartialEq, Hash, Debug)]
#[repr(u8)]
pub enum CType {
    Stop = 0,
    BoolTrue = 1,
    BoolFalse = 2,
    Byte = 3,
    I16 = 4,
    I32 = 5,
    I64 = 6,
    Double = 7,
    Binary = 8,
    List = 9,
    Set = 10,
    Map = 11,
    Struct = 12,
    Float = 13,
}

impl TryFrom<i8> for CType {
    type Error = anyhow::Error;

    fn try_from(v: i8) -> Result<Self> {
        let ret = match v {
            0 => CType::Stop,
            1 => CType::BoolTrue,
            2 => CType::BoolFalse,
            3 => CType::Byte,
            4 => CType::I16,
            5 => CType::I32,
            6 => CType::I64,
            7 => CType::Double,
            8 => CType::Binary,
            9 => CType::List,
            10 => CType::Set,
            11 => CType::Map,
            12 => CType::Struct,
            13 => CType::Float,
            _ => bail_err!(ProtocolError::InvalidTypeTag),
        };
        Ok(ret)
    }
}

impl From<CType> for TType {
    fn from(cty: CType) -> TType {
        match cty {
            CType::Stop => TType::Stop,
            CType::BoolFalse | CType::BoolTrue => TType::Bool,
            CType::Byte => TType::Byte,
            CType::Double => TType::Double,
            CType::I16 => TType::I16,
            CType::I32 => TType::I32,
            CType::I64 => TType::I64,
            CType::Binary => TType::String,
            CType::Struct => TType::Struct,
            CType::Map => TType::Map,
            CType::Set => TType::Set,
            CType::List => TType::List,
            CType::Float => TType::Float,
        }
    }
}

impl From<TType> for CType {
    fn from(tty: TType) -> CType {
        match tty {
            TType::Stop => CType::Stop,
            TType::Bool => CType::BoolTrue,
            TType::Byte => CType::Byte,
            TType::Double => CType::Double,
            TType::I16 => CType::I16,
            TType::I32 => CType::I32,
            TType::I64 => CType::I64,
            TType::String => CType::Binary,
            TType::Struct => CType::Struct,
            TType::Map => CType::Map,
            TType::Set => CType::Set,
            TType::List => CType::List,
            TType::Float => CType::Float,
            bad => panic!("Don't know how to convert TType {:?} to CType", bad),
        }
    }
}

impl From<bool> for CType {
    fn from(b: bool) -> CType {
        if b { CType::BoolTrue } else { CType::BoolFalse }
    }
}

/// An efficient, dense encoding that uses variable-length integers and squeezes
/// data into unused bits wherever possible.
///
/// ```ignore
/// let protocol = CompactProtocol;
/// let transport = HttpClient::new(ENDPOINT)?;
/// let client = <dyn BuckGraphService>::new(protocol, transport);
/// ```
///
/// The type parameter is the Framing expected by the transport on which this
/// protocol is operating. Usually by convention the transport itself serves as
/// the Framing impl, so for example in the case of HttpClient above, the
/// compiler has inferred `F = HttpClient`.
///
/// Where the compiler reports that a Framing can't be inferred, one can be
/// specified explicitly:
///
/// ```ignore
/// let protocol = CompactProtocol::<SRHeaderTransport>;
/// ```
#[phantom]
#[derive(Copy, Clone)]
pub struct CompactProtocol<F = Bytes>;

#[derive(Debug, Clone)]
struct EncState {
    idxstack: Vec<i16>,
    lastidx: i16,
    field: Option<(TType, i16)>,
}

impl EncState {
    fn new() -> Self {
        EncState {
            idxstack: Vec::new(),
            lastidx: 0,
            field: None,
        }
    }

    fn struct_begin(&mut self) {
        self.idxstack.push(self.lastidx);
        self.lastidx = 0;
    }

    fn struct_end(&mut self) {
        self.lastidx = self.idxstack.pop().expect("struct stack underrun");
    }

    fn field_begin(&mut self, tty: TType, idx: i16) {
        self.field = Some((tty, idx));
    }

    fn field_get(&mut self) -> Option<(TType, i16)> {
        self.field.take()
    }

    fn field_end(&mut self) {
        debug_assert!(
            // the field was consumed during the write, or
            self.field.is_none() ||
            // the field contains a void which wasn't written
            matches!(self.field, Some((TType::Void, _)))
        )
    }

    fn in_field(&self) -> bool {
        self.field.is_some()
    }
}

pub struct CompactProtocolSerializer<B> {
    state: EncState,
    buffer: B,
    string_limit: Option<usize>,
    container_limit: Option<usize>,
}

pub struct CompactProtocolDeserializer<B> {
    state: EncState,
    boolfield: Option<CType>,
    buffer: B,
    string_limit: Option<usize>,
    container_limit: Option<usize>,
}

impl<F> Protocol for CompactProtocol<F>
where
    F: Framing + 'static,
{
    type Frame = F;
    type Sizer = CompactProtocolSerializer<SizeCounter>;
    type Serializer = CompactProtocolSerializer<F::EncBuf>;
    type Deserializer = CompactProtocolDeserializer<F::DecBuf>;

    const PROTOCOL_ID: ProtocolID = ProtocolID::CompactProtocol;

    fn serializer<SZ, SER>(size: SZ, ser: SER) -> <Self::Serializer as ProtocolWriter>::Final
    where
        SZ: FnOnce(&mut Self::Sizer),
        SER: FnOnce(&mut Self::Serializer),
    {
        // Instantiate a serializer with SizeCounter to count up the output buffer size
        let mut sizer = CompactProtocolSerializer {
            state: EncState::new(),
            buffer: SizeCounter::new(),
            container_limit: None,
            string_limit: None,
        };
        size(&mut sizer);
        let sz = sizer.finish();

        // Now that we have the size, allocate an output buffer and serialize into it
        let mut buf = CompactProtocolSerializer {
            state: EncState::new(),
            buffer: F::enc_with_capacity(sz),
            container_limit: None,
            string_limit: None,
        };
        ser(&mut buf);

        // Done
        buf.finish()
    }

    fn deserializer(buf: F::DecBuf) -> Self::Deserializer {
        CompactProtocolDeserializer::new(buf)
    }

    fn into_buffer(deser: Self::Deserializer) -> F::DecBuf {
        deser.into_inner()
    }
}

impl<B> CompactProtocolSerializer<B>
where
    B: BufMutExt,
{
    pub fn with_buffer(buffer: B) -> Self {
        Self {
            state: EncState::new(),
            buffer,
            string_limit: None,
            container_limit: None,
        }
    }

    pub fn into_inner(self) -> B {
        self.buffer
    }

    #[inline]
    fn write_varint_i64(&mut self, v: i64) {
        self.buffer.put_varint_i64(v)
    }

    #[inline]
    fn write_varint_i32(&mut self, v: i32) {
        self.write_varint_i64(v as i64);
    }

    #[inline]
    fn write_varint_i16(&mut self, v: i16) {
        self.write_varint_i64(v as i64);
    }

    /// If we've just got a write_field_begin, then this will emit an appropriate field begin
    /// record to the wire. If we haven't then it's a no-op. This is used to defer emitting
    /// the field header until we get the field value, mostly to handle the strangeness of bool.
    fn write_field_id(&mut self, cty: CType) {
        match self.state.field_get() {
            None => {}
            Some((tty, idx)) => {
                debug_assert_eq!(tty, TType::from(cty));

                let delta = idx - self.state.lastidx;
                self.state.lastidx = idx;

                if delta <= 0 || delta > 15 {
                    self.buffer.put_u8(cty as u8);
                    self.write_varint_i16(idx);
                } else {
                    self.buffer.put_u8(((delta as u8) << 4) | (cty as u8));
                }
            }
        }
    }

    /// Common code for writing the start of a sequence of elements, used for lists and sets
    fn write_sequence(&mut self, elem_type: TType, size: usize) {
        assert!(
            self.container_limit.map_or(true, |lim| size < lim),
            "container too large {}, lim {:?}",
            size,
            self.container_limit
        );

        let cty = CType::from(elem_type) as u8;

        // Size = 0x0f is used as the marker for a separate varint size
        if size < 0x0f {
            self.buffer.put_u8((size as u8) << 4 | cty);
        } else {
            self.buffer.put_u8((0x0f << 4) | cty);
            self.buffer.put_varint_u64(size as u64);
        }
    }
}

impl<B: BufMutExt> ProtocolWriter for CompactProtocolSerializer<B> {
    type Final = B::Final; // Our final form is whatever the buffer produces

    fn write_message_begin(&mut self, name: &str, msgtype: MessageType, seqid: u32) {
        let msgtype = msgtype as u8;
        self.buffer.put_u8(PROTOCOL_ID);
        self.buffer
            .put_u8(COMPACT_PROTOCOL_VERSION | ((msgtype << TYPE_SHIFT_AMOUNT) & TYPE_MASK));
        self.buffer.put_varint_u64(seqid as u64);
        self.write_string(name);
    }

    #[inline]
    fn write_message_end(&mut self) {}

    #[inline]
    fn write_struct_begin(&mut self, _name: &str) {
        self.write_field_id(CType::Struct);
        self.state.struct_begin();
    }

    #[inline]
    fn write_struct_end(&mut self) {
        self.state.struct_end();
    }

    #[inline]
    fn write_field_begin(&mut self, _name: &str, type_id: TType, id: i16) {
        // Note the field, but we'll emit it when the value comes
        self.state.field_begin(type_id, id);
    }

    #[inline]
    fn write_field_end(&mut self) {
        self.state.field_end()
    }

    #[inline]
    fn write_field_stop(&mut self) {
        self.buffer.put_u8(CType::Stop as u8)
    }

    fn write_map_begin(&mut self, key_type: TType, value_type: TType, size: usize) {
        assert!(
            self.container_limit.map_or(true, |lim| size < lim),
            "map too large {}, lim {:?}",
            size,
            self.container_limit
        );

        self.write_field_id(CType::Map);
        self.buffer.put_varint_u64(size as u64);
        if size > 0 {
            let ckty = CType::from(key_type);
            let cvty = CType::from(value_type);

            self.buffer.put_u8((ckty as u8) << 4 | (cvty as u8));
        }
    }

    #[inline]
    fn write_map_key_begin(&mut self) {}

    #[inline]
    fn write_map_value_begin(&mut self) {}

    #[inline]
    fn write_map_end(&mut self) {}

    fn write_list_begin(&mut self, elem_type: TType, size: usize) {
        assert!(self.container_limit.map_or(true, |lim| size < lim));
        self.write_field_id(CType::List);
        self.write_sequence(elem_type, size);
    }

    #[inline]
    fn write_list_value_begin(&mut self) {}

    #[inline]
    fn write_list_end(&mut self) {}

    fn write_set_begin(&mut self, elem_type: TType, size: usize) {
        self.write_field_id(CType::Set);
        self.write_sequence(elem_type, size);
    }

    #[inline]
    fn write_set_value_begin(&mut self) {}

    fn write_set_end(&mut self) {}

    fn write_bool(&mut self, value: bool) {
        // If we're in a field then we need to encode the bool value as the field's type.
        // Otherwise we just emit it as a byte, but still using the field type values as
        // the encoding.
        if self.state.in_field() {
            self.write_field_id(CType::from(value))
        } else if value {
            self.write_byte(CType::BoolTrue as i8)
        } else {
            self.write_byte(CType::BoolFalse as i8)
        }
    }

    fn write_byte(&mut self, value: i8) {
        self.write_field_id(CType::Byte);
        self.buffer.put_i8(value)
    }

    fn write_i16(&mut self, value: i16) {
        self.write_field_id(CType::I16);
        self.write_varint_i16(value)
    }

    fn write_i32(&mut self, value: i32) {
        self.write_field_id(CType::I32);
        self.write_varint_i32(value)
    }

    fn write_i64(&mut self, value: i64) {
        self.write_field_id(CType::I64);
        self.write_varint_i64(value)
    }

    fn write_double(&mut self, value: f64) {
        self.write_field_id(CType::Double);
        self.buffer.put_f64(value)
    }

    fn write_float(&mut self, value: f32) {
        self.write_field_id(CType::Float);
        self.buffer.put_f32(value)
    }

    #[inline]
    fn write_string(&mut self, value: &str) {
        self.write_binary(value.as_bytes());
    }

    fn write_binary(&mut self, value: &[u8]) {
        let size = value.len();
        assert!(
            self.string_limit.map_or(true, |lim| size < lim),
            "string too large {}, lim {:?}",
            size,
            self.string_limit
        );

        self.write_field_id(CType::Binary);
        self.buffer.put_varint_u64(size as u64);
        self.buffer.put_slice(value)
    }

    fn finish(self) -> B::Final {
        self.buffer.finalize()
    }
}

impl<B: BufExt> CompactProtocolDeserializer<B> {
    pub fn new(buffer: B) -> Self {
        CompactProtocolDeserializer {
            state: EncState::new(),
            boolfield: None,
            buffer,
            string_limit: None,
            container_limit: None,
        }
    }

    pub fn into_inner(self) -> B {
        self.buffer
    }

    fn peek_bytes(&self, len: usize) -> Option<&[u8]> {
        if self.buffer.chunk().len() >= len {
            Some(&self.buffer.chunk()[..len])
        } else {
            None
        }
    }

    fn read_varint_u64(&mut self) -> Result<u64> {
        varint::read_u64(&mut self.buffer)
    }

    fn read_varint_i64(&mut self) -> Result<i64> {
        self.read_varint_u64().map(varint::unzigzag)
    }

    fn read_varint_i32(&mut self) -> Result<i32> {
        self.read_varint_i64()
            .and_then(|v| i32::try_from(v).map_err(|_| ProtocolError::InvalidValue.into()))
    }

    fn read_varint_i16(&mut self) -> Result<i16> {
        self.read_varint_i64()
            .and_then(|v| i16::try_from(v).map_err(|_| (ProtocolError::InvalidValue).into()))
    }
}

impl<B: BufExt> ProtocolReader for CompactProtocolDeserializer<B> {
    fn read_message_begin<F, T>(&mut self, msgfn: F) -> Result<(T, MessageType, u32)>
    where
        F: FnOnce(&[u8]) -> T,
    {
        let protocolid = self.read_byte()? as u8;
        ensure_err!(protocolid == PROTOCOL_ID, ProtocolError::BadVersion);

        let vandty = self.read_byte()? as u8;
        ensure_err!(
            (vandty & VERSION_MASK) == COMPACT_PROTOCOL_VERSION,
            ProtocolError::BadVersion
        );

        let msgty = (vandty & TYPE_MASK) >> TYPE_SHIFT_AMOUNT;
        let msgty = MessageType::try_from(msgty as u32)?;

        let seqid = self.read_varint_u64()? as u32;

        let name = {
            let len = self.read_varint_u64()? as usize;
            let (len, name) = {
                if self.peek_bytes(len).is_some() {
                    let namebuf = self.peek_bytes(len).unwrap();
                    (namebuf.len(), msgfn(namebuf))
                } else {
                    ensure_err!(
                        self.buffer.remaining() >= len,
                        ProtocolError::InvalidDataLength
                    );
                    let namebuf: Vec<u8> = Vec::copy_from_buf(&mut self.buffer, len);
                    (0, msgfn(namebuf.as_slice()))
                }
            };
            self.buffer.advance(len);
            name
        };

        Ok((name, msgty, seqid))
    }

    fn read_message_end(&mut self) -> Result<()> {
        Ok(())
    }

    fn read_struct_begin<F, T>(&mut self, namefn: F) -> Result<T>
    where
        F: FnOnce(&[u8]) -> T,
    {
        self.state.struct_begin();
        Ok(namefn(&[]))
    }

    fn read_struct_end(&mut self) -> Result<()> {
        self.state.struct_end();
        Ok(())
    }

    fn read_field_begin<F, T>(&mut self, fieldfn: F, _fields: &[Field]) -> Result<(T, TType, i16)>
    where
        F: FnOnce(&[u8]) -> T,
    {
        let tyid = self.read_byte()? as i8;
        let cty = CType::try_from(tyid & 0x0f)?;
        let didx = (tyid >> 4) & 0x0f;

        let tty = TType::from(cty);

        // Check if we're stopping, or get the next field id
        let idx = match (tty, didx) {
            (TType::Stop, _) => 0,
            (_, 0) => self.read_varint_i16()?,
            (_, didx) => self.state.lastidx + (didx as i16),
        };

        self.state.lastidx = idx;

        if tty == TType::Bool {
            self.boolfield = Some(cty);
        }

        let f = fieldfn(&[]);
        Ok((f, tty, idx))
    }

    fn read_field_end(&mut self) -> Result<()> {
        Ok(())
    }

    fn read_map_begin(&mut self) -> Result<(TType, TType, Option<usize>)> {
        let size = self.read_varint_u64()? as usize;

        ensure_err!(
            self.container_limit.map_or(true, |lim| size < lim),
            ProtocolError::InvalidDataLength
        );

        // If the size is 0 we treat the kv type byte as 0, which ends up being (Stop, Stop)
        let kvtype = if size > 0 { self.read_byte()? } else { 0 };

        let kcty = CType::try_from((kvtype >> 4) & 0x0f)?;
        let vcty = CType::try_from((kvtype) & 0x0f)?;

        Ok((TType::from(kcty), TType::from(vcty), Some(size as usize)))
    }

    #[inline]
    fn read_map_key_begin(&mut self) -> Result<bool> {
        Ok(true)
    }

    #[inline]
    fn read_map_value_begin(&mut self) -> Result<()> {
        Ok(())
    }

    #[inline]
    fn read_map_value_end(&mut self) -> Result<()> {
        Ok(())
    }

    fn read_map_end(&mut self) -> Result<()> {
        Ok(())
    }

    fn read_list_begin(&mut self) -> Result<(TType, Option<usize>)> {
        let szty = self.read_byte()?;
        let cty = CType::try_from(szty & 0x0f)?;
        let elem_type = TType::from(cty);

        let size = match (szty >> 4) & 0x0f {
            0x0f => self.read_varint_u64()? as usize,
            sz => sz as usize,
        };

        ensure_err!(
            self.container_limit.map_or(true, |lim| size < lim),
            ProtocolError::InvalidDataLength
        );

        Ok((elem_type, Some(size)))
    }

    #[inline]
    fn read_list_value_begin(&mut self) -> Result<bool> {
        Ok(true)
    }

    #[inline]
    fn read_list_value_end(&mut self) -> Result<()> {
        Ok(())
    }

    fn read_list_end(&mut self) -> Result<()> {
        Ok(())
    }

    fn read_set_begin(&mut self) -> Result<(TType, Option<usize>)> {
        self.read_list_begin()
    }

    #[inline]
    fn read_set_value_begin(&mut self) -> Result<bool> {
        Ok(true)
    }

    #[inline]
    fn read_set_value_end(&mut self) -> Result<()> {
        Ok(())
    }

    fn read_set_end(&mut self) -> Result<()> {
        Ok(())
    }

    fn read_bool(&mut self) -> Result<bool> {
        // If we're in a bool field then take the value from that, otherwise
        // read it as a byte from the wire.
        let cty = match self.boolfield.take() {
            None => CType::try_from(self.read_byte()?)?,
            Some(cty) => cty,
        };

        // Match cpp2 - not strict on value of false. This is supposed to be encoded as a
        // CType::BoolTrue/BoolFalse, which is 1/2 - but we just check for true == 1. I suspect
        // this is to allow false to be encoded as 0 (or anything other than 1).
        Ok(cty == CType::BoolTrue)
    }

    fn read_byte(&mut self) -> Result<i8> {
        ensure_err!(self.buffer.remaining() >= 1, ProtocolError::EOF);

        Ok(self.buffer.get_i8())
    }

    fn read_i16(&mut self) -> Result<i16> {
        self.read_varint_i16()
    }

    fn read_i32(&mut self) -> Result<i32> {
        self.read_varint_i32()
    }

    fn read_i64(&mut self) -> Result<i64> {
        self.read_varint_i64()
    }

    fn read_double(&mut self) -> Result<f64> {
        ensure_err!(self.buffer.remaining() >= 8, ProtocolError::EOF);

        Ok(self.buffer.get_f64())
    }

    fn read_float(&mut self) -> Result<f32> {
        ensure_err!(self.buffer.remaining() >= 4, ProtocolError::EOF);

        Ok(self.buffer.get_f32())
    }

    fn read_string(&mut self) -> Result<String> {
        let vec = self.read_binary::<Vec<u8>>()?;

        String::from_utf8(vec)
            .map_err(|utf8_error| anyhow!("deserializing `string` from Thrift compact protocol got invalid utf-8, you need to use `binary` instead: {utf8_error}"))
    }

    fn read_binary<V: CopyFromBuf>(&mut self) -> Result<V> {
        let received_len = self.read_varint_u64()? as usize;
        ensure_err!(
            self.string_limit.map_or(true, |lim| received_len < lim),
            ProtocolError::InvalidDataLength
        );
        ensure_err!(self.buffer.remaining() >= received_len, ProtocolError::EOF);

        Ok(V::copy_from_buf(&mut self.buffer, received_len))
    }
}

/// How large an item will be when `serialize()` is called
pub fn serialize_size<T>(v: &T) -> usize
where
    T: Serialize<CompactProtocolSerializer<SizeCounter>>,
{
    let mut sizer = CompactProtocolSerializer {
        state: EncState::new(),
        buffer: SizeCounter::new(),
        container_limit: None,
        string_limit: None,
    };
    v.write(&mut sizer);
    sizer.finish()
}

/// Serialize a Thrift value using the compact protocol to a pre-allocated buffer.
/// This will panic if the buffer is not large enough. A buffer at least as
/// large as the return value of `serialize_size` will not panic.
pub fn serialize_to_buffer<T>(v: T, buffer: BytesMut) -> CompactProtocolSerializer<BytesMut>
where
    T: Serialize<CompactProtocolSerializer<BytesMut>>,
{
    // Now that we have the size, allocate an output buffer and serialize into it
    let mut buf = CompactProtocolSerializer {
        state: EncState::new(),
        buffer,
        container_limit: None,
        string_limit: None,
    };
    v.write(&mut buf);
    buf
}

pub trait SerializeRef:
    Serialize<CompactProtocolSerializer<SizeCounter>> + Serialize<CompactProtocolSerializer<BytesMut>>
where
    for<'a> &'a Self: Serialize<CompactProtocolSerializer<SizeCounter>>,
    for<'a> &'a Self: Serialize<CompactProtocolSerializer<BytesMut>>,
{
}

impl<T> SerializeRef for T
where
    T: Serialize<CompactProtocolSerializer<BytesMut>>,
    T: Serialize<CompactProtocolSerializer<SizeCounter>>,
    for<'a> &'a T: Serialize<CompactProtocolSerializer<BytesMut>>,
    for<'a> &'a T: Serialize<CompactProtocolSerializer<SizeCounter>>,
{
}

/// Serialize a Thrift value using the compact protocol.
pub fn serialize<T>(v: T) -> Bytes
where
    T: Serialize<CompactProtocolSerializer<SizeCounter>>
        + Serialize<CompactProtocolSerializer<BytesMut>>,
{
    let sz = serialize_size(&v);
    let buf = serialize_to_buffer(v, BytesMut::with_capacity(sz));
    // Done
    buf.finish()
}

pub trait DeserializeSlice:
    for<'a> Deserialize<CompactProtocolDeserializer<Cursor<&'a [u8]>>>
{
}

impl<T> DeserializeSlice for T where
    T: for<'a> Deserialize<CompactProtocolDeserializer<Cursor<&'a [u8]>>>
{
}

/// Deserialize a Thrift blob using the compact protocol.
pub fn deserialize<T, B, C>(b: B) -> Result<T>
where
    B: Into<DeserializeSource<C>>,
    C: BufExt,
    T: Deserialize<CompactProtocolDeserializer<C>>,
{
    let source: DeserializeSource<C> = b.into();
    let mut deser = CompactProtocolDeserializer::new(source.0);
    Ok(T::read(&mut deser)?)
}
