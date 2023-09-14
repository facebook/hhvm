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
use anyhow::bail;
use anyhow::Context;
use anyhow::Result;
use bufsize::SizeCounter;
use bytes::buf::Writer;
use bytes::Buf;
use bytes::BufMut;
use bytes::Bytes;
use bytes::BytesMut;
use ghost::phantom;
use serde_json::ser::CompactFormatter;
use serde_json::ser::Formatter;

use crate::binary_type::CopyFromBuf;
use crate::bufext::BufExt;
use crate::bufext::BufMutExt;
use crate::bufext::DeserializeSource;
use crate::deserialize::Deserialize;
use crate::errors::ProtocolError;
use crate::framing::Framing;
use crate::protocol::should_break;
use crate::protocol::Field;
use crate::protocol::Protocol;
use crate::protocol::ProtocolReader;
use crate::protocol::ProtocolWriter;
use crate::protocol::DEFAULT_RECURSION_DEPTH;
use crate::serialize::Serialize;
use crate::thrift_protocol::MessageType;
use crate::thrift_protocol::ProtocolID;
use crate::ttype::TType;

#[phantom]
#[derive(Copy, Clone)]
pub struct SimpleJsonProtocol<F = Bytes>;

pub struct SimpleJsonProtocolSerializer<B: BufMutExt> {
    buffer: Writer<B>,
    state: Vec<SerializationState>,
}

pub struct SimpleJsonProtocolDeserializer<B> {
    buffer: B,
    remaining: usize,
}

impl<F> Protocol for SimpleJsonProtocol<F>
where
    F: Framing + 'static,
{
    type Frame = F;
    type Sizer = SimpleJsonProtocolSerializer<SizeCounter>;
    type Serializer = SimpleJsonProtocolSerializer<F::EncBuf>;
    type Deserializer = SimpleJsonProtocolDeserializer<F::DecBuf>;

    const PROTOCOL_ID: ProtocolID = ProtocolID::SimpleJSONProtocol;

    fn serializer<SZ, SER>(size: SZ, ser: SER) -> <Self::Serializer as ProtocolWriter>::Final
    where
        SZ: FnOnce(&mut Self::Sizer),
        SER: FnOnce(&mut Self::Serializer),
    {
        // Instantiate a serializer with SizeCounter to count up the output buffer size
        let mut sizer = SimpleJsonProtocolSerializer {
            buffer: SizeCounter::new().writer(),
            state: vec![SerializationState::NotInContainer],
        };
        size(&mut sizer);
        let sz = sizer.finish();

        // Now that we have the size, allocate an output buffer and serialize into it
        let mut buf = SimpleJsonProtocolSerializer {
            buffer: F::enc_with_capacity(sz).writer(),
            state: vec![SerializationState::NotInContainer],
        };
        ser(&mut buf);

        // Done
        buf.finish()
    }

    fn deserializer(buf: F::DecBuf) -> Self::Deserializer {
        SimpleJsonProtocolDeserializer::new(buf)
    }

    fn into_buffer(deser: Self::Deserializer) -> F::DecBuf {
        deser.into_inner()
    }
}

impl<B: BufMutExt> SimpleJsonProtocolSerializer<B> {
    fn possibly_write_comma(&mut self) {
        match self
            .state
            .last()
            .expect("Invariant of encoding state violated")
        {
            SerializationState::JustEnteredContainer => {
                CompactFormatter
                    .begin_object_key(&mut self.buffer, true)
                    .expect("Somehow failed to do \"io\" on a buffer");
                *self
                    .state
                    .last_mut()
                    .expect("Invariant of encoding state violated") =
                    SerializationState::InContainerValue;
            }
            SerializationState::InContainerKey
            | SerializationState::InContainerValue
            | SerializationState::NotInContainer => CompactFormatter
                .begin_object_key(&mut self.buffer, false)
                .expect("Somehow failed to do \"io\" on a buffer"),
        }
    }

    fn writing_key(&mut self) -> bool {
        match self
            .state
            .last()
            .expect("Invariant of encoding state violated")
        {
            SerializationState::InContainerKey => true,
            _ => false,
        }
    }
}

#[derive(Debug)]
enum SerializationState {
    JustEnteredContainer,
    InContainerKey,
    InContainerValue,
    NotInContainer,
}

