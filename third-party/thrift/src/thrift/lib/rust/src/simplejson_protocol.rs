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

use std::io;
use std::io::Cursor;
use std::num::ParseFloatError;
use std::str::FromStr;

use anyhow::Context;
use anyhow::Result;
use anyhow::anyhow;
use anyhow::bail;
use base64::Engine;
use base64::alphabet::STANDARD;
use base64::engine::DecodePaddingMode;
use base64::engine::general_purpose::GeneralPurpose;
use base64::engine::general_purpose::NO_PAD;
use bufsize::SizeCounter;
use bytes::Buf;
use bytes::BufMut;
use bytes::Bytes;
use bytes::BytesMut;
use bytes::buf::Writer;
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
use crate::protocol::DEFAULT_RECURSION_DEPTH;
use crate::protocol::Field;
use crate::protocol::Protocol;
use crate::protocol::ProtocolReader;
use crate::protocol::ProtocolWriter;
use crate::protocol::should_break;
use crate::serialize::Serialize;
use crate::thrift_protocol::MessageType;
use crate::thrift_protocol::ProtocolID;
use crate::ttype::TType;

// Bring back the pre 0.20 bevahiour and allow either padded or un-padded base64 strings at decode time.
const STANDARD_NO_PAD_INDIFFERENT: GeneralPurpose = GeneralPurpose::new(
    &STANDARD,
    NO_PAD.with_decode_padding_mode(DecodePaddingMode::Indifferent),
);

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
    #[inline]
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
    #[inline]
    fn deserializer(buf: F::DecBuf) -> Self::Deserializer {
        SimpleJsonProtocolDeserializer::new(buf)
    }
    #[inline]
    fn into_buffer(deser: Self::Deserializer) -> F::DecBuf {
        deser.into_inner()
    }
}

