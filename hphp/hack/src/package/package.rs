// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::HashMap;

use anyhow::Context;
use anyhow::Result;
use serde::Deserialize;

#[derive(Debug, Deserialize)]
struct Config {
    packages: HashMap<String, PackageInfo>,
}

#[derive(Debug, Deserialize)]
#[allow(dead_code)]
pub struct PackageInfo {
    r#use: Option<Vec<String>>,
    include: Option<Vec<String>>,
}

pub fn parse_config(contents: &str) -> Result<HashMap<String, PackageInfo>> {
    let config: Config = toml::from_str(contents)
        .with_context(|| format!("Failed to parse config file with contents: {}", contents))?;
    Ok(config.packages)
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_parsing_basic_file() {
        let contents = include_str!("tests/package-1.toml");
        let packages = parse_config(contents).unwrap();

        assert_eq!(packages["foo"].r#use.as_ref().unwrap()[0], "a.*");
        assert!(packages["foo"].include.is_none());

        assert_eq!(packages["bar"].r#use.as_ref().unwrap()[0], "b.*");
        assert_eq!(packages["bar"].include.as_ref().unwrap()[0], "foo");

        assert_eq!(packages["baz"].r#use.as_ref().unwrap()[0], "x.*");
        assert_eq!(packages["baz"].r#use.as_ref().unwrap()[1], "y.*");
        assert_eq!(packages["baz"].include.as_ref().unwrap()[0], "foo");
        assert_eq!(packages["baz"].include.as_ref().unwrap()[1], "bar");
    }
}
