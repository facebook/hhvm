#![allow(unused_variables, unused_mut)]
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

use hash::IndexMap;
use once_cell::sync::OnceCell;
use std::{
    borrow::Cow,
    ffi::{self, CStr, CString},
    fmt::{self, Debug, Display, Formatter},
    os::unix::ffi::OsStrExt,
    path::Path,
    ptr, result,
};
use thiserror::Error;

pub type Result<E> = result::Result<E, Error>;

#[derive(Debug, Error)]
pub enum Error {
    #[error("{0}")]
    Neo(String),

    #[error("Invalid boolean value '{0}'")]
    BadBool(String),

    #[error("Filename contained an embeded nul")]
    BadNul(#[from] ffi::NulError),

    #[error("I/O Error")]
    IoError(#[from] std::io::Error),

    #[error("'=' expected")]
    MissingEq,

    #[error("Unable to merge into leaf existing node")]
    UnableToMergeLeaf,

    #[error(r#"Missing '"'"#)]
    MissingQuote,

    #[error(r#"Missing ']'"#)]
    MissingBracket,
}

/// Split a name by its first '.'
///   "a" => ("a", None)
///   "a.b.c" => ("a", Some("b.c"))
fn split_name(name: &str) -> (&str, Option<&str>) {
    if let Some(n) = name.find('.') {
        (&name[..n], Some(&name[n + 1..]))
    } else {
        (name, None)
    }
}

/// Value represents a node in our Hdf tree.  The tree has Strings at the leaves and IndexMaps at
/// the inner nodes.
#[derive(Clone)]
pub enum Value {
    Leaf(String),
    NonLeaf(IndexMap<String, Value>),
}

impl Value {
    pub fn new() -> Value {
        Value::NonLeaf(IndexMap::default())
    }

    pub fn empty_map() -> &'static IndexMap<String, Value> {
        static EMPTY: OnceCell<IndexMap<String, Value>> = OnceCell::new();
        EMPTY.get_or_init(Default::default)
    }

    pub fn from_file(filename: &Path) -> Result<Value> {
        let hdf = Hdf::new()?;
        let c_path = CString::new(filename.as_os_str().as_bytes())?;
        convert_err(unsafe { hdf_sys::hdf_read_file(hdf.buf, c_path.as_ptr()) })?;
        Ok(hdf.to_value())
    }

    fn new_from(name: &str, value: String) -> Value {
        let (name, value) = match split_name(name) {
            (name, Some(rest)) => (name, Self::new_from(rest, value)),
            (name, None) => (name, Value::Leaf(value)),
        };

        let mut children = IndexMap::default();
        children.insert(name.to_string(), value);
        Value::NonLeaf(children)
    }

    pub fn from_ini_file(filename: &Path) -> Result<Value> {
        let f = std::fs::read_to_string(filename)?;
        Self::from_ini_string(&f)
    }

    pub fn from_ini_string(input: &str) -> Result<Value> {
        let mut kv = Value::new();

        for line in input.split('\n') {
            match IniLine::parse(line)? {
                IniLine::Empty => {}
                IniLine::Key(_) => {
                    // No value.  Silently ignore.
                }
                IniLine::KeyValue(key, value) => {
                    kv.set(key, value.into());
                }
                IniLine::Section(_) => {
                    // section markers are ignored
                }
            }
        }

        Ok(kv)
    }

    pub fn get(&self, name: &str) -> Option<&Value> {
        match (self, split_name(name)) {
            (Value::Leaf(_), _) => None,
            (Value::NonLeaf(children), (name, Some(value))) => {
                children.get(name).and_then(|v| v.get(value))
            }
            (Value::NonLeaf(children), (name, None)) => children.get(name),
        }
    }

    pub fn get_or_empty(&self, name: &str) -> &Value {
        static EMPTY: OnceCell<Value> = OnceCell::new();
        self.get(name)
            .unwrap_or_else(|| EMPTY.get_or_init(|| Value::NonLeaf(Default::default())))
    }

    pub fn get_str(&self, name: &str) -> Option<&str> {
        self.get(name).and_then(|v| v.as_str())
    }

    /// Gets the value and converts the value to a boolean according to HDF rules.  If the value
    /// exists but isn't a valid boolean returns an Err.
    pub fn get_bool(&self, name: &str) -> Result<Option<bool>> {
        self.get_str(name)
            .map(|v| match v.to_ascii_lowercase().as_str() {
                "true" | "on" | "yes" | "1" => Ok(true),
                "false" | "off" | "no" | "0" => Ok(false),
                _ => Err(Error::BadBool(name.to_owned())),
            })
            .transpose()
    }

    pub fn iter(&self) -> Iter<'_> {
        match self {
            Value::Leaf(_) => Self::empty_map().iter(),
            Value::NonLeaf(map) => map.iter(),
        }
    }

