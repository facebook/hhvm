// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;
use std::collections::BTreeSet;
use std::path::Path;

use bstr::ByteSlice;
use oxidized::global_options::AllOrSome;
use sha1::Digest;
use sha1::Sha1;

#[derive(Debug, Clone, Default)]
pub struct ConfigFile {
    map: BTreeMap<String, String>,
}

impl ConfigFile {
    pub fn from_file(path: impl AsRef<Path>) -> std::io::Result<Self> {
        let contents = std::fs::read(path.as_ref())?;
        Ok(Self::from_slice(&contents))
    }

    pub fn from_file_with_contents(path: impl AsRef<Path>) -> std::io::Result<(String, Self)> {
        let contents = std::fs::read(path.as_ref())?;
        Ok((
            String::from_utf8(contents.clone()).unwrap(),
            Self::from_slice(&contents),
        ))
    }

    pub fn from_file_with_sha1(path: impl AsRef<Path>) -> std::io::Result<(String, Self)> {
        let path = path.as_ref();
        let contents = std::fs::read(path)?;
        Ok(Self::from_slice_with_sha1(&contents))
    }

    pub fn from_slice_with_sha1(bytes: &[u8]) -> (String, Self) {
        let hash = format!("{:x}", Sha1::digest(bytes));
        let config = Self::from_slice(bytes);
        (hash, config)
    }

    fn parse_line(line: &[u8]) -> Option<(String, String)> {
        let mut key_value_iter = line.splitn(2, |&c| c == b'=');
        let key = key_value_iter.next()?.trim();
        let value = key_value_iter.next()?.trim();
        Some((
            String::from_utf8_lossy(key).into_owned(),
            String::from_utf8_lossy(value).into_owned(),
        ))
    }

    pub fn from_slice(bytes: &[u8]) -> Self {
        let map = (bytes.lines())
            .filter_map(|line| match line.first() {
                Some(b'#') => None,
                _ => Self::parse_line(line),
            })
            .collect();
        Self { map }
    }

    pub fn from_args<'i, I>(kvs: I) -> Self
    where
        I: IntoIterator<Item = &'i [u8]>,
    {
        Self {
            map: kvs.into_iter().filter_map(Self::parse_line).collect(),
        }
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

    pub fn get_int_or(&self, key: &str, default: isize) -> Result<isize, std::num::ParseIntError> {
        self.map.get(key).map_or(Ok(default), |s| parse_int(s))
    }

    pub fn get_int_set_or(
        &self,
        key: &str,
        default: BTreeSet<isize>,
    ) -> Result<BTreeSet<isize>, std::num::ParseIntError> {
        self.map.get(key).map_or(Ok(default), |s| {
            s.split_terminator(',')
                .map(|s| s.trim().parse())
                .collect::<Result<_, _>>()
        })
    }

    pub fn get_ints_or(
        &self,
        key: &str,
        default: Vec<isize>,
    ) -> Result<Vec<isize>, std::num::ParseIntError> {
        self.map.get(key).map_or(Ok(default), |s| {
            s.split_terminator(',')
                .map(|s| s.trim().parse())
                .collect::<Result<_, _>>()
        })
    }

    pub fn get_float(&self, key: &str) -> Option<Result<f64, std::num::ParseFloatError>> {
        self.map.get(key).map(|s| s.parse())
    }

    pub fn get_float_or(&self, key: &str, default: f64) -> Result<f64, std::num::ParseFloatError> {
        self.map.get(key).map_or(Ok(default), |s| s.parse())
    }

    pub fn get_bool(&self, key: &str) -> Option<Result<bool, std::str::ParseBoolError>> {
        self.map.get(key).map(|s| s.parse())
    }

    pub fn get_bool_or(&self, key: &str, default: bool) -> Result<bool, std::str::ParseBoolError> {
        self.map.get(key).map_or(Ok(default), |s| s.parse())
    }

    pub fn get_all_or_some_ints(
        &self,
        key: &str,
    ) -> Option<Result<AllOrSome<isize>, std::num::ParseIntError>> {
        self.map.get(key).map(|s| match s.as_str() {
            "all" | "true" => Ok(AllOrSome::All),
            "none" | "false" => Ok(AllOrSome::ASome(vec![])),
            s => s
                .split_terminator(',')
                .map(|s| s.trim().parse())
                .collect::<Result<_, _>>()
                .map(AllOrSome::ASome),
        })
    }

    pub fn get_all_or_some_ints_or(
        &self,
        key: &str,
        default: AllOrSome<isize>,
    ) -> Result<AllOrSome<isize>, std::num::ParseIntError> {
        match self.get_all_or_some_ints(key) {
            None => Ok(default),
            Some(r) => r,
        }
    }

    pub fn bool_if_min_version(
        &self,
        key: &str,
        _current_version: Option<&str>,
    ) -> Option<Result<bool, std::str::ParseBoolError>> {
        Some(match self.get_bool(key)? {
            Ok(b) => Ok(b),
            Err(e) => {
                // TODO handle versions
                Err(e)
            }
        })
    }

    pub fn get_either_int_or_str(&self, key: &str) -> Option<Result<isize, String>> {
        self.map.get(key).map(|s| match parse_int(s) {
            Ok(i) => Ok(i),
            _ => Err(s.to_owned()),
        })
    }

    pub fn get_str_list(&self, key: &str) -> Option<impl Iterator<Item = &str>> {
        lazy_static::lazy_static! {
            static ref RE: regex::Regex = regex::Regex::new(",[ \n\r\x0c\t]*").unwrap();
        }
        self.map.get(key).map(|s| RE.split(s.as_str()))
    }

    pub fn get_string_set_or(&self, key: &str, default: BTreeSet<String>) -> BTreeSet<String> {
        self.map.get(key).map_or(default, |s| {
            s.split_terminator(',')
                .map(|s| s.trim().to_owned())
                .collect()
        })
    }

    pub fn copy_key(&mut self, other: &ConfigFile, key: &str) -> Result<(), String> {
        if let Some(value) = other.get_str(key) {
            self.map.insert(key.to_owned(), value.to_owned());
            Ok(())
        } else {
            Err(format!("Missing {key} in input"))
        }
    }
}

impl FromIterator<(String, String)> for ConfigFile {
    fn from_iter<I: IntoIterator<Item = (String, String)>>(iter: I) -> Self {
        Self {
            map: BTreeMap::from_iter(iter),
        }
    }
}

impl IntoIterator for ConfigFile {
    type Item = (String, String);
    type IntoIter = std::collections::btree_map::IntoIter<String, String>;

    fn into_iter(self) -> Self::IntoIter {
        self.map.into_iter()
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