impl<B: BufMutExt> SimpleJsonProtocolSerializer<B> {
    #[inline]
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
    #[inline]
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
    #[inline]
    fn write_floating_point_number<F, W>(&mut self, value: F, finite_writer: W)
    where
        F: num_traits::Float,
        W: FnOnce(&mut CompactFormatter, &mut Writer<B>, F) -> io::Result<()>,
    {
        if value.is_infinite() {
            if value.is_sign_positive() {
                self.write_string("Infinity");
            } else {
                self.write_string("-Infinity");
            }
        } else if value.is_nan() {
            self.write_string("NaN");
        } else {
            finite_writer(&mut CompactFormatter, &mut self.buffer, value)
                .expect("Somehow failed to do \"io\" on a buffer");
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
    #[inline]
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
    #[inline]
    fn write_map_begin(&mut self, _key_type: TType, _value_type: TType, _size: usize) {
        self.state.push(SerializationState::JustEnteredContainer);
        CompactFormatter
            .begin_object(&mut self.buffer)
            .expect("Somehow failed to do \"io\" on a buffer");
    }
    #[inline]
    fn write_map_key_begin(&mut self) {
        self.possibly_write_comma();
        *self
            .state
            .last_mut()
            .expect("Invariant of encoding state violated") = SerializationState::InContainerKey;
    }
    #[inline]
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
    #[inline]
    fn write_list_begin(&mut self, _elem_type: TType, _size: usize) {
        CompactFormatter
            .begin_array(&mut self.buffer)
            .expect("Somehow failed to do \"io\" on a buffer");
        self.state.push(SerializationState::JustEnteredContainer);
    }
    #[inline]
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
    #[inline]
    fn write_set_begin(&mut self, elem_type: TType, size: usize) {
        self.write_list_begin(elem_type, size);
    }
    #[inline]
    fn write_set_value_begin(&mut self) {
        self.write_list_value_begin();
    }
    #[inline]
    fn write_set_end(&mut self) {
        self.write_list_end();
    }
    #[inline]
    fn write_bool(&mut self, value: bool) {
        CompactFormatter
            .write_bool(&mut self.buffer, value)
            .expect("Somehow failed to do \"io\" on a buffer");
    }
    #[inline]
    fn write_byte(&mut self, value: i8) {
        if self.writing_key() {
            self.write_string(&value.to_string());
            return;
        }
        CompactFormatter
            .write_i8(&mut self.buffer, value)
            .expect("Somehow failed to do \"io\" on a buffer");
    }
    #[inline]
    fn write_i16(&mut self, value: i16) {
        if self.writing_key() {
            self.write_string(&value.to_string());
            return;
        }
        CompactFormatter
            .write_i16(&mut self.buffer, value)
            .expect("Somehow failed to do \"io\" on a buffer");
    }
    #[inline]
    fn write_i32(&mut self, value: i32) {
        if self.writing_key() {
            self.write_string(&value.to_string());
            return;
        }
        CompactFormatter
            .write_i32(&mut self.buffer, value)
            .expect("Somehow failed to do \"io\" on a buffer");
    }
    #[inline]
    fn write_i64(&mut self, value: i64) {
        if self.writing_key() {
            self.write_string(&value.to_string());
            return;
        }
        CompactFormatter
            .write_i64(&mut self.buffer, value)
            .expect("Somehow failed to do \"io\" on a buffer");
    }
    #[inline]
    fn write_double(&mut self, value: f64) {
        if self.writing_key() {
            self.write_string(&value.to_string());
            return;
        }
        self.write_floating_point_number(value, CompactFormatter::write_f64::<Writer<B>>);
    }
    #[inline]
    fn write_float(&mut self, value: f32) {
        if self.writing_key() {
            self.write_string(&value.to_string());
            return;
        }
        self.write_floating_point_number(value, CompactFormatter::write_f32::<Writer<B>>);
    }
    #[inline]
    fn write_string(&mut self, value: &str) {
        serde_json::to_writer(&mut self.buffer, value)
            .expect("Somehow failed to do \"io\" on a buffer");
    }
    #[inline]
    fn write_binary(&mut self, value: &[u8]) {
        CompactFormatter
            .begin_string(&mut self.buffer)
            .expect("Somehow failed to do \"io\" on a buffer");
        CompactFormatter
            .write_raw_fragment(&mut self.buffer, &STANDARD_NO_PAD_INDIFFERENT.encode(value))
            .expect("Somehow failed to do \"io\" on a buffer");
        CompactFormatter
            .end_string(&mut self.buffer)
            .expect("Somehow failed to do \"io\" on a buffer");
    }
    #[inline]
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

struct JsonNumberString {
    string: Vec<u8>,
    quoted: bool,
}

trait From32Infinity {
    fn from_f32_inf() -> Self;
}

impl From32Infinity for f32 {
    fn from_f32_inf() -> Self {
        f32::INFINITY
    }
}

impl From32Infinity for f64 {
    fn from_f32_inf() -> Self {
        3.4028237e38f64
    }
}

trait From32NegInfinity {
    fn from_f32_neg_inf() -> Self;
}

impl From32NegInfinity for f32 {
    fn from_f32_neg_inf() -> Self {
        f32::NEG_INFINITY
    }
}

impl From32NegInfinity for f64 {
    fn from_f32_neg_inf() -> Self {
        -3.4028237e38f64
    }
}

trait FromF64Infinity {
    fn from_f64_inf() -> Self;
}

impl FromF64Infinity for f32 {
    fn from_f64_inf() -> Self {
        f32::NAN
    }
}

impl FromF64Infinity for f64 {
    fn from_f64_inf() -> Self {
        f64::INFINITY
    }
}

trait FromF64NegInfinity {
    fn from_f64_neg_inf() -> Self;
}

impl FromF64NegInfinity for f32 {
    fn from_f64_neg_inf() -> Self {
        f32::NAN
    }
}

impl FromF64NegInfinity for f64 {
    fn from_f64_neg_inf() -> Self {
        f64::NEG_INFINITY
    }
}

impl<B: Buf> SimpleJsonProtocolDeserializer<B> {
    #[inline]
    pub fn new(buffer: B) -> Self {
        let remaining = buffer.remaining();
        SimpleJsonProtocolDeserializer { buffer, remaining }
    }

    #[inline]
    pub fn into_inner(self) -> B {
        self.buffer
    }

    // TODO(azw): codify this (number of bytes left) in the Deserializer API
    // `bytes` (named `chunks` in bytes1.0) does not represent a contiguous slice
    // of the remaining bytes. All we can do is check if there is a byte to return
    /// Returns a byte from the underly buffer if there is enough remaining
    #[inline]
    pub fn peek(&self) -> Option<u8> {
        // fast path like https://docs.rs/bytes/1.0.1/src/bytes/buf/buf_impl.rs.html#18
        if !self.buffer.chunk().is_empty() {
            Some(self.buffer.chunk()[0])
        } else {
            None
        }
    }

    /// Like peek but panics if there is none remaining
    #[inline]
    fn peek_can_panic(&self) -> u8 {
        self.buffer.chunk()[0]
    }
    #[inline]
    fn strip_whitespace(&mut self) {
        while let Some(b) = self.peek() {
            if !&[b' ', b'\t', b'\n', b'\r'].contains(&b) {
                break;
            }
            self.advance(1);
        }
    }
    // Validates that next chars is `val`
    #[inline]
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
                bail!(
                    "Expected '{}' got '{}'",
                    char::from(*to_check),
                    char::from(b)
                )
            }
            self.advance(1);
        }
        Ok(())
    }

