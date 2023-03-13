/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

use std::collections::BTreeSet;

use anyhow::Result;
use regex::Regex;

struct CrateMapping {
    module: String,
    crate_name: String,
}

// Thrift writes a rust crate name mapping as follows:
// <thrift_module_name> <rust_crate_name>
//
// If you want to change this, please coordinate with dependent teams to ensure
// these libraries will still be able to divine the crate name.
fn parse_map(lines: &str) -> Vec<CrateMapping> {
    let re = Regex::new(r"(?m)^\s*([[:alnum:]_-]+)\s+([[:alnum:]_-]+)").unwrap();
    re.captures_iter(lines)
        .map(|caps| CrateMapping {
            module: caps[1].to_string(),
            crate_name: caps[2].to_string(),
        })
        .collect()
}

#[test]
fn test_crate_schema() -> Result<()> {
    let lines = include_str!("fubar_crate_map");
    let mappings = parse_map(lines);
    assert_eq!(mappings.len(), 2);
    let modules: BTreeSet<String> = mappings.iter().map(|m| m.module.clone()).collect();
    let crate_names: BTreeSet<String> = mappings.iter().map(|m| m.crate_name.clone()).collect();
    assert_eq!(crate_names.len(), 1);
    assert!(crate_names.contains("best_crate"));
    assert_eq!(modules.len(), 2);
    assert!(modules.contains("foos"));
    assert!(modules.contains("bars"));
    Ok(())
}
