// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(unused_variables)]
#![allow(dead_code)]

use std::fmt;

pub type Result<T, WE> = std::result::Result<T, Error<WE>>;

#[derive(Debug)]
pub enum Error<WE> {
    WriteError(WE),
}

pub trait Write {
    type Error;
    fn write(&mut self, s: &str) -> Result<(), Self::Error>;
}

impl<W: fmt::Write> Write for W {
    type Error = fmt::Error;
    fn write(&mut self, s: &str) -> Result<(), Self::Error> {
        self.write_str(s).map_err(Error::WriteError)
    }
}

pub fn newline<W: Write>(w: &mut W) -> Result<(), W::Error> {
    w.write("\n")
}

pub fn wrap_by_<W: Write, F>(w: &mut W, s: &str, e: &str, f: F) -> Result<(), W::Error>
where
    F: FnOnce(&mut W) -> Result<(), W::Error>,
{
    w.write(s)?;
    f(w)?;
    w.write(e)
}

pub fn wrap_by<W: Write, F>(w: &mut W, s: &str, f: F) -> Result<(), W::Error>
where
    F: FnOnce(&mut W) -> Result<(), W::Error>,
{
    wrap_by_(w, s, s, f)
}

pub fn wrap_by_braces<W: Write, F>(w: &mut W, f: F) -> Result<(), W::Error>
where
    F: FnOnce(&mut W) -> Result<(), W::Error>,
{
    wrap_by_(w, "{", "}", f)
}

pub fn write_list<W: Write>(w: &mut W, items: &[impl AsRef<str>]) -> Result<(), W::Error> {
    Ok(for i in items {
        w.write(i.as_ref())?;
    })
}

pub fn write_list_by<W: Write>(
    w: &mut W,
    sep: impl AsRef<str>,
    items: &[impl AsRef<str>],
) -> Result<(), W::Error> {
    let mut first = true;
    let sep = sep.as_ref();
    Ok(for i in items {
        if first {
            first = false;
        } else {
            w.write(sep)?;
        }
        w.write(i.as_ref())?;
    })
}

pub fn write_map<'i, W: Write, Item: 'i, F>(
    w: &mut W,
    f: F,
    items: impl Iterator<Item = &'i Item>,
) -> Result<(), W::Error>
where
    F: Fn(&mut W, &Item) -> Result<(), W::Error>,
{
    Ok(for i in items {
        f(w, i)?
    })
}