impl<B: BufMutExt> ProtocolWriter for SimpleJsonProtocolSerializer<B> {
    type Final = B::Final; // Our final form is whatever the buffer produces

    // TODO what to do here
    fn write_message_begin(&mut self, name: &str, msgtype: MessageType, seqid: u32) {
        self.write_list_begin(TType::Stop, 0);
        self.write_list_value_begin();
        self.write_string(name);
        self.write_list_value_begin();
        self.write_byte(msgtype as i8);
        self.write_list_value_begin();
        self.write_i32(seqid as i32);
    }

    #[inline]
    fn write_message_end(&mut self) {
        self.write_list_end();
    }

    #[inline]
    fn write_struct_begin(&mut self, _name: &str) {
        CompactFormatter
            .begin_object(&mut self.buffer)
            .expect("Somehow failed to do \"io\" on a buffer");
        self.state.push(SerializationState::JustEnteredContainer);
    }

    #[inline]
    fn write_struct_end(&mut self) {
        CompactFormatter
            .end_object(&mut self.buffer)
            .expect("Somehow failed to do \"io\" on a buffer");
        self.state.pop();
    }

    #[inline]
    fn write_field_begin(&mut self, name: &str, _type_id: TType, _id: i16) {
        self.possibly_write_comma();
        self.write_string(name);
        CompactFormatter
            .end_object_key(&mut self.buffer)
            .expect("Somehow failed to do \"io\" on a buffer");
        CompactFormatter
            .begin_object_value(&mut self.buffer)
            .expect("Somehow failed to do \"io\" on a buffer");
    }

    #[inline]
    fn write_field_end(&mut self) {}

    #[inline]
    fn write_field_stop(&mut self) {}

    fn write_map_begin(&mut self, _key_type: TType, _value_type: TType, _size: usize) {
        self.state.push(SerializationState::JustEnteredContainer);
        CompactFormatter
            .begin_object(&mut self.buffer)
            .expect("Somehow failed to do \"io\" on a buffer");
    }

    fn write_map_key_begin(&mut self) {
        self.possibly_write_comma();
        *self
            .state
            .last_mut()
            .expect("Invariant of encoding state violated") = SerializationState::InContainerKey;
    }

    fn write_map_value_begin(&mut self) {
        CompactFormatter
            .end_object_key(&mut self.buffer)
            .expect("Somehow failed to do \"io\" on a buffer");
        CompactFormatter
            .begin_object_value(&mut self.buffer)
            .expect("Somehow failed to do \"io\" on a buffer");
        *self
            .state
            .last_mut()
            .expect("Invariant of encoding state violated") = SerializationState::InContainerValue;
    }

    #[inline]
    fn write_map_end(&mut self) {
        CompactFormatter
            .end_object(&mut self.buffer)
            .expect("Somehow failed to do \"io\" on a buffer");
        self.state.pop();
    }

    fn write_list_begin(&mut self, _elem_type: TType, _size: usize) {
        CompactFormatter
            .begin_array(&mut self.buffer)
            .expect("Somehow failed to do \"io\" on a buffer");
        self.state.push(SerializationState::JustEnteredContainer);
    }

    fn write_list_value_begin(&mut self) {
        self.possibly_write_comma();
    }

    #[inline]
    fn write_list_end(&mut self) {
        CompactFormatter
            .end_array(&mut self.buffer)
            .expect("Somehow failed to do \"io\" on a buffer");
        self.state.pop();
    }

    fn write_set_begin(&mut self, elem_type: TType, size: usize) {
        self.write_list_begin(elem_type, size);
    }

    fn write_set_value_begin(&mut self) {
        self.write_list_value_begin();
    }

    fn write_set_end(&mut self) {
        self.write_list_end();
    }

    fn write_bool(&mut self, value: bool) {
        CompactFormatter
            .write_bool(&mut self.buffer, value)
            .expect("Somehow failed to do \"io\" on a buffer");
    }

    fn write_byte(&mut self, value: i8) {
        if self.writing_key() {
            self.write_string(&value.to_string());
            return;
        }
        CompactFormatter
            .write_i8(&mut self.buffer, value)
            .expect("Somehow failed to do \"io\" on a buffer");
    }

    fn write_i16(&mut self, value: i16) {
        if self.writing_key() {
            self.write_string(&value.to_string());
            return;
        }
        CompactFormatter
            .write_i16(&mut self.buffer, value)
            .expect("Somehow failed to do \"io\" on a buffer");
    }