    // Validates that next chars is `val`. Strip whitespace before and after
    #[inline]
    fn eat(&mut self, val: &[u8]) -> Result<()> {
        self.strip_whitespace();
        self.eat_only(val)?;
        self.strip_whitespace();
        Ok(())
    }
    #[inline]
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
    #[inline]
    fn possibly_read_comma(&mut self, trailing: u8) -> CommaState {
        self.strip_whitespace();
        match self.peek() {
            Some(b',') => {
                self.advance(1);
                self.strip_whitespace();
                match self.peek() {
                    Some(b) if b == trailing => CommaState::Trailing,
                    _ => CommaState::NonTrailing,
                }
            }
            Some(b) if b == trailing => CommaState::End,
            _ => CommaState::NoComma,
        }
    }
    #[inline]
    fn read_json_number_string(&mut self) -> Result<JsonNumberString> {
        let mut string = Vec::new();

        let quoted = match self.peek() {
            Some(b'"') => {
                self.advance(1);
                true
            }
            _ => false,
        };

        while let Some(b) = self.peek() {
            match b {
                b' ' | b'\t' | b'\n' | b'\r' | b'}' | b']' | b',' | b':' => {
                    if quoted {
                        bail!("Missing closing quote \" for number")
                    }
                    break;
                }
                b'"' => {
                    if quoted {
                        self.advance(1);
                    }
                    break;
                }
                _ => {
                    string.push(b);
                    self.advance(1);
                }
            }
        }

        Ok(JsonNumberString { string, quoted })
    }
    /// Used for deserializing into a `serde_json::Value`.
    #[inline]
    fn read_json_number(&mut self) -> Result<serde_json::Number> {
        self.strip_whitespace();
        let JsonNumberString { string, .. } = self.read_json_number_string()?;

        serde_json::from_slice(&string).map_err(Into::into)
    }
    #[inline]
    fn read_json_integer_number(&mut self) -> Result<serde_json::Number> {
        self.strip_whitespace();
        let JsonNumberString { string, .. } = self.read_json_number_string()?;

        let v: std::result::Result<serde_json::Value, _> = serde_json::from_slice(&string);
        match v {
            Ok(serde_json::Value::Number(n)) => Ok(n),
            _ => bail!("Invalid number"),
        }
    }
    #[inline]
    fn read_floating_point_number<F>(&mut self) -> Result<F>
    where
        F: num_traits::Float
            + FromStr<Err = ParseFloatError>
            + From32Infinity
            + From32NegInfinity
            + FromF64Infinity
            + FromF64NegInfinity,
    {
        self.strip_whitespace();
        let JsonNumberString { string, quoted } = self.read_json_number_string()?;

        let result = match &string[..] {
            b"NaN" => {
                if !quoted {
                    bail!("NaN must be quoted")
                }
                F::nan()
            }
            b"-NaN" => {
                if !quoted {
                    bail!("-NaN must be quoted")
                }
                -F::nan()
            }
            b"Infinity" => {
                if !quoted {
                    bail!("Infinity must be quoted")
                }
                F::infinity()
            }
            b"-Infinity" => {
                if !quoted {
                    bail!("-Infinity must be quoted")
                }
                F::neg_infinity()
            }
            b"1.797693134862316e308" => F::from_f64_inf(),
            b"-1.797693134862316e308" => F::from_f64_neg_inf(),
            b"3.4028237e38" => F::from_f32_inf(),
            b"-3.4028237e38" => F::from_f32_neg_inf(),
            other => {
                let s = std::str::from_utf8(other)
                    .context("Invalid UTF-8 for floating point number")?;
                // Curiously, this parses the below incorrectly-serialized nan/infinities as infinite nans,
                // instead of erroring out like other float parsers.
                // We currently rely on this behavior (but its unit-tested!).
                let x = s.parse::<F>().context("Invalid floating point number")?;

                // This if-condition is added here for backward compatibility with the original Rust simplejson_protocol::serialize
                // and can be removed in the future along with excessive match arms and corresponding tests. For reference, it used
                // to be that the original Rust serializer produced a string representation of a floating point number by
                // serde_json::Formatter::write_f64 or serde_json::Formatter::write_32, both of each boil down to ryu::Buffer::format_finite.
                //
                // Examples of this include but are not limited to:
                // - "2.696539702293474e308" for f64::NAN
                // - "-2.696539702293474e308" for -f64::NAN
                // - "1.797693134862316e308" for f64::INFINITY
                // - "-1.797693134862316e308" for f64::NEG_INFINITY
                // - "5.1042355e38" for f32::NAN
                // - "-5.1042355e38" for -f32::NAN
                // - "3.4028237e38" for f32::INFINITY
                // - "-3.4028237e38" for f32::NEG_INFINITY
                //
                // Given that we've checked for string representations of positive and negative infinity above, combined with the fact that
                // the standard parser successfully parses every grammar correct string representation of NaN values into ±Infinity or ±NaN as
                // "closest representable floating-point number", we can safely turn infinities into NaNs and all other values into themselves.
                if x.is_infinite() { F::nan() } else { x }
            }
        };

        Ok(result)
    }
    #[inline]
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
                self.read_list_begin_unchecked()?;
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
                    let key = self.read_string().context("Expected a object key")?;
                    self.eat(b":")
                        .context("Expected a colon between object key and value")?;
                    let value = self.read_json_value(max_depth - 1)?;
                    map.insert(key, value);
                    self.read_field_end()?;
                }
                self.read_struct_end()?;
                Ok(serde_json::Value::Object(map))
            }
        }
    }
    #[inline]
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
                let (_, len) = self.read_list_begin_unchecked()?;
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
    #[inline]
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
    #[inline]
    fn guess_kind(&mut self) -> Result<ValueKind> {
        match self.peek() {
            Some(b'{') => Ok(ValueKind::Object),
            Some(b'[') => Ok(ValueKind::Array),
            Some(b'"') => Ok(ValueKind::String),
            Some(b'n') => Ok(ValueKind::Null),
            Some(b't') | Some(b'f') => Ok(ValueKind::Bool),
            Some(b'-') | Some(b'I') | Some(b'N') => Ok(ValueKind::Number),
            Some(b) if (b as char).is_ascii_digit() => Ok(ValueKind::Number),
            ch => bail!(
                "Expected [, {{, or \", or number after {:?}",
                ch.map(|a| a as char)
            ),
        }
    }
    #[inline]
    fn read_null(&mut self) -> Result<()> {
        self.eat(b"null").context("Expected null")
    }
    #[inline]
    fn check_null(&mut self) -> bool {
        self.strip_whitespace();
        match self.peek() {
            Some(b'n') => true,
            _ => false,
        }
    }
}

