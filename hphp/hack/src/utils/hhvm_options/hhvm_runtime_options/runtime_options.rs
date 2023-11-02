// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

use std::borrow::Cow;
use std::fs;

use anyhow::Result;

use crate::cxx_ffi;

/// A machine can belong to a tier, which can overwrite
/// various settings, even if they are set in the same
/// hdf file. However, CLI overrides still win the day over
/// everything.
///
/// Based on getTierOverwrites() in runtime-option.cpp
pub fn apply_tier_overrides(mut config: hdf::Value) -> Result<hdf::Value> {
    // Machine metrics
    let hostname = config
        .get_str("Machine.name")?
        .unwrap_or_else(cxx_ffi::Process_GetHostName);

    let tier: String = config.get_str("Machine.tier")?.unwrap_or_default();
    let task: String = config.get_str("Machine.task")?.unwrap_or_default();

    let cpu: String = config
        .get_str("Machine.cpu")?
        .unwrap_or_else(cxx_ffi::Process_GetCPUModel);

    let tiers: String = config
        .get_str("Machine.tiers")?
        .and_then(|tiers| fs::read_to_string(tiers).ok())
        .unwrap_or_else(|| "".to_owned());

    let tags: String = config
        .get_str("Machine.tags")?
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

    let check_patterns = |hdf: &hdf::Value| -> Result<bool> {
        Ok(match_hdf_pattern(&hostname, hdf, "machine", "")?
            && match_hdf_pattern(&tier, hdf, "tier", "")?
            && match_hdf_pattern(&task, hdf, "task", "")?
            && match_hdf_pattern(&tiers, hdf, "tiers", "m")?
            && match_hdf_pattern(&tags, hdf, "tags", "m")?
            && match_hdf_pattern(&cpu, hdf, "cpu", "")?)
    };

    let mut enable_shards = true;

    if let Some(tiers) = config.get("Tiers")? {
        for tier in tiers.into_children()? {
            let tier = tier?;
            if check_patterns(&tier)?
                && (!tier.contains_key("exclude")?
                    || !tier
                        .get("exclude")?
                        .map_or(Ok(false), |v| check_patterns(&v))?
                        && match_shard(enable_shards, &hostname, &tier))
            {
                log::info!("Matched tier: {}", tier.name()?);

                if enable_shards && tier.get_bool_or("DisableShards", false)? {
                    log::info!("Sharding is disabled.");
                    enable_shards = false;
                }

                if let Some(remove) = tier.get("clear")? {
                    for s in remove.values()? {
                        config.remove(&s)?;
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

#[allow(clippy::todo)]
fn match_shard(_en: bool, _hostname: &str, _config: &hdf::Value) -> bool {
    todo!();
}

// Config::matchHdfPattern()
#[allow(clippy::todo)]
fn match_hdf_pattern(_value: &str, config: &hdf::Value, name: &str, suffix: &str) -> Result<bool> {
    let pattern = config.get_str(name)?.unwrap_or_default();
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
    Ok(true)
}
