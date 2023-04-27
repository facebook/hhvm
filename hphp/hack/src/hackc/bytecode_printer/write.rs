// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::cell::Cell;
use std::fmt::Debug;
use std::io;
use std::io::Result;
use std::io::Write;

use thiserror::Error;
use write_bytes::BytesFormatter;
use write_bytes::DisplayBytes;

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

pub(crate) fn get_embedded_error(e: &io::Error) -> Option<&Error> {
    if e.kind() == io::ErrorKind::Other {
        if let Some(e) = e.get_ref() {
            if e.is::<Error>() {
                let err = e.downcast_ref::<Error>();
                return err;
            }
        }
    }
    None
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

pub(crate) fn newline(w: &mut dyn Write) -> Result<()> {
    w.write_all(b"\n")?;
    Ok(())
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

wrap_by!(braces, "{", "}");
wrap_by!(paren, "(", ")");
wrap_by!(quotes, "\"", "\"");
wrap_by!(triple_quotes, "\"\"\"", "\"\"\"");
wrap_by!(angle, "<", ">");
wrap_by!(square, "[", "]");

pub(crate) fn concat_str<I: AsRef<str>>(w: &mut dyn Write, ss: impl AsRef<[I]>) -> Result<()> {
    concat(w, ss, |w, s| {
        w.write_all(s.as_ref().as_bytes())?;
        Ok(())
    })
}

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

pub(crate) fn option_or<T, F>(
    w: &mut dyn Write,
    i: impl Into<Option<T>>,
    f: F,
    default: impl AsRef<str>,
) -> Result<()>
where
    F: Fn(&mut dyn Write, T) -> Result<()>,
{
    match i.into() {
        None => w.write_all(default.as_ref().as_bytes()),
        Some(i) => f(w, i),
    }
}

pub(crate) struct FmtSeparated<'a, T, I>
where
    T: DisplayBytes,
    I: Iterator<Item = T>,
{
    items: Cell<Option<I>>,
    sep: &'a str,
}

impl<'a, Item, I> DisplayBytes for FmtSeparated<'a, Item, I>
where
    Item: DisplayBytes,
    I: Iterator<Item = Item>,
{
    fn fmt(&self, f: &mut BytesFormatter<'_>) -> Result<()> {
        let mut items = self.items.take().unwrap();
        if let Some(item) = items.next() {
            item.fmt(f)?;
        }
        for item in items {
            f.write_all(self.sep.as_bytes())?;
            item.fmt(f)?;
        }
        Ok(())
    }
}

pub(crate) fn fmt_separated<'a, Item, I, II>(sep: &'a str, items: II) -> FmtSeparated<'a, Item, I>
where
    Item: DisplayBytes,
    I: Iterator<Item = Item>,
    II: IntoIterator<Item = Item, IntoIter = I>,
{
    FmtSeparated {
        items: Cell::new(Some(items.into_iter())),
        sep,
    }
}

pub(crate) struct FmtSeparatedWith<'a, Item, I, F>
where
    I: Iterator<Item = Item>,
    F: Fn(&mut BytesFormatter<'_>, Item) -> Result<()>,
{
    items: Cell<Option<I>>,
    sep: &'a str,
    with: F,
}

impl<'a, Item, I, F> DisplayBytes for FmtSeparatedWith<'a, Item, I, F>
where
    I: Iterator<Item = Item>,
    F: Fn(&mut BytesFormatter<'_>, Item) -> Result<()>,
{
    fn fmt(&self, f: &mut BytesFormatter<'_>) -> Result<()> {
        let mut items = self.items.take().unwrap();
        if let Some(item) = items.next() {
            (self.with)(f, item)?;
        }
        for item in items {
            f.write_all(self.sep.as_bytes())?;
            (self.with)(f, item)?;
        }
        Ok(())
    }
}

pub(crate) fn fmt_separated_with<'a, Item, I, F, II>(
    sep: &'a str,
    items: II,
    with: F,
) -> FmtSeparatedWith<'a, Item, I, F>
where
    I: Iterator<Item = Item>,
    F: Fn(&mut BytesFormatter<'_>, Item) -> Result<()>,
    II: IntoIterator<Item = Item, IntoIter = I>,
{
    FmtSeparatedWith {
        items: Cell::new(Some(items.into_iter())),
        sep,
        with,
    }
}

#[test]
fn test_fmt_separated() -> Result<()> {
    use bstr::BStr;
    use write_bytes::format_bytes;
    use write_bytes::write_bytes;

    let v: Vec<&str> = vec!["a", "b"];
    assert_eq!(
        format_bytes!("{}", fmt_separated(" ", v)),
        <&BStr>::from("a b")
    );

    let v: Vec<&BStr> = vec!["a".into(), "b".into()];
    assert_eq!(
        format_bytes!("{}", fmt_separated(" ", v)),
        <&BStr>::from("a b")
    );

    let v1: Vec<&BStr> = vec!["a".into(), "b".into()];
    let v2: Vec<i64> = vec![1, 2];
    let it = v1
        .iter()
        .map(|i: &&BStr| -> &dyn DisplayBytes { i })
        .chain(v2.iter().map(|i: &i64| -> &dyn DisplayBytes { i }));

    assert_eq!(
        format_bytes!("{}", fmt_separated(" ", it),),
        <&BStr>::from("a b 1 2")
    );

    Ok(())
}