impl<B: Buf> ProtocolReader for SimpleJsonProtocolDeserializer<B> {
    #[inline]
    fn read_message_begin<F, T>(&mut self, _msgfn: F) -> Result<(T, MessageType, u32)>
    where
        F: FnOnce(&[u8]) -> T,
    {
        bail!("Not implemented")
    }
    #[inline]
    fn read_message_end(&mut self) -> Result<()> {
        bail!("Not implemented")
    }
    #[inline]
    fn read_struct_begin<F, T>(&mut self, namefn: F) -> Result<T>
    where
        F: FnOnce(&[u8]) -> T,
    {
        self.eat(b"{").context("Expected a start of a struct")?;
        Ok(namefn(&[]))
    }
    #[inline]
    fn read_struct_end(&mut self) -> Result<()> {
        self.eat(b"}").context("Expected an end of a struct")?;
        Ok(())
    }
    #[inline]
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
        let field_name = self.read_string().context("Expected a field name")?;
        self.eat(b":")
            .context("Expected a colon between struct key and value")?;

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
    #[inline]
    fn read_field_end(&mut self) -> Result<()> {
        match self.possibly_read_comma(b'}') {
            CommaState::Trailing => bail!("Found trailing comma"),
            CommaState::NoComma => bail!("Missing comma between fields"),
            _ => {}
        }
        Ok(())
    }
    #[inline]
    fn read_map_begin_unchecked(&mut self) -> Result<(TType, TType, Option<usize>)> {
        self.eat(b"{").context("Expected a start of a list")?;
        // Meaningless type, self.skip_inner and deserialize do not depend on it
        Ok((TType::Stop, TType::Stop, None))
    }
    #[inline]
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
    #[inline]
    fn read_map_value_begin(&mut self) -> Result<()> {
        self.eat(b":")
            .context("Expected a colon between map key and value")?;
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
    #[inline]
    fn read_map_end(&mut self) -> Result<()> {
        self.eat(b"}").context("Expected an end of a map")?;
        Ok(())
    }
    #[inline]
    fn read_list_begin_unchecked(&mut self) -> Result<(TType, Option<usize>)> {
        self.eat(b"[").context("Expected a start of a list")?;
        Ok((TType::Stop, None))
    }
    #[inline]
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
    #[inline]
    fn read_list_end(&mut self) -> Result<()> {
        self.eat(b"]").context("Expected an end of a list")?;
        Ok(())
    }
    #[inline]
    fn read_set_begin_unchecked(&mut self) -> Result<(TType, Option<usize>)> {
        self.read_list_begin_unchecked()
    }
    #[inline]
    fn read_set_value_begin(&mut self) -> Result<bool> {
        self.read_list_value_begin()
    }

    #[inline]
    fn read_set_value_end(&mut self) -> Result<()> {
        self.read_list_value_end()
    }
    #[inline]
    fn read_set_end(&mut self) -> Result<()> {
        self.read_list_end()
    }
    #[inline]
    fn read_bool(&mut self) -> Result<bool> {
        self.strip_whitespace();
        match self.peek() {
            Some(b't') => {
                self.eat_only(b"true").context("Expected `true`")?;
                self.strip_whitespace();
                Ok(true)
            }
            Some(b'f') => {
                self.eat_only(b"false").context("Expected `false`")?;
                self.strip_whitespace();
                Ok(false)
            }
            _ => bail!("Expected `true` or `false`"),
        }
    }
    #[inline]
    fn read_byte(&mut self) -> Result<i8> {
        Ok(self.read_i64()? as i8)
    }
    #[inline]
    fn read_i16(&mut self) -> Result<i16> {
        Ok(self.read_i64()? as i16)
    }
    #[inline]
    fn read_i32(&mut self) -> Result<i32> {
        Ok(self.read_i64()? as i32)
    }
    #[inline]
    fn read_i64(&mut self) -> Result<i64> {
        self.read_json_integer_number()?
            .as_i64()
            .ok_or_else(|| anyhow!("Invalid number"))
    }
    #[inline]
    fn read_double(&mut self) -> Result<f64> {
        self.read_floating_point_number()
    }
    #[inline]
    fn read_float(&mut self) -> Result<f32> {
        self.read_floating_point_number()
    }
    #[inline]
    fn read_string(&mut self) -> Result<String> {
        self.strip_whitespace();
        self.eat_only(b"\"")
            .context("Expected a start of a string")?;
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
                None => bail!("Expected the string to continue"),
            }
        }
        ret.push(b'"');

