// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::fmt::Debug;
use std::io;
use std::io::Result;
use std::io::Write;

use thiserror::Error;

#[derive(Error, Debug)]
pub enum Error {
    #[error("write error: {0:?}")]
    WriteError(anyhow::Error),

    #[error("a string may contain invalid utf-8")]
    InvalidUTF8,

    //TODO(hrust): This is a temp error during porting
    #[error("NOT_IMPL: {0}")]
    NotImpl(String),

    #[error("Failed: {0}")]
    Fail(String),
}

impl Error {
    pub fn fail(s: impl Into<String>) -> Self {
        Self::Fail(s.into())
    }
}

pub(crate) fn into_error(e: io::Error) -> Error {
    if e.kind() == io::ErrorKind::Other && e.get_ref().map_or(false, |e| e.is::<Error>()) {
        let err: Error = *e.into_inner().unwrap().downcast::<Error>().unwrap();
        return err;
    }
    Error::WriteError(e.into())
}

impl From<Error> for std::io::Error {
    fn from(e: Error) -> Self {
        io::Error::new(io::ErrorKind::Other, e)
    }
}

pub(crate) fn wrap_by_<F>(w: &mut dyn Write, s: &str, e: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut dyn Write) -> Result<()>,
{
    w.write_all(s.as_bytes())?;
    f(w)?;
    w.write_all(e.as_bytes())
}

pub(crate) fn wrap_by<F>(w: &mut dyn Write, s: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut dyn Write) -> Result<()>,
{
    wrap_by_(w, s, s, f)
}

macro_rules! wrap_by {
    ($name:ident, $left:expr, $right:expr) => {
        pub(crate) fn $name<F>(w: &mut dyn Write, f: F) -> Result<()>
        where
            F: FnOnce(&mut dyn Write) -> Result<()>,
        {
            $crate::write::wrap_by_(w, $left, $right, f)
        }
    };
}

wrap_by!(paren, "(", ")");
wrap_by!(square, "[", "]");

pub(crate) fn concat_str_by<I: AsRef<str>>(
    w: &mut dyn Write,
    sep: impl AsRef<str>,
    ss: impl AsRef<[I]>,
) -> Result<()> {
    concat_by(w, sep, ss, |w, s| {
        w.write_all(s.as_ref().as_bytes())?;
        Ok(())
    })
}

pub(crate) fn concat<T, F>(w: &mut dyn Write, items: impl AsRef<[T]>, f: F) -> Result<()>
where
    F: FnMut(&mut dyn Write, &T) -> Result<()>,
{
    concat_by(w, "", items, f)
}

pub(crate) fn concat_by<T, F>(
    w: &mut dyn Write,
    sep: impl AsRef<str>,
    items: impl AsRef<[T]>,
    mut f: F,
) -> Result<()>
where
    F: FnMut(&mut dyn Write, &T) -> Result<()>,
{
    let mut first = true;
    let sep = sep.as_ref();
    Ok(for i in items.as_ref() {
        if first {
            first = false;
        } else {
            w.write_all(sep.as_bytes())?;
        }
        f(w, i)?;
    })
}

pub(crate) fn option<T, F>(w: &mut dyn Write, i: impl Into<Option<T>>, mut f: F) -> Result<()>
where
    F: FnMut(&mut dyn Write, T) -> Result<()>,
{
    match i.into() {
        None => Ok(()),
        Some(i) => f(w, i),
    }
}