    fn write_i32(&mut self, value: i32) {
        if self.writing_key() {
            self.write_string(&value.to_string());
            return;
        }
        CompactFormatter
            .write_i32(&mut self.buffer, value)
            .expect("Somehow failed to do \"io\" on a buffer");
    }

    fn write_i64(&mut self, value: i64) {
        if self.writing_key() {
            self.write_string(&value.to_string());
            return;
        }
        CompactFormatter
            .write_i64(&mut self.buffer, value)
            .expect("Somehow failed to do \"io\" on a buffer");
    }

    fn write_double(&mut self, value: f64) {
        if self.writing_key() {
            self.write_string(&value.to_string());
            return;
        }
        CompactFormatter
            .write_f64(&mut self.buffer, value)
            .expect("Somehow failed to do \"io\" on a buffer");
    }

    fn write_float(&mut self, value: f32) {
        if self.writing_key() {
            self.write_string(&value.to_string());
            return;
        }
        CompactFormatter
            .write_f32(&mut self.buffer, value)
            .expect("Somehow failed to do \"io\" on a buffer");
    }

    fn write_string(&mut self, value: &str) {
        serde_json::to_writer(&mut self.buffer, value)
            .expect("Somehow failed to do \"io\" on a buffer");
    }

    fn write_binary(&mut self, value: &[u8]) {
        CompactFormatter
            .begin_string(&mut self.buffer)
            .expect("Somehow failed to do \"io\" on a buffer");
        CompactFormatter
            .write_raw_fragment(
                &mut self.buffer,
                &base64::encode_config(value, base64::STANDARD_NO_PAD),
            )
            .expect("Somehow failed to do \"io\" on a buffer");
        CompactFormatter
            .end_string(&mut self.buffer)
            .expect("Somehow failed to do \"io\" on a buffer");
    }

    fn finish(self) -> B::Final {
        self.buffer.into_inner().finalize()
    }
}

#[derive(Debug)]
enum CommaState {
    Trailing,
    NonTrailing,
    NoComma,
    End,
}

impl<B: Buf> SimpleJsonProtocolDeserializer<B> {
    pub fn new(buffer: B) -> Self {
        let remaining = buffer.remaining();
        SimpleJsonProtocolDeserializer { buffer, remaining }
    }

    pub fn into_inner(self) -> B {
        self.buffer
    }

    // TODO(azw): codify this (number of bytes left) in the Deserializer API
    // `bytes` (named `chunks` in bytes1.0) does not represent a contiguous slice
    // of the remaining bytes. All we can do is check if there is a byte to return
    /// Returns a byte from the underly buffer if there is enough remaining
    pub fn peek(&self) -> Option<u8> {
        // fast path like https://docs.rs/bytes/1.0.1/src/bytes/buf/buf_impl.rs.html#18
        if !self.buffer.chunk().is_empty() {
            Some(self.buffer.chunk()[0])
        } else {
            None
        }
    }

    /// Like peek but panics if there is none remaining
    fn peek_can_panic(&self) -> u8 {
        self.buffer.chunk()[0]
    }

    fn strip_whitespace(&mut self) {
        while let Some(b) = self.peek() {
            if !&[b' ', b'\t', b'\n', b'\r'].contains(&b) {
                break;
            }
            self.advance(1);
        }
    }
    // Validates that next chars is `val`
    fn eat_only(&mut self, val: &[u8]) -> Result<()> {
        if self.remaining < val.len() {
            bail!(
                "Expected the following chars: {:?}, not enough bytes remaining",
                String::from_utf8_lossy(val)
            )
        }

        for to_check in val {
            let b = self.peek_can_panic();
            if b != *to_check {
                // TODO(azw): better error messages
                bail!("Expected {} got {}", to_check, b)
            }
            self.advance(1);
        }
        Ok(())
    }

    // Validates that next chars is `val`. Strip whitespace before and after
    fn eat(&mut self, val: &[u8]) -> Result<()> {
        self.strip_whitespace();
        self.eat_only(val)?;
        self.strip_whitespace();
        Ok(())
    }

    fn advance(&mut self, len: usize) {
        self.buffer.advance(len);

        // advance will panic if we try to advance more than remaining
        self.remaining -= len
    }