        let v: std::result::Result<serde_json::Value, _> = serde_json::from_slice(&ret);
        match v {
            Ok(serde_json::Value::String(s)) => Ok(s),
            _ => bail!("Invalid string"),
        }
    }
    #[inline]
    fn read_binary<V: CopyFromBuf>(&mut self) -> Result<V> {
        self.eat(b"\"").context("Expected a start of a string")?;
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
                None => bail!("Expected the string to continue"),
            }
        }
        let bin = STANDARD_NO_PAD_INDIFFERENT
            .decode(&ret)
            .context("The `binary` data in JSON string does not contain valid base64")?;
        Ok(V::from_vec(bin))
    }

    /// Override the default skip impl to handle random JSON noise
    #[inline]
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
#[inline]
pub fn serialize<T>(v: T) -> Bytes
where
    T: Serializable,
{
    let mut sizer = SimpleJsonProtocolSerializer {
        buffer: SizeCounter::new().writer(),
        state: vec![SerializationState::NotInContainer],
    };
    v.rs_thrift_write(&mut sizer);

    let sz = sizer.finish();

    // Now that we have the size, allocate an output buffer and serialize into it
    let mut buf = SimpleJsonProtocolSerializer {
        buffer: BytesMut::with_capacity(sz).writer(),
        state: vec![SerializationState::NotInContainer],
    };
    v.rs_thrift_write(&mut buf);

    // Done
    buf.finish()
}

