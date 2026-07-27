/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

mod count_write;
#[cfg(test)]
mod test;

use std::io;

use bytes::BufMut;
use serde::ser;
use serde::ser::Serialize;

use self::count_write::CountWrite;
use crate::errors::*;
use crate::header::*;

// How full must the buffer get before we start flushing it?
const HIGHWATER: usize = 4096;

pub fn serialize<W, T>(mut writer: W, value: T) -> Result<W>
where
    W: io::Write,
    T: ser::Serialize,
{
    // For the PDU info we need to first count how many bytes it is going to be.
    let mut count_serializer = Serializer::new(CountWrite::new());
    value.serialize(&mut count_serializer)?;
    let count_write = count_serializer.finish()?;
    let count = count_write.count();

    // Now write out the first bits of PDU info.
    // TODO: make this tokio AsyncWrite compatible
    // TODO: support capabilities
    writer.write_all(b"\x00\x02\x00\x00\x00\x00")?;
    let mut serializer = Serializer::new(writer);
    count.serialize(&mut serializer)?;

    // Finally, serialize the value
    value.serialize(&mut serializer)?;
    Ok(serializer.finish()?)
}

pub struct Serializer<W> {
    writer: W,
    scratch: Vec<u8>,
    offset: usize,
}

/// If the value fits in the size specified by `$to`, call the `$put` function.
///
/// This works for all $val types except for `u64`.
macro_rules! maybe_put_int {
    ($self:ident, $val:expr, $to:ident, $put:ident) => {
        let min = $to::MIN as i64;
        let max = $to::MAX as i64;
        let val = $val as i64;
        if val >= min && val <= max {
            return $self.$put($val as $to);
        }
    };
}

impl<W> Serializer<W>
where
    W: io::Write,
{
    // Create a new BSER serializer without leading PDU info.
    fn new(writer: W) -> Self {
        Serializer {
            writer,
            scratch: Vec::with_capacity(HIGHWATER * 2),
            offset: 0,
        }
    }

    /// Write out any internally cached data through to the writer.
    #[inline]
    pub fn flush(&mut self) -> io::Result<()> {
        while self.offset < self.scratch.len() {
            match self.writer.write(&self.scratch[self.offset..]) {
                Ok(n) => self.offset += n,
                Err(ref e) if e.kind() == io::ErrorKind::Interrupted => {}
                Err(e) => return Err(e),
            };
        }
        self.offset = 0;
        unsafe {
            self.scratch.set_len(0);
        }
        Ok(())
    }

    /// Finish writing any buffered data and unwrap the inner `Write`.
    pub fn finish(mut self) -> io::Result<W> {
        self.flush()?;
        Ok(self.writer)
    }

    /// Try flushing any buffered data. If successful, unwrap the inner
    /// `Write`. On failure, return `Self` and the error.
    #[inline]
    pub fn try_finish(mut self) -> ::std::result::Result<W, (Self, io::Error)> {
        match self.flush() {
            Ok(()) => Ok(self.writer),
            Err(e) => Err((self, e)),
        }
    }

    #[inline]
    fn maybe_flush(&mut self) -> Result<()> {
        if self.scratch.len() > HIGHWATER {
            Ok(self.flush()?)
        } else {
            Ok(())
        }
    }

    #[inline]
    fn put_i8(&mut self, v: i8) {
        self.scratch.push(BSER_INT8);
        self.scratch.push(v as u8);
    }

    #[inline]
    fn put_i16(&mut self, v: i16) {
        maybe_put_int!(self, v, i8, put_i8);
        self.scratch.push(BSER_INT16);
        #[cfg(target_endian = "little")]
        self.scratch.put_i16_le(v);
        #[cfg(target_endian = "big")]
        self.scratch.put_i16(v);
    }

    #[inline]
    fn put_i32(&mut self, v: i32) {
        maybe_put_int!(self, v, i16, put_i16);
        self.scratch.push(BSER_INT32);
        #[cfg(target_endian = "little")]
        self.scratch.put_i32_le(v);
        #[cfg(target_endian = "big")]
        self.scratch.put_i32(v);
    }

    #[inline]
    fn put_i64(&mut self, v: i64) {
        maybe_put_int!(self, v, i32, put_i32);
        self.scratch.push(BSER_INT64);
        #[cfg(target_endian = "little")]
        self.scratch.put_i64_le(v);
        #[cfg(target_endian = "big")]
        self.scratch.put_i64(v);
    }
}

macro_rules! write_val {
    ($self:ident, $val:expr) => {{
        $self.maybe_flush()?;
        $self.scratch.push($val);
        Ok(())
    }};
    ($self:ident, $val:expr, $put:ident) => {{
        $self.maybe_flush()?;
        $self.$put($val);
        Ok(())
    }};
}

