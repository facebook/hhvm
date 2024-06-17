/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::ffi::OsString;
use std::fmt::Write;
use std::path::PathBuf;

use serde::Deserialize;
use serde::Serialize;

/// The ByteString type represents values encoded using BSER_BYTESTRING.
/// The purpose of this encoding is to represent bytestrings with an arbitrary
/// encoding.
///
/// In practice, as used by watchman, bytestrings have the filesystem encoding
/// on posix systems (which is usually utf8 on linux, guaranteed utf8 on macos)
/// and when they appear as file names in file results are guaranteed to be utf8
/// on Windows systems.
///
/// When it comes to interoperating with Rust code, ByteString is nominally
/// equivalent to `OsString`/`PathBuf` and is convertible to and from those values.
///
/// It is worth noting that on Windows sytems the conversion from ByteString to OsString is
/// potentially lossy: while convention is that bytestring holds utf8 in the common case in
/// watchman, that isn't enforced by its serializer and we may potentially encounter an error
/// during conversion.  Converting from OsString to ByteString can potentially fail on windows
/// because Watchman and thus ByteString doesn't have a way to represent the poorly formed
/// surrogate pairs that Windows allows in its filenames.

#[derive(Serialize, Deserialize, Clone, PartialEq, Eq)]
#[serde(transparent)]
pub struct ByteString(#[serde(with = "serde_bytes")] Vec<u8>);

impl std::fmt::Debug for ByteString {
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> Result<(), std::fmt::Error> {
        let escaped = self.as_escaped_string();
        write!(fmt, "\"{}\"", escaped.escape_debug())
    }
}

impl std::fmt::Display for ByteString {
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> Result<(), std::fmt::Error> {
        let escaped = self.as_escaped_string();
        write!(fmt, "\"{}\"", escaped.escape_default())
    }
}

impl std::ops::Deref for ByteString {
    type Target = [u8];

    fn deref(&self) -> &[u8] {
        &self.0
    }
}

impl std::ops::DerefMut for ByteString {
    fn deref_mut(&mut self) -> &mut [u8] {
        &mut self.0
    }
}

impl ByteString {
    /// Returns the raw bytes as a slice.
    pub fn as_bytes(&self) -> &[u8] {
        self.0.as_slice()
    }

    /// Consumes ByteString yielding bytes.
    pub fn into_bytes(self) -> Vec<u8> {
        self.0
    }

    /// Returns a version of the bytestring encoded as a mostly-utf-8
    /// string, with invalid sequences escaped using `\xXX` hex notation.
    /// This is for diagnostic and display purposes.
    pub fn as_escaped_string(&self) -> String {
        let mut input = self.0.as_slice();
        let mut output = String::new();

        loop {
            match ::std::str::from_utf8(input) {
                Ok(valid) => {
                    output.push_str(valid);
                    break;
                }
                Err(error) => {
                    let (valid, after_valid) = input.split_at(error.valid_up_to());
                    unsafe { output.push_str(::std::str::from_utf8_unchecked(valid)) }

                    if let Some(invalid_sequence_length) = error.error_len() {
                        for b in &after_valid[..invalid_sequence_length] {
                            write!(output, "\\x{:x}", b).unwrap();
                        }
                        input = &after_valid[invalid_sequence_length..];
                    } else {
                        break;
                    }
                }
            }
        }

        output
    }
}

/// Guaranteed conversion from an owned byte vector to a ByteString
impl From<Vec<u8>> for ByteString {
    fn from(vec: Vec<u8>) -> Self {
        Self(vec)
    }
}

/// Guaranteed conversion from a UTF-8 string to a ByteString
impl From<String> for ByteString {
    fn from(s: String) -> Self {
        Self(s.into_bytes())
    }
}

impl From<&str> for ByteString {
    fn from(s: &str) -> Self {
        Self(s.as_bytes().to_vec())
    }
}

/// Attempt to convert a ByteString into a UTF-8 String
impl TryInto<String> for ByteString {
    type Error = std::string::FromUtf8Error;

    fn try_into(self) -> Result<String, Self::Error> {
        String::from_utf8(self.0)
    }
}

/// Conversion to OsString is guaranteed to succeed on unix systems
/// but can potentially fail on Windows systems.
impl TryInto<OsString> for ByteString {
    type Error = std::string::FromUtf8Error;

    #[cfg(unix)]
    fn try_into(self) -> Result<OsString, Self::Error> {
        Ok(std::os::unix::ffi::OsStringExt::from_vec(self.0))
    }

    #[cfg(windows)]
    fn try_into(self) -> Result<OsString, Self::Error> {
        let s = String::from_utf8(self.0)?;
        Ok(s.into())
    }
}

/// Conversion to PathBuf is subject to the same rules as conversion
/// to OsString
impl TryInto<PathBuf> for ByteString {
    type Error = std::string::FromUtf8Error;

    fn try_into(self) -> Result<PathBuf, Self::Error> {
        let os: OsString = self.try_into()?;
        Ok(os.into())
    }
}

/// Conversion from OsString -> ByteString is guaranteed to succeed on unix
/// systems but can potentially fail on Windows systems.
impl TryInto<ByteString> for OsString {
    type Error = &'static str;

    #[cfg(unix)]
    fn try_into(self) -> Result<ByteString, Self::Error> {
        Ok(ByteString(std::os::unix::ffi::OsStringExt::into_vec(self)))
    }

    #[cfg(windows)]
    fn try_into(self) -> Result<ByteString, Self::Error> {
        let s = self
            .into_string()
            .map_err(|_| "OsString is not representible as UTF-8")?;
        Ok(ByteString(s.into_bytes()))
    }
}

/// Conversion from PathBuf -> ByteString is subject to the same rules
/// as conversion from OsString
impl TryInto<ByteString> for PathBuf {
    type Error = &'static str;

    fn try_into(self) -> Result<ByteString, Self::Error> {
        self.into_os_string().try_into()
    }
}

#[cfg(test)]
mod tests {
    use super::ByteString;
    use crate::from_slice;
    use crate::ser::serialize;

    #[test]
    fn test_serde() {
        let bs = ByteString::from(vec![1, 2, 3, 4]);

        let out = serialize(Vec::<u8>::new(), &bs).unwrap();
        assert_eq!(
            out,
            b"\x00\x02\x00\x00\x00\x00\x03\x07\x02\x03\x04\x01\x02\x03\x04"
        );

        let got: ByteString = from_slice(&out).unwrap();
        assert_eq!(bs, got);
    }
}