    pub fn keys(&self) -> Keys<'_> {
        match self {
            Value::Leaf(_) => Self::empty_map().keys(),
            Value::NonLeaf(map) => map.keys(),
        }
    }

    pub fn values(&self) -> Values<'_> {
        match self {
            Value::Leaf(_) => Self::empty_map().values(),
            Value::NonLeaf(map) => map.values(),
        }
    }

    pub fn remove(&mut self, name: &str) -> Option<Value> {
        match (self, split_name(name)) {
            (Value::Leaf(_), _) => None,
            (Value::NonLeaf(children), (name, Some(rest))) => {
                children.get_mut(name).and_then(|value| value.remove(rest))
            }
            (Value::NonLeaf(children), (name, None)) => children.remove(name),
        }
    }

    pub fn contains_key(&self, name: &str) -> bool {
        match (self, split_name(name)) {
            (Value::Leaf(_), _) => false,
            (Value::NonLeaf(children), (name, Some(rest))) => {
                children.get(name).map_or(false, |v| v.contains_key(rest))
            }
            (Value::NonLeaf(children), (name, None)) => children.contains_key(name),
        }
    }

    pub fn set(&mut self, name: &str, value: String) {
        use indexmap::map::Entry::*;
        if matches!(self, Value::Leaf(_)) {
            *self = Value::NonLeaf(IndexMap::default());
        }
        match (self, split_name(name)) {
            (Value::Leaf(_), _) => unreachable!(),
            (Value::NonLeaf(children), (name, Some(rest))) => match children.entry(name.into()) {
                Occupied(mut e) => {
                    e.get_mut().set(rest, value);
                }
                Vacant(e) => {
                    e.insert(Value::new_from(rest, value));
                }
            },
            (Value::NonLeaf(children), (name, None)) => {
                children.insert(name.into(), Value::Leaf(value));
            }
        }
    }

    pub fn as_str(&self) -> Option<&str> {
        match self {
            Value::Leaf(s) => Some(s),
            Value::NonLeaf(_) => None,
        }
    }

    pub fn merge(&mut self, other: Value) -> Result<()> {
        match (self, other) {
            (Value::Leaf(_), Value::Leaf(_))
            | (Value::Leaf(_), Value::NonLeaf(_))
            | (Value::NonLeaf(_), Value::Leaf(_)) => {
                return Err(Error::UnableToMergeLeaf);
            }
            (Value::NonLeaf(map), Value::NonLeaf(other_map)) => {
                for (k, v) in other_map.into_iter() {
                    use indexmap::map::Entry::*;
                    match map.entry(k) {
                        Occupied(mut e) => {
                            e.get_mut().merge(v)?;
                        }
                        Vacant(e) => {
                            e.insert(v);
                        }
                    }
                }
            }
        }
        Ok(())
    }
}

impl Debug for Value {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        use fmt::Write;
        match self {
            Value::Leaf(s) => {
                let needs_quote = s.contains(|c| c == ' ' || c == ',' || c == ';' || c == '"');
                if needs_quote {
                    f.write_char('"')?;
                }
                f.write_str(s)?;
                if needs_quote {
                    f.write_char('"')?;
                }
                Ok(())
            }
            Value::NonLeaf(children) => {
                f.write_char('{')?;
                let mut first = true;
                for (k, v) in children.iter() {
                    if !first {
                        f.write_str(", ")?;
                    }
                    first = false;
                    f.write_str(k)?;
                    f.write_str(" = ")?;
                    v.fmt(f)?;
                }
                f.write_char('}')
            }
        }
    }
}

