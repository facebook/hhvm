// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

use std::borrow::Cow;
use std::fs;

use anyhow::Result;
use byteorder::ByteOrder;
use byteorder::NetworkEndian;
use md5::Digest;
use md5::Md5;

use crate::cxx_ffi;

/// A machine can belong to a tier, which can overwrite
/// various settings, even if they are set in the same
/// hdf file. However, CLI overrides still win the day over
/// everything.
///
/// Based on getTierOverwrites() in runtime-option.cpp
pub fn apply_tier_overrides(config: hdf::Value) -> Result<hdf::Value> {
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

    apply_tier_overrides_with_params(config, &hostname, &tier, &task, &cpu, &tiers, &tags, None)
}

pub fn apply_tier_overrides_with_params(
    mut config: hdf::Value,
    hostname: &String,
    tier: &String,
    task: &String,
    cpu: &String,
    tiers: &String,
    tags: &String,
    predefined_shard_value: Option<i64>,
) -> Result<hdf::Value> {
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
        Ok(match_hdf_pattern(hostname, hdf, "machine", "")?
            && match_hdf_pattern(tier, hdf, "tier", "")?
            && match_hdf_pattern(task, hdf, "task", "")?
            && match_hdf_pattern(tiers, hdf, "tiers", "m")?
            && match_hdf_pattern(tags, hdf, "tags", "m")?
            && match_hdf_pattern(cpu, hdf, "cpu", "")?)
    };

    let mut enable_shards = true;

    if let Some(tiers) = config.get("Tiers")? {
        for tier in tiers.into_children()? {
            let tier = tier?;
            if check_patterns(&tier)?
                && (!tier.contains_key("exclude")?
                    || !tier
                        .get("exclude")?
                        .map_or(Ok(false), |v| check_patterns(&v))?)
                && match_shard(enable_shards, hostname, &tier, predefined_shard_value)?
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

fn match_shard(
    enable_shards: bool,
    hostname: &str,
    config: &hdf::Value,
    predefined_shard_value: Option<i64>,
) -> Result<bool> {
    if !enable_shards || !config.contains_key("Shard")? {
        return Ok(true);
    }
    let shard = config.get_int64_or("Shard", -1)?;
    let nshards = config.get_int64_or("ShardCount", 100)?;
    if shard < 0 || shard >= nshards {
        log::warn!(
            "Invalid shard number {}, must in the range [0..{})",
            shard,
            nshards
        );
        return Ok(true);
    }

    // This is a small variation from the standard logic to accommodate forced
    // evaluation of tier overrides for validation.  Reverse engineering
    // a md5 hash is ineffective for that purpose.
    if let Some(predefined_shard_value) = predefined_shard_value {
        return Ok(predefined_shard_value % nshards <= shard);
    }

    // This is close to the hash behavior in HHVM but not exact its a decent approximation however
    let mut input: String = hostname.to_string();
    if config.contains_key("ShardSalt")? {
        if let Some(salt) = config.get_str("ShardSalt")? {
            input.push_str(salt.as_str());
        }
    }
    let seed = create_seed(&input)?;
    log::info!(
        "Checking Shard = {shard}; input = {input}, seed = {seed}; ShardCount = {nshards}; Value = {}",
        seed % nshards
    );
    Ok(seed % nshards <= shard)
}

// This is to match the behavior of matchShard in runtime-option.cpp
fn create_seed(input: &str) -> Result<i64> {
    // This NetworkEndian and shift is to match the behavior of sharding in chef:
    //   seed = Digest::MD5.hexdigest(seed_input)[0...7].to_i(16)
    Ok((NetworkEndian::read_u32(&Md5::digest(input.as_bytes()).as_slice()[..4]) >> 4) as i64)
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

#[test]
fn test_match_shard() -> Result<()> {
    // From HHVM
    // Checking Shard = 4; Input = test_hostnamebespoke; Seed = 190005716; ShardCount = 100; Value = 16
    assert_eq!(create_seed("test_hostnamebespoke")?, 190005716);
    // From HHVM
    // Checking Shard = 0; Input = test_hostnamenoinline; Seed = 44676460; ShardCount = 1000; Value = 460
    assert_eq!(create_seed("test_hostnamenoinline")?, 44676460);

    Ok(())
}

#[test]
fn test_get_seed() -> Result<()> {
    let hostname = "test_hostname";
    let enabled = true;
    let disabled = false;
    assert_eq!(create_seed(hostname)? % 100, 81);
    assert_eq!(create_seed("test_hostnamebespoke")? % 100, 16);

    let mut config = hdf::Value::default();

    // Config with no Shard should be true
    assert!(match_shard(enabled, hostname, &config, None)?);

    // With Shard

    // Invalid
    config.set_hdf("Shard = -1")?;
    assert!(match_shard(disabled, hostname, &config, None)?);
    config.set_hdf("Shard = 100")?;
    assert!(match_shard(disabled, hostname, &config, None)?);

    // Valid
    config.set_hdf("Shard = 80")?;
    assert!(match_shard(disabled, hostname, &config, None)?);
    assert!(!match_shard(enabled, hostname, &config, None)?);

    config.set_hdf("Shard = 81")?;
    assert!(match_shard(enabled, hostname, &config, None)?);

    // With salt
    config.set_hdf("Shard = 0\nShardSalt = bespoke")?;
    assert!(!match_shard(enabled, hostname, &config, None)?);
    config.set_hdf("Shard = 16")?;
    assert!(match_shard(enabled, hostname, &config, None)?);

    // With custom ShardCount and Salt
    config.set_hdf("Shard = 5\nShardCount = 10")?;
    assert!(!match_shard(enabled, hostname, &config, None)?);
    config.set_hdf("Shard = 6")?;
    assert!(match_shard(enabled, hostname, &config, None)?);

    // Precomputed shards cutoff 1 (non-prod behavior) and ShardCount 10
    config.set_hdf("Shard = 1")?;
    assert!(match_shard(enabled, hostname, &config, Some(0))?);
    assert!(match_shard(enabled, hostname, &config, Some(1))?);
    assert!(!match_shard(enabled, hostname, &config, Some(2))?);
    assert!(match_shard(enabled, hostname, &config, Some(11))?);

    Ok(())
}