    // Attempts to eat a comma, returns
    // Trailing if this a "trailing" comma, where trailing means its
    //   trailed by the `trailing` byte
    // NonTrailing if its read a comma that isn't trailing
    // NoComma if it found no comma in the middle of the container
    // End if it correctly found no comma at the end of the container
    fn possibly_read_comma(&mut self, trailing: u8) -> CommaState {
        match self.eat(b",") {
            Ok(()) => match self.peek() {
                Some(b) if b == trailing => CommaState::Trailing,
                _ => CommaState::NonTrailing,
            },
            _ => match self.peek() {
                Some(b) if b == trailing => CommaState::End,
                _ => CommaState::NoComma,
            },
        }
    }

    fn read_json_number(&mut self) -> Result<serde_json::Number> {
        self.strip_whitespace();
        let mut ret = Vec::new();

        let as_string = match self.peek() {
            Some(b'"') => {
                self.advance(1);
                true
            }
            _ => false,
        };

        while let Some(b) = self.peek() {
            match b {
                b' ' | b'\t' | b'\n' | b'\r' | b'}' | b']' | b',' | b':' | b'"' => {
                    if as_string && b == b'"' {
                        self.advance(1);
                    }
                    break;
                }
                _ => {
                    ret.push(b);
                    self.advance(1);
                }
            }
        }

        let v: std::result::Result<serde_json::Value, _> = serde_json::from_slice(&ret);
        match v {
            Ok(serde_json::Value::Number(n)) => Ok(n),
            _ => bail!("invalid number"),
        }
    }

    fn read_json_value(&mut self, max_depth: i32) -> Result<serde_json::Value> {
        if max_depth <= 0 {
            bail_err!(ProtocolError::SkipDepthExceeded)
        }

        self.strip_whitespace();
        match self.guess_kind()? {
            ValueKind::Null => {
                self.read_null()?;
                Ok(serde_json::Value::Null)
            }
            ValueKind::Bool => {
                let value = self.read_bool()?;
                Ok(serde_json::Value::Bool(value))
            }
            ValueKind::Number => {
                let value = self.read_json_number()?;
                Ok(serde_json::Value::Number(value))
            }
            ValueKind::String => {
                let value = self.read_string()?;
                Ok(serde_json::Value::String(value))
            }
            ValueKind::Array => {
                let mut vec = Vec::new();
                self.read_list_begin()?;
                while self.read_list_value_begin()? {
                    let element = self.read_json_value(max_depth - 1)?;
                    vec.push(element);
                    self.read_list_value_end()?;
                }
                self.read_list_end()?;
                Ok(serde_json::Value::Array(vec))
            }
            ValueKind::Object => {
                let mut map = serde_json::Map::new();
                self.read_struct_begin(|_| ())?;
                while let Some(b'"') = self.peek() {
                    let key = self.read_string()?;
                    self.eat(b":")?;
                    let value = self.read_json_value(max_depth - 1)?;
                    map.insert(key, value);
                    self.read_field_end()?;
                }
                self.read_struct_end()?;
                Ok(serde_json::Value::Object(map))
            }
        }
    }

    fn skip_inner(&mut self, field_type: TType, max_depth: i32) -> Result<()> {
        if max_depth <= 0 {
            bail_err!(ProtocolError::SkipDepthExceeded)
        }

        match field_type {
            TType::Void => {
                self.read_null()?;
            }
            TType::Bool => {
                self.read_bool()?;
            }
            TType::Byte => {
                self.read_byte()?;
            }
            TType::I16 => {
                self.read_i16()?;
            }
            TType::I32 => {
                self.read_i32()?;
            }
            TType::I64 => {
                self.read_i64()?;
            }
            TType::Double => {
                self.read_double()?;
            }
            TType::Float => {
                self.read_float()?;
            }
            TType::Map => bail_err!(ProtocolError::InvalidTypeInSkip(field_type)),
            TType::Set => bail_err!(ProtocolError::InvalidTypeInSkip(field_type)),
            TType::Struct => {
                self.read_struct_begin(|_| ())?;
                loop {
                    let fields = &[];
                    let (_, type_id, _) = self.read_field_begin(|_| (), fields)?;
                    if type_id == TType::Stop {
                        break;
                    }
                    self.skip_inner(type_id, max_depth - 1)?;
                    self.read_field_end()?;
                }
                self.read_struct_end()?;
            }
            TType::List => {
                let (_, len) = self.read_list_begin()?;
                let mut idx = 0;
                loop {
                    let more = self.read_list_value_begin()?;
                    if should_break(len, more, idx + 1) {
                        break;
                    }

                    let elem_type = self.guess_type()?;
                    self.skip_inner(elem_type, max_depth - 1)?;
                    self.read_list_value_end()?;

                    idx += 1;
                }
                self.read_list_end()?;
            }
            TType::UTF8 => {
                self.read_string()?;
            }
            TType::UTF16 => {
                self.read_string()?;
            }
            TType::String => {
                self.read_binary::<Vec<u8>>()?;
            }
            TType::Stream => bail_err!(ProtocolError::StreamUnsupported),
            TType::Stop => bail_err!(ProtocolError::UnexpectedStopInSkip),
        };
        Ok(())
    }