#[derive(Eq, PartialEq, Debug)]
pub enum IniLine<'a> {
    Empty,
    Key(&'a str),
    KeyValue(&'a str, Cow<'a, str>),
    Section(&'a str),
}

impl IniLine<'_> {
    pub fn parse(input: &str) -> Result<IniLine<'_>> {
        // This minimal parser is good enough to handle what we currently need.
        let input = input.trim();
        if input.starts_with('[') {
            // [section]
            let input = input.strip_prefix('[').unwrap();

            // Check for comment.
            let input = input.split_once(';').map_or(input, |(k, _)| k).trim();

            if let Some(section) = input.strip_suffix(']') {
                Ok(IniLine::Section(section))
            } else {
                Err(Error::MissingBracket)
            }
        } else if let Some((key, value)) = input.split_once('=') {
            // key=value
            let key = key.trim_end();
            let value = Self::parse_value(value)?;
            Ok(IniLine::KeyValue(key, value))
        } else {
            // No '=' so no value.  Check for a comment too.
            let key = input.split_once(';').map_or(input, |(k, _)| k).trim();
            if key.is_empty() {
                Ok(IniLine::Empty)
            } else {
                Ok(IniLine::Key(key))
            }
        }
    }

    pub(crate) fn parse_value(value: &str) -> Result<Cow<'_, str>> {
        let value = value.trim_start();

        // Check for double-quoted string.
        if !value.contains('"') {
            // Not double-quoted. Check for comment.
            let value = value.split_once(';').map_or(value, |(k, _)| k).trim();
            return Ok(value.into());
        }

        let mut out = String::new();
        let mut in_quote = false;
        let mut prev_escape = false;
        // We use trailing_junk to figure out extra whitespace that appeared at
        // the end of a line outside of a double-quoted string.
        let mut trailing_junk = 0;
        for c in value.chars() {
            match c {
                ';' if !in_quote => {
                    // This starts a comment - ignore the rest of the
                    // line.
                    break;
                }
                '\\' if in_quote => {
                    if prev_escape {
                        out.push('\\');
                    }
                }
                '"' => {
                    if prev_escape {
                        // This is an escaped quote.
                        out.push('"');
                    } else {
                        in_quote = !in_quote;
                        trailing_junk = 0;
                    }
                }
                _ => {
                    out.push(c);
                    if !in_quote && c.is_whitespace() {
                        trailing_junk += 1;
                    } else {
                        trailing_junk = 0;
                    }
                }
            }
            prev_escape = !prev_escape && (c == '\\');
        }

        out.truncate(out.len() - trailing_junk);
        Ok(out.into())
    }
}

