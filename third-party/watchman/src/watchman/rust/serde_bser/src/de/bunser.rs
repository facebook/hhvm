/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! Internal stateless code for handling BSER deserialization.

use anyhow::Context as _;
use byteorder::ByteOrder;
use byteorder::NativeEndian;

use crate::de::read::DeRead;
use crate::de::read::Reference;
use crate::errors::*;
use crate::header::*;

#[derive(Debug)]
pub struct Bunser<R> {
    read: R,
    scratch: Vec<u8>,
}

pub struct PduInfo {
    pub bser_capabilities: u32,
    pub len: i64,
    pub start: i64,
}

impl<'de, R> Bunser<R>
where
    R: DeRead<'de>,
{
    pub fn new(read: R) -> Self {
        Bunser {
            read,
            scratch: Vec::with_capacity(128),
        }
    }

    /// Read the PDU off the stream. This should be called in the beginning.
    pub fn read_pdu(&mut self) -> Result<PduInfo> {
        {
            let magic = self.read_bytes(2)?;
            if magic.get_ref() != &EMPTY_HEADER[..2] {
                return Err(Error::DeInvalidMagic {
                    magic: Vec::from(magic.get_ref()),
                });
            }
        }
        let bser_capabilities = self
            .read
            .next_u32(&mut self.scratch)
            .map_err(Error::de_reader_error)?;
        let len = self.check_next_int()?;
        let start = self.read_count();
        Ok(PduInfo {
            bser_capabilities,
            len,
            start,
        })
    }

    pub fn read_count(&self) -> i64 {
        self.read.read_count() as i64
    }

    pub fn end(&self, pdu_info: &PduInfo) -> Result<()> {
        let expected = (pdu_info.start + pdu_info.len) as usize;
        if self.read.read_count() != expected {
            return Err(Error::DeEof {
                expected,
                read: self.read.read_count(),
            });
        }
        Ok(())
    }

    #[inline]
    pub fn peek(&mut self) -> Result<u8> {
        self.read.peek().map_err(Error::de_reader_error)
    }

    #[inline]
    pub fn discard(&mut self) {
        self.read.discard();
    }

    /// Return a borrowed or copied version of the next n bytes.
    #[inline]
    pub fn read_bytes<'s>(&'s mut self, len: i64) -> Result<Reference<'de, 's, [u8]>> {
        let len = len as usize;
        self.read
            .next_bytes(len, &mut self.scratch)
            .map_err(Error::de_reader_error)
    }

    /// Return the next i8 value. This assumes the caller already knows the next
    /// value is an i8.
    pub fn next_i8(&mut self) -> Result<i8> {
        self.read.discard();
        let bytes = self
            .read_bytes(1)
            .context("error while reading i8")
            .map_err(Error::de_reader_error)?
            .get_ref();
        Ok(bytes[0] as i8)
    }

    /// Return the next i16 value. This assumes the caller already knows the
    /// next value is an i16.
    pub fn next_i16(&mut self) -> Result<i16> {
        self.read.discard();
        let bytes = self
            .read_bytes(2)
            .context("error while reading i16")
            .map_err(Error::de_reader_error)?
            .get_ref();
        Ok(NativeEndian::read_i16(bytes))
    }

    /// Return the next i32 value. This assumes the caller already knows the
    /// next value is an i32.
    pub fn next_i32(&mut self) -> Result<i32> {
        self.read.discard();
        let bytes = self
            .read_bytes(4)
            .context("error while reading i32")
            .map_err(Error::de_reader_error)?
            .get_ref();
        Ok(NativeEndian::read_i32(bytes))
    }

    /// Return the next i64 value. This assumes the caller already knows the
    /// next value is an i64.
    pub fn next_i64(&mut self) -> Result<i64> {
        self.read.discard();
        let bytes = self
            .read_bytes(8)
            .context("error while reading i64")
            .map_err(Error::de_reader_error)?
            .get_ref();
        Ok(NativeEndian::read_i64(bytes))
    }

    /// Check and return the next integer value. Errors out if the next value is
    /// not actually an int.
    pub fn check_next_int(&mut self) -> Result<i64> {
        let value = match self.peek()? {
            BSER_INT8 => self.next_i8()? as i64,
            BSER_INT16 => self.next_i16()? as i64,
            BSER_INT32 => self.next_i32()? as i64,
            BSER_INT64 => self.next_i64()?,
            ch => {
                return Err(Error::DeInvalidStartByte {
                    kind: "integer".into(),
                    byte: ch,
                });
            }
        };

        Ok(value)
    }

    pub fn next_f64(&mut self) -> Result<f64> {
        self.read.discard();
        let bytes = self
            .read_bytes(8)
            .context("error while reading f64")
            .map_err(Error::de_reader_error)?
            .get_ref();
        Ok(NativeEndian::read_f64(bytes))
    }
}
