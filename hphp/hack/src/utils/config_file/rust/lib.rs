// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;
use std::path::Path;

use sha1::{Digest, Sha1};

#[derive(Clone)]
pub struct ConfigFile {
    map: BTreeMap<String, String>,
}

impl ConfigFile {
    pub fn empty() -> Self {
        Self {
            map: BTreeMap::new(),
        }
    }

    pub fn from_file(path: impl AsRef<Path>) -> std::io::Result<Self> {
        let contents = std::fs::read(path.as_ref())?;
        Ok(Self::from_slice(&contents))
    }

    pub fn from_file_with_sha1(path: impl AsRef<Path>) -> std::io::Result<(String, Self)> {
        let path = path.as_ref();
        let contents = std::fs::read(path)?;
        let hash = format!("{:x}", Sha1::digest(&contents));
        Ok((hash, Self::from_slice(&contents)))
    }

    pub fn from_slice(bytes: &[u8]) -> Self {
        use bstr::ByteSlice;

        let mut map = BTreeMap::new();

        for line in bytes.lines() {
            if matches!(line.get(0), Some(b'#')) {
                continue;
            }
            let mut key_value_iter = line.splitn(2, |&c| c == b'=');
            let key = match key_value_iter.next() {
                Some(key) => key.trim(),
                None => continue,
            };
            let value = match key_value_iter.next() {
                Some(value) => value.trim(),
                None => continue,
            };

            map.insert(
                String::from_utf8_lossy(key).into_owned(),
                String::from_utf8_lossy(value).into_owned(),
            );
        }

        Self { map }
    }

    pub fn is_empty(&self) -> bool {
        self.map.is_empty()
    }

    pub fn apply_overrides(&mut self, overrides: &Self) {
        for (key, value) in overrides.map.iter() {
            self.map.insert(key.clone(), value.clone());
        }
    }

    pub fn to_json(&self) -> serde_json::Result<String> {
        serde_json::to_string(&self.map)
    }

    pub fn iter(&self) -> impl Iterator<Item = (&str, &str)> {
        self.map.iter().map(|(k, v)| (k.as_str(), v.as_str()))
    }

    pub fn keys(&self) -> impl Iterator<Item = &str> {
        self.map.keys().map(|s| s.as_str())
    }

    pub fn get_str(&self, key: &str) -> Option<&str> {
        self.map.get(key).map(|s| s.as_str())
    }

    pub fn get_int(&self, key: &str) -> Option<Result<isize, std::num::ParseIntError>> {
        self.map.get(key).map(|s| parse_int(s))
    }

    pub fn get_float(&self, key: &str) -> Option<Result<f64, std::num::ParseFloatError>> {
        self.map.get(key).map(|s| s.parse())
    }

    pub fn get_bool(&self, key: &str) -> Option<Result<bool, std::str::ParseBoolError>> {
        self.map.get(key).map(|s| s.parse())
    }

    pub fn get_str_list(&self, key: &str) -> Option<impl Iterator<Item = &str>> {
        lazy_static::lazy_static! {
            static ref RE: regex::Regex = regex::Regex::new(",[ \n\r\x0c\t]*").unwrap();
        }
        self.map.get(key).map(|s| RE.split(s.as_str()))
    }
}

impl FromIterator<(String, String)> for ConfigFile {
    fn from_iter<I: IntoIterator<Item = (String, String)>>(iter: I) -> Self {
        Self {
            map: BTreeMap::from_iter(iter),
        }
    }
}

// Intended to behave like `caml_int_of_string`.
fn parse_int(input: &str) -> Result<isize, std::num::ParseIntError> {
    use std::borrow::Cow;

    let input = if input.contains('_') {
        Cow::Owned(input.chars().filter(|&c| c != '_').collect())
    } else {
        Cow::Borrowed(input)
    };

    if input.starts_with("0b") {
        isize::from_str_radix(input.trim_start_matches("0b"), 2)
    } else if input.starts_with("0o") {
        isize::from_str_radix(input.trim_start_matches("0o"), 8)
    } else if input.starts_with("0x") {
        isize::from_str_radix(input.trim_start_matches("0x"), 16)
    } else {
        input.parse()
    }
}

#[cfg(test)]
mod test_parse_int {
    use super::parse_int;

    #[test]
    fn test() {
        // These test cases assert the same behavior as `caml_int_of_string`.
        assert_eq!(parse_int("42"), Ok(42));
        assert_eq!(parse_int("-42"), Ok(-42));
        assert_eq!(parse_int("0x42"), Ok(66));
        assert_eq!(parse_int("0o42"), Ok(34));
        assert_eq!(parse_int("042"), Ok(42));
        assert_eq!(parse_int("0b0110"), Ok(6));
        assert_eq!(parse_int("0b0110_0110"), Ok(102));
        assert_eq!(parse_int("0x0110_0110"), Ok(17_826_064));
        assert_eq!(parse_int("0o0110_0110"), Ok(294_984));
        assert_eq!(parse_int("1_100_110"), Ok(1_100_110));
        assert_eq!(parse_int("0110_0110"), Ok(1_100_110));
    }
}
