// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;
use std::path::Path;
use std::str::FromStr;

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

    pub fn get<T: FromStr>(&self, key: &str) -> Option<Result<T, T::Err>> {
        self.map.get(key).map(|s| s.parse())
    }

    pub fn get_str(&self, key: &str) -> Option<&str> {
        self.map.get(key).map(|s| s.as_str())
    }

    pub fn get_str_list(&self, key: &str) -> Option<impl Iterator<Item = &str>> {
        lazy_static::lazy_static! {
            static ref RE: regex::Regex = regex::Regex::new(",[ \n\r\x0c\t]*").unwrap();
        }
        self.map.get(key).map(|s| RE.split(s.as_str()))
    }
}

impl std::iter::FromIterator<(String, String)> for ConfigFile {
    fn from_iter<I: IntoIterator<Item = (String, String)>>(iter: I) -> Self {
        Self {
            map: BTreeMap::from_iter(iter),
        }
    }
}