#[test]
fn test_parse_value() {
    fn chk(a: &str, b: &str) {
        assert_eq!(
            IniLine::parse_value(a).map(|s| s.into_owned()).ok(),
            Some(b.into())
        );
    }

    chk("xyzzy", "xyzzy");
    chk("xyzzy ; this is a comment", "xyzzy");
    chk(r#" "xyzzy" ; this is a comment"#, "xyzzy");
    chk(r#""xyz\"zy""#, "xyz\"zy");
    chk(r#""xyzzy " ; this is a comment"#, "xyzzy ");
    chk(r#""xyz;zy""#, "xyz;zy");
}

/// Lifetime management for neo_hdf STRING.
struct NeoString {
    buf: hdf_sys::_string,
}

impl NeoString {
    fn new() -> Self {
        NeoString {
            // This is what string_init() does - but since we have to provide an initialized struct
            // to Rust it's kind of silly to do it twice.
            buf: hdf_sys::_string {
                buf: ptr::null_mut(),
                len: 0,
                max: 0,
            },
        }
    }

    fn raw_ptr(&mut self) -> &mut hdf_sys::_string {
        &mut self.buf
    }

    fn to_str_lossy(&self) -> Cow<'_, str> {
        let slice =
            unsafe { std::slice::from_raw_parts(self.buf.buf as *mut u8, self.buf.len as usize) };
        String::from_utf8_lossy(slice)
    }
}

impl Debug for NeoString {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        Debug::fmt(&NeoString::to_str_lossy(self), f)
    }
}

impl Display for NeoString {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        Display::fmt(&NeoString::to_str_lossy(self), f)
    }
}

impl Drop for NeoString {
    fn drop(&mut self) {
        unsafe {
            hdf_sys::string_clear(self.raw_ptr());
        }
    }
}

/// Convert a NEOERR to an hdf::Error and free the NEOERR.
fn convert_err(err: *mut hdf_sys::NEOERR) -> Result<()> {
    if err.is_null() {
        return Ok(());
    }

    let mut string = NeoString::new();
    unsafe {
        hdf_sys::nerr_error_string(err, string.raw_ptr());
    }

    unsafe fn _err_free(err: *mut hdf_sys::NEOERR) {
        let p = err as *mut hdf_sys::NEOERR as isize;
        if p == 0 || p == -1 {
            return;
        }
        if !(*err).next.is_null() {
            _err_free((*err).next);
        }
        libc::free(err as *mut libc::c_void);
    }

    unsafe {
        // This actually frees the error chain.
        _err_free(err);
    }
    Err(Error::Neo(string.to_str_lossy().to_string()))
}

/// Lifetime management for neo_hdf HDF objects.
struct Hdf {
    buf: *mut hdf_sys::HDF,
}

impl Hdf {
    fn new() -> Result<Hdf> {
        let mut buf = ptr::null_mut();
        convert_err(unsafe { hdf_sys::hdf_init(&mut buf) })?;
        Ok(Hdf { buf })
    }

    fn raw_ptr(&mut self) -> &mut *mut hdf_sys::HDF {
        &mut self.buf
    }

    fn obj_child(&self) -> Option<Hdf> {
        let mut err = ptr::null_mut();
        let buf = unsafe { hdf_sys::hdf_obj_child(self.buf, &mut err) };
        if buf.is_null() {
            None
        } else {
            Some(Hdf { buf })
        }
    }

    fn obj_next(&self) -> Option<Hdf> {
        let buf = unsafe { hdf_sys::hdf_obj_next(self.buf) };
        if buf.is_null() {
            None
        } else {
            Some(Hdf { buf })
        }
    }

    fn obj_name(&self) -> String {
        let name = unsafe { hdf_sys::hdf_obj_name(self.buf) };
        if name.is_null() {
            String::new()
        } else {
            let name = unsafe { CStr::from_ptr(name) };
            name.to_string_lossy().into_owned()
        }
    }

    fn obj_value(&self) -> Option<String> {
        let mut err = ptr::null_mut();
        let value = unsafe { hdf_sys::hdf_obj_value(self.buf, &mut err) };
        if value.is_null() {
            None
        } else {
            let name = unsafe { CStr::from_ptr(value) };
            Some(name.to_string_lossy().into_owned())
        }
    }

    fn to_value(&self) -> Value {
        if let Some(value) = self.obj_value() {
            Value::Leaf(value)
        } else {
            let mut children = IndexMap::default();

            let mut maybe_child = self.obj_child();
            while let Some(child) = maybe_child {
                let name = child.obj_name();
                children.insert(name, child.to_value());
                maybe_child = child.obj_next();
            }

            Value::NonLeaf(children)
        }
    }
}

pub type Iter<'a> = indexmap::map::Iter<'a, String, Value>;
pub type Keys<'a> = indexmap::map::Keys<'a, String, Value>;
pub type Values<'a> = indexmap::map::Values<'a, String, Value>;

impl Drop for Hdf {
    fn drop(&mut self) {
        unsafe {
            hdf_sys::hdf_destroy(self.raw_ptr());
        }
    }
}

#[test]
fn test_value() {
    let mut hdf = Value::new();

    hdf.set("a.b.c", "d".into());
    assert_eq!(format!("{:?}", hdf), "{a = {b = {c = d}}}");
    assert_eq!(hdf.get_str("a"), None);
    assert_eq!(hdf.get_str("a.b"), None);
    assert_eq!(hdf.get_str("a.b.c"), Some("d"));
    assert_eq!(hdf.get_str("e"), None);

    hdf.set("q", "h".into());
    hdf.set("a.q", "g".into());
    hdf.set("a.c.q", "f".into());
    assert_eq!(
        format!("{:?}", hdf),
        "{a = {b = {c = d}, q = g, c = {q = f}}, q = h}"
    );
}

#[test]
fn test_ini() {
    let a = Value::from_ini_string("a.b.c=d").expect("expected to parse");
    assert_eq!(format!("{:?}", a), "{a = {b = {c = d}}}");

    let a =
        Value::from_ini_string("hhvm.php7.all\nhhvm.php7.all=false").expect("expected to parse");
    assert_eq!(format!("{:?}", a), "{hhvm = {php7 = {all = false}}}");
}

#[test]
fn test_merge() {
    let mut a = Value::from_ini_string("a.b.c=d").expect("expected to parse");
    let b = Value::from_ini_string("a.b.e=f").expect("expected to parse");
    a.merge(b).unwrap();
    assert_eq!(format!("{:?}", a), "{a = {b = {c = d, e = f}}}");
}

#[test]
fn test_ini_comment() {
    let a = Value::from_ini_string(
        "
; this is a comment
a.b.c=d ; this is also a comment",
    )
    .expect("expected to parse");
    assert_eq!(format!("{:?}", a), "{a = {b = {c = d}}}");
}

#[test]
fn test_ini_section() {
    let a = Value::from_ini_string(
        "
[php] ; section markers are ignored
a.b.c=d",
    )
    .expect("expected to parse");
    assert_eq!(format!("{:?}", a), "{a = {b = {c = d}}}");
}

#[test]
fn test_ini_quotes() {
    let a = Value::from_ini_string(
        r#"
a.b.c="d e"
"#,
    )
    .expect("expected to parse");
    assert_eq!(format!("{:?}", a), r#"{a = {b = {c = "d e"}}}"#);
    assert_eq!(a.get_str("a.b.c"), Some("d e"));

    let a = Value::from_ini_string(
        r#"
a.b.c="d;e"
"#,
    )
    .expect("expected to parse");
    assert_eq!(format!("{:?}", a), r#"{a = {b = {c = "d;e"}}}"#);
}

#[test]
fn test_ini_line() {
    assert_eq!(
        IniLine::parse("a=b").ok(),
        Some(IniLine::KeyValue("a", "b".into()))
    );
}
