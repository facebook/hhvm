// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

use crate::cxx_ffi;
use anyhow::Result;
use std::{borrow::Cow, fs};

/// A machine can belong to a tier, which can overwrite
/// various settings, even if they are set in the same
/// hdf file. However, CLI overrides still win the day over
/// everything.
///
/// Based on getTierOverwrites() in runtime-option.cpp
pub fn apply_tier_overrides(mut config: hdf::Value) -> Result<hdf::Value> {
    // Machine metrics
    let hostname: String = config
        .get_str("Machine.name")
        .map_or_else(cxx_ffi::Process_GetHostName, |s| s.to_string());

    let tier: String = config.get_str("Machine.tier").unwrap_or("").to_string();
    let task: String = config.get_str("Machine.task").unwrap_or("").to_string();

    let cpu: String = config
        .get_str("Machine.cpu")
        .map_or_else(cxx_ffi::Process_GetCPUModel, |s| s.to_string());

    let tiers: String = config
        .get_str("Machine.tiers")
        .and_then(|tiers| fs::read_to_string(tiers).ok())
        .unwrap_or_else(|| "".to_owned());

    let tags: String = config
        .get_str("Machine.tags")
        .and_then(|tiers| fs::read_to_string(tiers).ok())
        .unwrap_or_else(|| "".to_owned());

    log::debug!(
        "Matching tiers using: machine='{}', tier='{}', task='{}', cpu='{}', tiers='{}', tags='{}'",
        hostname,
        tier,
        task,
        cpu,
        tiers,
        tags
    );

    let check_patterns = |hdf: &hdf::Value| {
        match_hdf_pattern(&hostname, hdf, "machine", "")
            && match_hdf_pattern(&tier, hdf, "tier", "")
            && match_hdf_pattern(&task, hdf, "task", "")
            && match_hdf_pattern(&tiers, hdf, "tiers", "m")
            && match_hdf_pattern(&tags, hdf, "tags", "m")
            && match_hdf_pattern(&cpu, hdf, "cpu", "")
    };

    let mut enable_shards = true;

    let tiers: Vec<String> = config.get_or_empty("Tiers").keys().cloned().collect();
    for tier_key in &tiers {
        if let Some(tier) = config.get(tier_key) {
            if check_patterns(tier)
                && (!tier.contains_key("exclude")
                    || !tier.get("exclude").map_or(false, check_patterns)
                        && match_shard(enable_shards, &hostname, tier))
            {
                log::info!("Matched tier: {}", tier_key);

                if enable_shards && tier.get_bool("DisableShards")?.unwrap_or(false) {
                    log::info!("Sharding is disabled.");
                    enable_shards = false;
                }

                if let Some(clear) = tier.get("clear") {
                    let remove: Vec<String> = clear
                        .values()
                        .map(|v| v.as_str().map_or_else(String::new, str::to_owned))
                        .collect();
                    for s in remove {
                        config.remove(&s);
                    }
                }

                //-- config.copy(tier["overwrite"]);
                // no break here, so we can continue to match more overwrites
            }

            // Avoid lint errors about unvisited nodes when the tier does not match.
            //-- tier["DisableShards"].setVisited();
            //-- tier["clear"].setVisited();
            //-- tier["overwrite"].setVisited();
        }
    }


    Ok(config)
}

fn match_shard(_en: bool, _hostname: &str, _config: &hdf::Value) -> bool {
    todo!();
}

// Config::matchHdfPattern()
fn match_hdf_pattern(_value: &str, config: &hdf::Value, name: &str, suffix: &str) -> bool {
    let pattern = config.get_str(name).unwrap_or("");
    if !pattern.is_empty() {
        let _pattern: Cow<'_, str> = if suffix.is_empty() {
            pattern.into()
        } else {
            format!("{}{}", pattern, suffix).into()
        };
        todo!();
        //-- Variant ret = preg_match(String(pattern.c_str(), pattern.size(),
        //--                                 CopyString),
        //--                          String(value.c_str(), value.size(),
        //--                                 CopyString));
        //-- if (ret.toInt64() <= 0) {
        //--   return false;
        //-- }
    }
    true
}
