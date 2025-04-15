/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#[cfg(feature = "debug_bytes")]
use std::fmt;
use std::io;
use std::result;

use anyhow::Context as _;
use anyhow::bail;
use byteorder::ByteOrder;
use byteorder::NativeEndian;

#[cfg(feature = "debug_bytes")]
macro_rules! debug_bytes {
    ($($arg:tt)*) => {
       eprint!($($arg)*);
    }
}

#[cfg(not(feature = "debug_bytes"))]
macro_rules! debug_bytes {
    ($($arg:tt)*) => {};
}

#[cfg(feature = "debug_bytes")]
struct ByteBuf<'a>(&'a [u8]);
#[cfg(feature = "debug_bytes")]
impl<'a> fmt::LowerHex for ByteBuf<'a> {
    fn fmt(&self, fmtr: &mut fmt::Formatter) -> result::Result<(), fmt::Error> {
        for byte in self.0 {
            let val = byte.clone();
            if (val >= b'-' && val <= b'9')
                || (val >= b'A' && val <= b'Z')
                || (val >= b'a' && val <= b'z')
                || val == b'_'
            {
                fmtr.write_fmt(format_args!("{}", val as char))?;
            } else {
                fmtr.write_fmt(format_args!(r"\x{:02x}", byte))?;
            }
        }
        Ok(())
    }
}

pub trait DeRead<'de> {
    /// read next byte (if peeked byte not discarded return it)
    fn next(&mut self) -> anyhow::Result<u8>;
    /// peek next byte (peeked byte should come in next and next_bytes unless discarded)
    fn peek(&mut self) -> anyhow::Result<u8>;
    /// how many bytes have been read so far.
    /// this doesn't include the peeked byte
    fn read_count(&self) -> usize;
    /// discard peeked byte
    fn discard(&mut self);
    /// read next byte (if peeked byte not discarded include it)
    fn next_bytes<'s>(
        &'s mut self,
        len: usize,
        scratch: &'s mut Vec<u8>,
    ) -> anyhow::Result<Reference<'de, 's, [u8]>>;
    /// read u32 as native endian
    fn next_u32(&mut self, scratch: &mut Vec<u8>) -> anyhow::Result<u32> {
        let bytes = self
            .next_bytes(4, scratch)
            .context("error while parsing u32")?
            .get_ref();
        Ok(NativeEndian::read_u32(bytes))
    }
}

pub struct SliceRead<'a> {
    slice: &'a [u8],
    index: usize,
}

impl<'a> SliceRead<'a> {
    pub fn new(slice: &'a [u8]) -> Self {
        SliceRead { slice, index: 0 }
    }
}

pub struct IoRead<R>
where
    R: io::Read,
{
    reader: R,
    read_count: usize,
    /// Temporary storage of peeked byte.
    peeked: Option<u8>,
}

impl<R> IoRead<R>
where
    R: io::Read,
{
    pub fn new(reader: R) -> Self {
        debug_bytes!("Read bytes:\n");
        IoRead {
            reader,
            read_count: 0,
            peeked: None,
        }
    }
}

impl<R> Drop for IoRead<R>
where
    R: io::Read,
{
    fn drop(&mut self) {
        debug_bytes!("\n");
    }
}

impl<'a> DeRead<'a> for SliceRead<'a> {
    fn next(&mut self) -> anyhow::Result<u8> {
        if self.index >= self.slice.len() {
            bail!("eof while reading next byte");
        }
        let ch = self.slice[self.index];
        self.index += 1;
        Ok(ch)
    }

    fn peek(&mut self) -> anyhow::Result<u8> {
        if self.index >= self.slice.len() {
            bail!("eof while peeking next byte");
        }
        Ok(self.slice[self.index])
    }

    #[inline]
    fn read_count(&self) -> usize {
        self.index
    }

    #[inline]
    fn discard(&mut self) {
        self.index += 1;
    }

    fn next_bytes<'s>(
        &'s mut self,
        len: usize,
        _scratch: &'s mut Vec<u8>,
    ) -> anyhow::Result<Reference<'a, 's, [u8]>> {
        // BSER has no escaping or anything similar, so just go ahead and return
        // a reference to the bytes.
        if self.index + len > self.slice.len() {
            bail!("eof while parsing bytes/string");
        }
        let borrowed = &self.slice[self.index..(self.index + len)];
        self.index += len;
        Ok(Reference::Borrowed(borrowed))
    }
}

impl<'de, R> DeRead<'de> for IoRead<R>
where
    R: io::Read,
{
    fn next(&mut self) -> anyhow::Result<u8> {
        match self.peeked.take() {
            Some(peeked) => Ok(peeked),
            None => {
                let mut buffer = [0; 1];
                self.reader.read_exact(&mut buffer)?;
                debug_bytes!("{:x}", ByteBuf(&buffer));
                self.read_count += 1;
                Ok(buffer[0])
            }
        }
    }

    fn peek(&mut self) -> anyhow::Result<u8> {
        match self.peeked {
            Some(peeked) => Ok(peeked),
            None => {
                let mut buffer = [0; 1];
                self.reader.read_exact(&mut buffer)?;
                debug_bytes!("{:x}", ByteBuf(&buffer));
                self.peeked = Some(buffer[0]);
                self.read_count += 1;
                Ok(buffer[0])
            }
        }
    }

    #[inline]
    fn read_count(&self) -> usize {
        match self.peeked {
            Some(_) => self.read_count - 1,
            None => self.read_count,
        }
    }

    #[inline]
    fn discard(&mut self) {
        self.peeked = None
    }

    fn next_bytes<'s>(
        &'s mut self,
        len: usize,
        scratch: &'s mut Vec<u8>,
    ) -> anyhow::Result<Reference<'de, 's, [u8]>> {
        scratch.resize(len, 0);
        let mut idx = 0;
        if self.peeked.is_some() {
            idx += 1;
        }
        if idx < len {
            self.reader.read_exact(&mut scratch[idx..len])?;
            debug_bytes!("{:x}", ByteBuf(&scratch[idx..len]));
            self.read_count += len - idx;
        }
        if let Some(peeked) = self.peeked.take() {
            scratch[0] = peeked;
        }
        Ok(Reference::Copied(&scratch[0..len]))
    }
}

#[derive(Debug)]
pub enum Reference<'b, 'c, T: ?Sized> {
    Borrowed(&'b T),
    Copied(&'c T),
}

impl<'b, 'c, T> Reference<'b, 'c, T>
where
    T: ?Sized,
{
    pub fn map_result<F, U, E>(self, f: F) -> anyhow::Result<Reference<'b, 'c, U>>
    where
        F: FnOnce(&T) -> result::Result<&U, E>,
        E: std::error::Error + Send + Sync + 'static,
        U: ?Sized + 'b + 'c,
    {
        match self {
            Reference::Borrowed(borrowed) => Ok(Reference::Borrowed(f(borrowed)?)),
            Reference::Copied(copied) => Ok(Reference::Copied(f(copied)?)),
        }
    }

    pub fn get_ref<'a>(&self) -> &'a T
    where
        'b: 'a,
        'c: 'a,
    {
        match *self {
            Reference::Borrowed(borrowed) => borrowed,
            Reference::Copied(copied) => copied,
        }
    }
}