impl<'a, W> ser::Serializer for &'a mut Serializer<W>
where
    W: io::Write,
{
    type Ok = ();
    type Error = Error;

    type SerializeSeq = Compound<'a, W>;
    type SerializeTuple = Compound<'a, W>;
    type SerializeTupleStruct = Compound<'a, W>;
    type SerializeTupleVariant = Compound<'a, W>;
    type SerializeMap = Compound<'a, W>;
    type SerializeStruct = Compound<'a, W>;
    type SerializeStructVariant = Compound<'a, W>;

    #[inline]
    fn serialize_bool(self, value: bool) -> Result<()> {
        if value {
            write_val!(self, BSER_TRUE)
        } else {
            write_val!(self, BSER_FALSE)
        }
    }

    #[inline]
    fn serialize_i8(self, v: i8) -> Result<()> {
        write_val!(self, v, put_i8)
    }

    #[inline]
    fn serialize_i16(self, v: i16) -> Result<()> {
        write_val!(self, v, put_i16)
    }

    #[inline]
    fn serialize_i32(self, v: i32) -> Result<()> {
        write_val!(self, v, put_i32)
    }

    #[inline]
    fn serialize_i64(self, v: i64) -> Result<()> {
        write_val!(self, v, put_i64)
    }

    #[inline]
    fn serialize_u8(self, v: u8) -> Result<()> {
        maybe_put_int!(self, v, i8, serialize_i8);
        self.serialize_i16(v as i16)
    }

    #[inline]
    fn serialize_u16(self, v: u16) -> Result<()> {
        maybe_put_int!(self, v, i16, serialize_i16);
        self.serialize_i32(v as i32)
    }

    #[inline]
    fn serialize_u32(self, v: u32) -> Result<()> {
        maybe_put_int!(self, v, i32, serialize_i32);
        self.serialize_i64(v as i64)
    }

    #[inline]
    fn serialize_u64(self, v: u64) -> Result<()> {
        // maybe_put_int! doesn't work for u64 because it converts to i64
        // internally.
        if v > (i64::MAX as u64) {
            Err(Error::SerU64TooBig { v })
        } else {
            self.serialize_i64(v as i64)
        }
    }

    #[inline]
    fn serialize_f32(self, v: f32) -> Result<()> {
        self.serialize_f64(v as f64)
    }

    #[inline]
    fn serialize_f64(self, v: f64) -> Result<()> {
        self.maybe_flush()?;
        self.scratch.push(BSER_REAL);
        #[cfg(target_endian = "little")]
        self.scratch.put_f64_le(v);
        #[cfg(target_endian = "big")]
        self.scratch.put_f64(v);
        Ok(())
    }

    #[inline]
    fn serialize_char(self, v: char) -> Result<()> {
        self.serialize_str(&v.to_string())
    }

    #[inline]
    fn serialize_str(self, v: &str) -> Result<()> {
        self.maybe_flush()?;
        self.scratch.push(BSER_UTF8STRING);
        self.put_i64(v.len() as i64);
        self.scratch.extend_from_slice(v.as_bytes());
        Ok(())
    }

    #[inline]
    fn serialize_bytes(self, v: &[u8]) -> Result<()> {
        self.maybe_flush()?;
        self.scratch.push(BSER_BYTESTRING);
        self.put_i64(v.len() as i64);
        self.scratch.extend_from_slice(v);
        Ok(())
    }

    #[inline]
    fn serialize_unit(self) -> Result<()> {
        write_val!(self, BSER_NULL)
    }

    #[inline]
    fn serialize_unit_struct(self, _name: &'static str) -> Result<()> {
        self.serialize_unit()
    }

    #[inline]
    fn serialize_unit_variant(
        self,
        _name: &'static str,
        _variant_index: u32,
        variant: &'static str,
    ) -> Result<()> {
        self.serialize_str(variant)
    }

    #[inline]
    fn serialize_newtype_struct<T>(self, _name: &'static str, value: &T) -> Result<()>
    where
        T: ?Sized + ser::Serialize,
    {
        // This is e.g. E(T). Ignore the E.
        value.serialize(self)
    }

    #[inline]
    fn serialize_newtype_variant<T>(
        self,
        _name: &'static str,
        _variant_index: u32,
        variant: &'static str,
        value: &T,
    ) -> Result<()>
    where
        T: ?Sized + ser::Serialize,
    {
        use serde::ser::SerializeStruct;
        // This is e.g. E { N(T) }, where N is the variant.
        // Serialize this as {variant: value}
        let mut ser_struct = self.serialize_struct("", 1)?;
        ser_struct.serialize_field(variant, value)?;
        ser_struct.end()
    }

    #[inline]
    fn serialize_none(self) -> Result<()> {
        self.serialize_unit()
    }

    #[inline]
    fn serialize_some<T>(self, value: &T) -> Result<()>
    where
        T: ?Sized + ser::Serialize,
    {
        value.serialize(self)
    }

    #[inline]
    fn serialize_seq(self, len: Option<usize>) -> Result<Self::SerializeSeq> {
        match len {
            None => Err(Error::SerNeedSize { kind: "sequence" }),
            Some(len) => self.serialize_tuple(len),
        }
    }

    #[inline]
    fn serialize_tuple(self, len: usize) -> Result<Self::SerializeTuple> {
        // (A, B, C) etc. Serialize this as an array.
        self.maybe_flush()?;
        self.scratch.push(BSER_ARRAY);
        self.put_i64(len as i64);
        Ok(Compound { ser: self })
    }

    #[inline]
    fn serialize_tuple_struct(
        self,
        _name: &'static str,
        len: usize,
    ) -> Result<Self::SerializeTupleStruct> {
        self.serialize_tuple(len)
    }

    #[inline]
    fn serialize_tuple_variant(
        self,
        _name: &'static str,
        _variant_index: u32,
        variant: &'static str,
        len: usize,
    ) -> Result<Self::SerializeTupleVariant> {
        // This is e.g. E { N(A, B, C) }, where N is the variant. Serialize this
        // as { variant: [values] }.
        self.maybe_flush()?;
        self.scratch.push(BSER_OBJECT);
        self.put_i8(1);
        self.serialize_str(variant)?;
        self.serialize_tuple(len)
    }

    #[inline]
    fn serialize_map(self, len: Option<usize>) -> Result<Self::SerializeMap> {
        match len {
            None => Err(Error::SerNeedSize { kind: "map" }),
            Some(len) => self.serialize_struct("", len),
        }
    }

    #[inline]
    fn serialize_struct(self, _name: &'static str, len: usize) -> Result<Self::SerializeStruct> {
        self.maybe_flush()?;
        // BSER objects are serialized as <BSER_OBJECT><len>(<key><value>...).
        self.scratch.push(BSER_OBJECT);
        self.put_i64(len as i64);
        Ok(Compound { ser: self })
    }

    #[inline]
    fn serialize_struct_variant(
        self,
        name: &'static str,
        variant_index: u32,
        variant: &'static str,
        len: usize,
    ) -> Result<Self::SerializeStructVariant> {
        // This is e.g. E { N { foo: A, bar: B } }, where N is the
        // variant. Serialize this as { variant: { foo: valA, bar: valB } }.
        // That's really the same as serialize_tuple_variant.
        self.serialize_tuple_variant(name, variant_index, variant, len)
    }
}

#[doc(hidden)]
pub struct Compound<'a, W> {
    ser: &'a mut Serializer<W>,
}