    // Fallback to guessing what "type" of structure we are parsing and mark
    // the field_id as something impossible so we skip everything underneath this string
    fn guess_type(&mut self) -> Result<TType> {
        match self.guess_kind()? {
            ValueKind::Object => Ok(TType::Struct),
            ValueKind::Array => Ok(TType::List),
            ValueKind::String => Ok(TType::UTF8),
            ValueKind::Null => Ok(TType::Void),
            ValueKind::Bool => Ok(TType::Bool),
            ValueKind::Number => Ok(TType::Double),
        }
    }

    fn guess_kind(&mut self) -> Result<ValueKind> {
        match self.peek() {
            Some(b'{') => Ok(ValueKind::Object),
            Some(b'[') => Ok(ValueKind::Array),
            Some(b'"') => Ok(ValueKind::String),
            Some(b'n') => Ok(ValueKind::Null),
            Some(b't') | Some(b'f') => Ok(ValueKind::Bool),
            Some(b'-') => Ok(ValueKind::Number),
            Some(b) if (b as char).is_ascii_digit() => Ok(ValueKind::Number),
            ch => bail!(
                "Expected [, {{, or \", or number after {:?}",
                ch.map(|a| a as char)
            ),
        }
    }

    fn read_null(&mut self) -> Result<()> {
        self.eat(b"null")
    }

    fn check_null(&mut self) -> bool {
        self.strip_whitespace();
        match self.peek() {
            Some(b'n') => true,
            _ => false,
        }
    }
}

impl<B: Buf> ProtocolReader for SimpleJsonProtocolDeserializer<B> {
    fn read_message_begin<F, T>(&mut self, _msgfn: F) -> Result<(T, MessageType, u32)>
    where
        F: FnOnce(&[u8]) -> T,
    {
        bail!("Not implemented")
    }

    fn read_message_end(&mut self) -> Result<()> {
        bail!("Not implemented")
    }

    fn read_struct_begin<F, T>(&mut self, namefn: F) -> Result<T>
    where
        F: FnOnce(&[u8]) -> T,
    {
        self.eat(b"{")?;
        Ok(namefn(&[]))
    }

    fn read_struct_end(&mut self) -> Result<()> {
        self.eat(b"}")?;
        Ok(())
    }

    fn read_field_begin<F, T>(&mut self, fieldfn: F, fields: &[Field]) -> Result<(T, TType, i16)>
    where
        F: FnOnce(&[u8]) -> T,
    {
        // Check if its time to give up
        self.strip_whitespace();
        match self.peek() {
            Some(b'"') => {}
            _ => return Ok((fieldfn(&[]), TType::Stop, -1)),
        }

        // Something went wrong if we dont find a string
        let field_name = self.read_string()?;
        self.eat(b":")?;

        // Did we find a field we know about?
        if let Ok(idx) = fields.binary_search_by_key(&field_name.as_str(), |f| f.name) {
            let field = &fields[idx];
            if !self.check_null() {
                return Ok((fieldfn(field.name.as_bytes()), field.ttype, field.id));
            }
        }

        let elem_type = self.guess_type()?;

        // -1 means we fallthrough to start skipping
        Ok((fieldfn(field_name.as_bytes()), elem_type, -1))
    }

    fn read_field_end(&mut self) -> Result<()> {
        match self.possibly_read_comma(b'}') {
            CommaState::Trailing => bail!("Found trailing comma"),
            CommaState::NoComma => bail!("Missing comma between fields"),
            _ => {}
        }
        Ok(())
    }