/// Serialize a Thrift value using the simple JSON protocol from a reference.
/// This avoids taking ownership of the value being serialized.
#[inline]
pub fn serialize_ref<T>(v: &T) -> Bytes
where
    T: SerializeRef,
{
    let mut sizer = SimpleJsonProtocolSerializer {
        buffer: SizeCounter::new().writer(),
        state: vec![SerializationState::NotInContainer],
    };
    v.rs_thrift_write(&mut sizer);

    let sz = sizer.finish();

    // Now that we have the size, allocate an output buffer and serialize into it
    let mut buf = SimpleJsonProtocolSerializer {
        buffer: BytesMut::with_capacity(sz).writer(),
        state: vec![SerializationState::NotInContainer],
    };
    v.rs_thrift_write(&mut buf);

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
#[inline]
pub fn deserialize<T, B, C>(b: B) -> Result<T>
where
    B: Into<DeserializeSource<C>>,
    C: BufExt,
    T: Deserialize<SimpleJsonProtocolDeserializer<C>>,
{
    let source: DeserializeSource<C> = b.into();
    let mut deser = SimpleJsonProtocolDeserializer::new(source.0);
    let t = T::rs_thrift_read(&mut deser)?;
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
    #[inline]
    fn rs_thrift_read(p: &mut SimpleJsonProtocolDeserializer<B>) -> Result<Self> {
        p.read_json_value(DEFAULT_RECURSION_DEPTH)
    }
}