macro_rules! impl_compound {
    ($trait:ty, [$($fns:ident),*]) => {
        impl<'a, W> $trait for Compound<'a, W>
            where W: io::Write
        {
            type Ok = ();
            type Error = Error;

            $(
                #[inline]
                fn $fns<T>(&mut self, value: &T) -> Result<()>
                    where T: ?Sized + ser::Serialize
                {
                    value.serialize(&mut *self.ser)
                }
            )*

            #[inline]
            fn end(self) -> Result<()> {
                Ok(())
            }
        }
    }
}

impl_compound!(ser::SerializeSeq, [serialize_element]);
impl_compound!(ser::SerializeTuple, [serialize_element]);
impl_compound!(ser::SerializeTupleStruct, [serialize_field]);
impl_compound!(ser::SerializeTupleVariant, [serialize_field]);
impl_compound!(ser::SerializeMap, [serialize_key, serialize_value]);

macro_rules! impl_compound_struct {
    ($trait:ty) => {
        impl<'a, W> $trait for Compound<'a, W>
        where
            W: io::Write,
        {
            type Ok = ();
            type Error = Error;

            #[inline]
            fn serialize_field<T>(&mut self, key: &'static str, value: &T) -> Result<()>
            where
                T: ?Sized + ser::Serialize,
            {
                // TODO: this can go wrong if writing out the key succeeds but
                // writing the value fails.
                ser::SerializeMap::serialize_key(self, key)?;
                ser::SerializeMap::serialize_value(self, value)
            }

            #[inline]
            fn end(self) -> Result<()> {
                ser::SerializeMap::end(self)
            }
        }
    };
}

impl_compound_struct!(ser::SerializeStruct);
impl_compound_struct!(ser::SerializeStructVariant);