    fn read_map_begin(&mut self) -> Result<(TType, TType, Option<usize>)> {
        self.eat(b"{")?;
        // Meaningless type, self.skip_inner and deserialize do not depend on it
        Ok((TType::Stop, TType::Stop, None))
    }

    fn read_map_key_begin(&mut self) -> Result<bool> {
        self.strip_whitespace();
        match self.peek() {
            Some(b'}') => {
                // We are finished with this map
                return Ok(false);
            }
            _ => {}
        }
        Ok(true)
    }

    fn read_map_value_begin(&mut self) -> Result<()> {
        self.eat(b":")?;
        Ok(())
    }

    #[inline]
    fn read_map_value_end(&mut self) -> Result<()> {
        match self.possibly_read_comma(b'}') {
            CommaState::Trailing => bail!("Found trailing comma"),
            CommaState::NoComma => bail!("Missing comma between fields"),
            _ => {}
        }
        Ok(())
    }

    fn read_map_end(&mut self) -> Result<()> {
        self.eat(b"}")?;
        Ok(())
    }

    fn read_list_begin(&mut self) -> Result<(TType, Option<usize>)> {
        self.eat(b"[")?;
        Ok((TType::Stop, None))
    }

    fn read_list_value_begin(&mut self) -> Result<bool> {
        match self.peek() {
            Some(b']') => return Ok(false),
            _ => {}
        }
        Ok(true)
    }

    #[inline]
    fn read_list_value_end(&mut self) -> Result<()> {
        match self.possibly_read_comma(b']') {
            CommaState::Trailing => bail!("Found trailing comma"),
            CommaState::NoComma => bail!("Missing comma between fields"),
            _ => {}
        }
        Ok(())
    }

    fn read_list_end(&mut self) -> Result<()> {
        self.eat(b"]")?;
        Ok(())
    }

    fn read_set_begin(&mut self) -> Result<(TType, Option<usize>)> {
        self.read_list_begin()
    }

    fn read_set_value_begin(&mut self) -> Result<bool> {
        self.read_list_value_begin()
    }

    #[inline]
    fn read_set_value_end(&mut self) -> Result<()> {
        self.read_list_value_end()
    }

    fn read_set_end(&mut self) -> Result<()> {
        self.read_list_end()
    }

    fn read_bool(&mut self) -> Result<bool> {
        match self.eat(b"true") {
            Ok(_) => Ok(true),
            Err(_) => match self.eat(b"false") {
                Ok(_) => Ok(false),
                Err(_) => bail!("Expected `true` or `false`"),
            },
        }
    }

    fn read_byte(&mut self) -> Result<i8> {
        Ok(self.read_i64()? as i8)
    }

    fn read_i16(&mut self) -> Result<i16> {
        Ok(self.read_i64()? as i16)
    }

    fn read_i32(&mut self) -> Result<i32> {
        Ok(self.read_i64()? as i32)
    }

    fn read_i64(&mut self) -> Result<i64> {
        self.read_json_number()?
            .as_i64()
            .ok_or_else(|| anyhow!("Invalid number"))
    }

    fn read_double(&mut self) -> Result<f64> {
        self.read_json_number()?
            .as_f64()
            .ok_or_else(|| anyhow!("Invalid number"))
    }

    fn read_float(&mut self) -> Result<f32> {
        Ok(self.read_double()? as f32)
    }

    fn read_string(&mut self) -> Result<String> {
        self.strip_whitespace();
        self.eat_only(b"\"")?;
        let mut ret = Vec::new();
        ret.push(b'"');
        loop {
            match self.peek() {
                Some(b'"') => {
                    self.advance(1);
                    break;
                }
                Some(b'\\') => {
                    self.advance(1);
                    ret.push(b'\\');
                    match self.peek() {
                        Some(b'"') => {
                            self.advance(1);
                            ret.push(b'"')
                        }
                        Some(b'\\') => {
                            self.advance(1);
                            ret.push(b'\\')
                        }
                        _ => {}
                    }
                }
                Some(b) => {
                    self.advance(1);
                    ret.push(b);
                }
                None => bail!("Expected some char"),
            }
        }
        ret.push(b'"');

        let v: std::result::Result<serde_json::Value, _> = serde_json::from_slice(&ret);
        match v {
            Ok(serde_json::Value::String(s)) => Ok(s),
            _ => bail!("invalid string  "),
        }
    }

