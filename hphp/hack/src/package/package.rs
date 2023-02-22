// Copyright (c) Meta, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Context;
use anyhow::Result;
use hash::HashMap;
use serde::Deserialize;

#[derive(Debug, Deserialize)]
struct Config {
    packages: HashMap<String, PackageInfo>,
}

#[derive(Debug, Deserialize)]
#[allow(dead_code)]
pub struct PackageInfo {
    uses: Option<Vec<String>>,
    includes: Option<Vec<String>>,
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

        assert_eq!(packages["foo"].uses.as_ref().unwrap()[0], "a.*");
        assert!(packages["foo"].includes.is_none());

        assert_eq!(packages["bar"].uses.as_ref().unwrap()[0], "b.*");
        assert_eq!(packages["bar"].includes.as_ref().unwrap()[0], "foo");

        assert_eq!(packages["baz"].uses.as_ref().unwrap()[0], "x.*");
        assert_eq!(packages["baz"].uses.as_ref().unwrap()[1], "y.*");
        assert_eq!(packages["baz"].includes.as_ref().unwrap()[0], "foo");
        assert_eq!(packages["baz"].includes.as_ref().unwrap()[1], "bar");
    }
}
