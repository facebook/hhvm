// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;
use std::path::Path;

use oxidized::decl_parser_options::DeclParserOptions;
use sha1::Digest;
use sha1::Sha1;

pub const FILE_PATH_RELATIVE_TO_ROOT: &str = ".hhconfig";

/// For now, this struct only contains the parts of .hhconfig which
/// have been needed in Rust tools.
#[derive(Debug, Clone, Default)]
pub struct HhConfig {
    pub auto_namespace_map: BTreeMap<String, String>,
    pub disable_xhp_element_mangling: bool,
    pub interpret_soft_types_as_like_types: bool,
    pub everything_sdt: bool,
    pub deregister_php_stdlib: bool,
    pub version: Option<String>,

    /// List of regex patterns of root-relative paths to ignore.
    pub ignored_paths: Vec<String>,

    /// SHA1 Hash of the .hhconfig file contents.
    pub hash: String,
}

impl HhConfig {
    pub fn from_root(root: &Path) -> std::io::Result<Self> {
        let hhconfig_path = root.join(FILE_PATH_RELATIVE_TO_ROOT);
        let hhconfig_contents = std::fs::read(&hhconfig_path)?;
        Ok(Self::from_slice(&hhconfig_contents))
    }

    pub fn from_slice(bytes: &[u8]) -> Self {
        use bstr::ByteSlice;

        let mut c = Self::default();
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

            match key {
                b"auto_namespace_map" => {
                    c.auto_namespace_map = serde_json::from_slice(value).unwrap();
                }
                b"disable_xhp_element_mangling" => {
                    c.disable_xhp_element_mangling = serde_json::from_slice(value).unwrap();
                }
                b"interpret_soft_types_as_like_types" => {
                    c.interpret_soft_types_as_like_types = serde_json::from_slice(value).unwrap();
                }
                b"everything_sdt" => {
                    c.everything_sdt = serde_json::from_slice(value).unwrap();
                }
                b"deregister_php_stdlib" => {
                    c.deregister_php_stdlib = serde_json::from_slice(value).unwrap();
                }
                b"version" => {
                    c.version = Some(String::from_utf8_lossy(value).into_owned());
                }
                b"ignored_paths" => {
                    c.ignored_paths = serde_json::from_slice(value).unwrap();
                }
                _ => {}
            }
        }
        c.hash = format!("{:x}", Sha1::digest(bytes));
        c
    }

    pub fn get_decl_parser_options(&self) -> DeclParserOptions {
        let auto_namespace_map = self
            .auto_namespace_map
            .iter()
            .map(|(name, value)| (name.clone(), value.clone()))
            .collect();
        DeclParserOptions {
            auto_namespace_map,
            disable_xhp_element_mangling: self.disable_xhp_element_mangling,
            interpret_soft_types_as_like_types: self.interpret_soft_types_as_like_types,
            everything_sdt: self.everything_sdt,
            ..Default::default()
        }
    }
}