    fn read_binary<V: CopyFromBuf>(&mut self) -> Result<V> {
        self.eat(b"\"")?;
        let mut ret = Vec::new();
        loop {
            match self.peek() {
                Some(b'"') => {
                    self.advance(1);
                    break;
                }
                Some(b) => {
                    self.advance(1);
                    ret.push(b);
                }
                None => bail!("Expected some char"),
            }
        }
        let bin = base64::decode_config(&ret, base64::STANDARD_NO_PAD)
            .context("the `binary` data in JSON string does not contain valid base64")?;
        Ok(V::from_vec(bin))
    }

    /// Override the default skip impl to handle random JSON noise
    fn skip(&mut self, field_type: TType) -> Result<()> {
        self.skip_inner(field_type, DEFAULT_RECURSION_DEPTH)
    }
}

pub trait SerializeRef:
    Serialize<SimpleJsonProtocolSerializer<SizeCounter>>
    + Serialize<SimpleJsonProtocolSerializer<BytesMut>>
where
    for<'a> &'a Self: Serialize<SimpleJsonProtocolSerializer<SizeCounter>>,
    for<'a> &'a Self: Serialize<SimpleJsonProtocolSerializer<BytesMut>>,
{
}

impl<T> SerializeRef for T
where
    T: Serialize<SimpleJsonProtocolSerializer<BytesMut>>,
    T: Serialize<SimpleJsonProtocolSerializer<SizeCounter>>,
    for<'a> &'a T: Serialize<SimpleJsonProtocolSerializer<BytesMut>>,
    for<'a> &'a T: Serialize<SimpleJsonProtocolSerializer<SizeCounter>>,
{
}

pub trait Serializable:
    Serialize<SimpleJsonProtocolSerializer<SizeCounter>>
    + Serialize<SimpleJsonProtocolSerializer<BytesMut>>
{
}

impl<T> Serializable for T where
    T: Serialize<SimpleJsonProtocolSerializer<SizeCounter>>
        + Serialize<SimpleJsonProtocolSerializer<BytesMut>>
{
}

/// Serialize a Thrift value using the simple JSON protocol.
pub fn serialize<T>(v: T) -> Bytes
where
    T: Serializable,
{
    let mut sizer = SimpleJsonProtocolSerializer {
        buffer: SizeCounter::new().writer(),
        state: vec![SerializationState::NotInContainer],
    };
    v.write(&mut sizer);

    let sz = sizer.finish();

    // Now that we have the size, allocate an output buffer and serialize into it
    let mut buf = SimpleJsonProtocolSerializer {
        buffer: BytesMut::with_capacity(sz).writer(),
        state: vec![SerializationState::NotInContainer],
    };
    v.write(&mut buf);

    // Done
    buf.finish()
}

pub trait DeserializeSlice:
    for<'a> Deserialize<SimpleJsonProtocolDeserializer<Cursor<&'a [u8]>>>
{
}

impl<T> DeserializeSlice for T where
    T: for<'a> Deserialize<SimpleJsonProtocolDeserializer<Cursor<&'a [u8]>>>
{
}

/// Deserialize a Thrift blob using the compact protocol.
pub fn deserialize<T, B, C>(b: B) -> Result<T>
where
    B: Into<DeserializeSource<C>>,
    C: BufExt,
    T: Deserialize<SimpleJsonProtocolDeserializer<C>>,
{
    let source: DeserializeSource<C> = b.into();
    let mut deser = SimpleJsonProtocolDeserializer::new(source.0);
    let t = T::read(&mut deser)?;
    if deser.peek().is_some() {
        bail!(ProtocolError::TrailingData);
    }
    Ok(t)
}

// Aligns with the variant names of serde_json::Value.
enum ValueKind {
    Null,
    Bool,
    Number,
    String,
    Array,
    Object,
}

impl<B: Buf> Deserialize<SimpleJsonProtocolDeserializer<B>> for serde_json::Value {
    fn read(p: &mut SimpleJsonProtocolDeserializer<B>) -> Result<Self> {
        p.read_json_value(DEFAULT_RECURSION_DEPTH)
    }
}
