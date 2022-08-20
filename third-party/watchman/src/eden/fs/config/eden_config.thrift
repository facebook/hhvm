/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This software may be used and distributed according to the terms of the
 * GNU General Public License version 2.
 */

namespace cpp2 facebook.eden
namespace java com.facebook.eden.thrift
namespace py facebook.eden.eden_config
namespace py3 eden.fs.config

/**
 * ConfigSource identifies the point of origin of a config setting.
 * It is ordered from low to high precedence. Higher precedence
 * configuration values over-ride lower precedence values. A config
 * setting of CommandLine takes precedence over all other settings.
 */
enum ConfigSource {
  Default = 0,
  SystemConfig = 1,
  UserConfig = 2,
  CommandLine = 3,
}

enum ConfigReloadBehavior {
  // Automatically reload the configuration file from disk if it appears to be
  // necessary.
  AutoReload = 0,
  // Do not reload the config from disk, and return the current cached values.
  NoReload = 1,
  // Always attempt to reload the config from disk, even if we have recently
  // checked to see if it was up-to-date.
  ForceReload = 2,
}

struct ConfigValue {
  // parsedValue contains the value parsed by Eden, after performing variable
  // substitution (${HOME}, ${USER}, etc)
  // TODO: In the future it may be nice to add a sourceValue field that
  // contains the original value from before variable interpolation.  We don't
  // currently store this data after performing parsing, however.
  1: string parsedValue;
  2: ConfigSource source;
}

struct EdenConfigData {
  1: map<string, ConfigValue> values;
}
