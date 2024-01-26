// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::borrow::Cow;
use std::ffi::CStr;
use std::os::unix::ffi::OsStrExt;
use std::path::Path;

use cxx::let_cxx_string;
use cxx::UniquePtr;
use thiserror::Error;

use crate::hdf::ffi;

pub type Result<T, E = Error> = std::result::Result<T, E>;

#[derive(Debug, Error)]
pub enum Error {
    #[error("Missing ']'")]
    MissingBracket,

    #[error(transparent)]
    Exn(#[from] cxx::Exception),

    #[error(transparent)]
    Io(#[from] std::io::Error),

    #[error(transparent)]
    Utf8(#[from] std::str::Utf8Error),
}

/// Value represents a node in our Hdf tree. This is a wrapper around
/// the C++ HPHP::Hdf class defined in hphp/util/hdf.h. A Value represents
/// a node in an HDF configuration tree. The underlying state for nodes
/// is represented by a single-threaded refcounted HdfRaw node, that
/// suports inner mutability.
///
/// This Rust API uses `&mut self` for operations that directly mutate a node,
/// but because of the refcounted nature of node ownership, &mut self does not
/// provide a guarantee that child (or parent) nodes are exclusively owned.
pub struct Value {
    inner: UniquePtr<ffi::Hdf>,
}

impl Default for Value {
    fn default() -> Self {
        Self {
            inner: ffi::hdf_new(),
        }
    }
}

/// Impl Debug using the underlying C++ Hdf pretty printer.
impl std::fmt::Debug for Value {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self.inner.toString().map_err(|_| std::fmt::Error)? {
            cstr if cstr.is_null() => f.write_str("{null}"),
            cstr => f.write_str(&unsafe { CStr::from_ptr(cstr) }.to_string_lossy()),
        }
    }
}

impl Value {
    /// Construct an HDF value from the given hdf-format file.
    pub fn from_file(filename: &Path) -> Result<Value> {
        let mut v = Value::default();
        let_cxx_string!(cxx_filename = filename.as_os_str().as_bytes());
        v.inner.pin_mut().append(&cxx_filename)?;
        Ok(v)
    }

    /// Construct an HDF value from the given INI file.
    pub fn from_ini_file(filename: &Path) -> Result<Value> {
        use std::fs::File;
        use std::io::BufRead;
        use std::io::BufReader;
        let input = BufReader::new(File::open(filename)?);
        let mut kv = Value::default();
        for line in input.lines() {
            kv.set_ini(&line?)?;
        }
        Ok(kv)
    }

    /// Construct an HDF value from the given INI string.
    pub fn from_ini_string(input: &str) -> Result<Value> {
        let mut kv = Value::default();
        for line in input.lines() {
            kv.set_ini(line)?;
        }
        Ok(kv)
    }

    /// Set one HDF value from the given INI string.
    pub fn set_ini(&mut self, line: &str) -> Result<()> {
        if let Err(e) = self.try_set_ini(line) {
            eprintln!("Warning: unable to convert INI to Hdf: {}: {}", line, e);
        }
        Ok(())
    }

    fn try_set_ini(&mut self, line: &str) -> Result<()> {
        // This should match HPHP::Config::ParseIniString
        match IniLine::parse(line)? {
            IniLine::Empty => {}
            IniLine::Key(key) => {
                self.set_hdf(&format!("{} =", key))?;
            }
            IniLine::KeyValue(key, value) => {
                self.set_hdf(&format!("{} = {}", key, value))?;
            }
            IniLine::Section(_) => {
                // section markers are ignored
            }
        }
        Ok(())
    }

    /// Set this node's value and/or subvalues from the HDF-format `input` str.
    pub fn set_hdf(&mut self, input: &str) -> Result<()> {
        let_cxx_string!(cxx_input = input);
        Ok(self.inner.pin_mut().fromString(&cxx_input)?)
    }

    /// Return a Value representing the referenced node, if it exists,
    /// or an Error in case of an internal Hdf format error.
    pub fn get(&self, name: &str) -> Result<Option<Value>> {
        let_cxx_string!(name = name);
        let inner = ffi::hdf_new_child(&self.inner, &name);
        match inner.exists()? {
            true => Ok(Some(Self { inner })),
            false => Ok(None),
        }
    }

    /// Gets the value and converts the value to a boolean according to HDF rules.
    /// If the value doesn't exist, return Ok<None>.
    pub fn get_bool(&self, name: &str) -> Result<Option<bool>> {
        match self.get(name)? {
            Some(v) => Ok(Some(v.inner.configGetBool(false)?)),
            None => Ok(None),
        }
    }

    /// Gets the value and converts the value to a boolean according to HDF rules.
    /// If the value doesn't exist, return the default value.
    pub fn get_bool_or(&self, name: &str, default: bool) -> Result<bool> {
        match self.get(name)? {
            Some(v) => Ok(v.inner.configGetBool(default)?),
            None => Ok(default),
        }
    }

    /// Gets the value and converts the value to a uint32 according to HDF rules.
    /// If the value doesn't exist, return the default value.
    pub fn get_uint32(&self, name: &str) -> Result<Option<u32>> {
        match self.get(name)? {
            Some(v) => Ok(Some(v.inner.configGetUInt32(0)?)),
            None => Ok(None),
        }
    }

    /// Gets the value and converts the value to a i64 according to HDF rules.
    /// If the value doesn't exist, return the default value.
    pub fn get_int64_or(&self, name: &str, or_default: i64) -> Result<i64> {
        match self.get(name)? {
            Some(v) => Ok(v.inner.configGetInt64(or_default)?),
            None => Ok(or_default),
        }
    }

    /// Return the utf8 string value of this node, if it exists.
    /// Returns an error for Hdf internal errors or failed utf8 validation.
    pub fn as_str(&self) -> Result<Option<String>> {
        match unsafe { self.inner.configGet(std::ptr::null()) }? {
            cstr if cstr.is_null() => Ok(None),
            cstr => Ok(Some(unsafe { CStr::from_ptr(cstr) }.to_str()?.into())),
        }
    }

    /// Lookup the node with the given name.
    /// If it exists, return it's string value, otherwise return None.
    /// Fails on internal Hdf parsing errors or Utf8 validation checks.
    pub fn get_str(&self, name: &str) -> Result<Option<String>> {
        match self.get(name)? {
            Some(v) => v.as_str(),
            None => Ok(None),
        }
    }

    /// Return this node's name.
    /// Fails on internal Hdf parsing errors or Utf8 validation checks.
    pub fn name(&self) -> Result<String> {
        Ok(ffi::hdf_name(&self.inner)?)
    }

    /// Return this node's name as read by the config, this detects
    /// when a wildcard name exists and returns '*' instead of the internal
    /// numeric index.
    /// Fails on internal Hdf parsing errors or Utf8 validation checks.
    pub fn name_as_read(&self) -> Result<String> {
        if self.inner.isWildcardName()? {
            return Ok("*".into());
        }
        Ok(ffi::hdf_name(&self.inner)?)
    }

    /// Convert self to an iterator over children, if possible.
    /// Fails on internal Hdf parsing errors.
    pub fn into_children(self) -> Result<Children> {
        Ok(Children {
            next: ffi::hdf_first_child(&self.inner)?,
        })
    }

    pub fn get_child_names(&self) -> Result<Vec<String>> {
        match ffi::hdf_child_names(&self.inner) {
            Ok(names) => Ok(names),
            Err(e) => Err(e.into()),
        }
    }

    /// Return the string values of child nodes.
    /// Fails on internal Hdf parsing errors or Utf8 validation checks.
    pub fn values(self) -> Result<Vec<String>> {
        self.into_children()?
            .map(|v| Ok(v?.as_str()?.unwrap_or_default()))
            .collect::<Result<_>>()
    }

    /// Return whether a node with the given name exists.
    /// Fails on internal Hdf parsing errors.
    pub fn contains_key(&self, name: &str) -> Result<bool> {
        match self.get(name)? {
            Some(v) => Ok(v.inner.exists()?),
            None => Ok(false),
        }
    }

    /// Delete the node with the given name, if it exists.
    /// Does nothing if the node does not exist.
    /// Fails on internal Hdf errors.
    pub fn remove(&mut self, name: &str) -> Result<()> {
        let_cxx_string!(name = name);
        Ok(self.inner.remove(&name)?)
    }

    pub fn copy(&mut self, src: &Value) -> Result<()> {
        Ok(self.inner.pin_mut().copy(&src.inner)?)
    }
}

pub struct Children {
    next: UniquePtr<ffi::Hdf>,
}

impl Iterator for Children {
    type Item = Result<Value>;
    fn next(&mut self) -> Option<Self::Item> {
        match self.next.exists() {
            Ok(false) => None,
            Ok(true) => match ffi::hdf_next(&self.next) {
                Ok(next) => Some(Ok(Value {
                    inner: std::mem::replace(&mut self.next, next),
                })),
                Err(e) => Some(Err(e.into())),
            },
            Err(e) => Some(Err(e.into())),
        }
    }
}

#[derive(Eq, PartialEq, Debug)]
enum IniLine<'a> {
    Empty,
    Key(&'a str),
    KeyValue(&'a str, Cow<'a, str>),
    Section(&'a str),
}

impl IniLine<'_> {
    fn parse(input: &str) -> Result<IniLine<'_>> {
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

    fn parse_value(value: &str) -> Result<Cow<'_, str>> {
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

#[cfg(test)]
mod test {
    use super::*;

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

    #[test]
    fn test_value() {
        let mut hdf = Value::default();

        hdf.set_hdf("a.b.c=d").unwrap();
        assert_eq!(format!("{:?}", hdf), "a {\n  b {\n    c = d\n  }\n}\n");
        assert_eq!(hdf.get_str("a").unwrap(), None);
        assert_eq!(hdf.get_str("a.b").unwrap(), None);
        assert_eq!(hdf.get_str("a.b.c").unwrap(), Some("d".into()));
        assert_eq!(hdf.get_str("e").unwrap(), None);
        assert!(hdf.get("x").unwrap().is_none());
        assert!(hdf.get("x.x").unwrap().is_none());

        hdf.set_hdf("q=h").unwrap();
        hdf.set_hdf("a.q=g").unwrap();
        hdf.set_hdf("a.c.q=f").unwrap();
        assert_eq!(
            format!("{:?}", hdf),
            "a {\n  b {\n    c = d\n  }\n  q = g\n  c {\n    q = f\n  }\n}\nq = h\n"
        );
    }

    #[test]
    fn test_copy() -> Result<()> {
        let mut hdf = Value::default();
        let mut hdf2 = Value::default();

        hdf.set_hdf("Eval.bool = True")?;
        hdf.set_hdf("Eval.text = foo")?;
        hdf2.set_hdf("Override.Eval.bool = False")?;
        hdf2.set_hdf("Override.Eval.bool2 = True")?;
        hdf2.set_hdf("Override.Eval.text = bar")?;
        hdf.copy(&hdf2.get("Override")?.unwrap())?;
        assert_eq!(hdf.get_bool("Eval.bool")?, Some(false));
        assert_eq!(hdf.get_bool("Eval.bool2")?, Some(true));
        assert_eq!(hdf.get_str("Eval.text")?, Some("bar".into()));
        assert_eq!(
            format!("{:?}", hdf),
            "Eval {\n  bool = False\n  text = bar\n  bool2 = True\n}\n"
        );

        Ok(())
    }

    #[test]
    fn test_ini() {
        let a = Value::from_ini_string("a.b.c=d").expect("expected to parse");
        assert_eq!(format!("{:?}", a), "a {\n  b {\n    c = d\n  }\n}\n");

        let a = Value::from_ini_string("hhvm.php7.all\nhhvm.php7.all=false")
            .expect("expected to parse");
        assert_eq!(
            format!("{:?}", a),
            "hhvm {\n  php7 {\n    all = false\n  }\n}\n"
        );
    }

    #[test]
    fn test_ini_comment() {
        let a = Value::from_ini_string(
            "
; this is a comment
a.b.c=d ; this is also a comment",
        )
        .expect("expected to parse");
        assert_eq!(format!("{:?}", a), "a {\n  b {\n    c = d\n  }\n}\n");
    }

    #[test]
    fn test_ini_section() {
        let a = Value::from_ini_string(
            "
[php] ; section markers are ignored
a.b.c=d",
        )
        .expect("expected to parse");
        assert_eq!(format!("{:?}", a), "a {\n  b {\n    c = d\n  }\n}\n");
    }

    #[test]
    fn test_ini_quotes() {
        let a = Value::from_ini_string(
            r#"
a.b.c="d e"
"#,
        )
        .expect("expected to parse");
        assert_eq!(format!("{:?}", a), "a {\n  b {\n    c = d e\n  }\n}\n");
        assert_eq!(a.get_str("a.b.c").unwrap().as_deref(), Some("d e"));

        let a = Value::from_ini_string(
            r#"
a.b.c="d;e"
"#,
        )
        .expect("expected to parse");
        assert_eq!(format!("{:?}", a), "a {\n  b {\n    c = d;e\n  }\n}\n");
    }

    #[test]
    fn test_ini_line() {
        assert_eq!(
            IniLine::parse("a=b").ok(),
            Some(IniLine::KeyValue("a", "b".into()))
        );
    }

    #[test]
    fn test_wildcard_names() -> Result<()> {
        let mut hdf = Value::default();
        hdf.set_hdf(
            r#"
            foo {
                * => zero
                * => one
                # This is a dangerous config because if next after this if * => three
                # occurs foo.2 will be replaced with three
                2 => two
            }
            bar.* => two
            bar.* => three
            bar.4 => four
        }
        "#,
        )?;

        let node = hdf.get("foo")?.unwrap();
        assert_eq!(node.name()?, "foo");
        assert_eq!(node.name_as_read()?, "foo");

        let node = hdf.get("foo.0")?.unwrap();
        assert_eq!(node.name()?, "0");
        assert_eq!(node.name_as_read()?, "*");
        let node = hdf.get("foo.1")?.unwrap();
        assert_eq!(node.name()?, "1");
        assert_eq!(node.name_as_read()?, "*");
        let node = hdf.get("foo.2")?.unwrap();
        assert_eq!(node.name()?, "2");
        assert_eq!(node.name_as_read()?, "2");

        let node = hdf.get("bar")?.unwrap();
        assert_eq!(node.name()?, "bar");
        assert_eq!(node.name_as_read()?, "bar");

        let node = hdf.get("bar.2")?.unwrap();
        assert_eq!(node.name()?, "2");
        assert_eq!(node.name_as_read()?, "*");
        let node = hdf.get("bar.3")?.unwrap();
        assert_eq!(node.name()?, "3");
        assert_eq!(node.name_as_read()?, "*");
        let node = hdf.get("bar.4")?.unwrap();
        assert_eq!(node.name()?, "4");
        assert_eq!(node.name_as_read()?, "4");

        Ok(())
    }
}
