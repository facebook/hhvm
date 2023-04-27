// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use config_file::ConfigFile;
use ocamlrep_custom::Custom;

struct Config(ConfigFile);

impl ocamlrep_custom::CamlSerialize for Config {
    ocamlrep_custom::caml_serialize_default_impls!();
}

fn wrap(config: ConfigFile) -> Custom<Config> {
    Custom::from(Config(config))
}

ocamlrep_ocamlpool::ocaml_ffi! {
    fn hh_config_file_empty() -> Custom<Config> {
        wrap(ConfigFile::default())
    }

    fn hh_config_file_parse_contents(contents: Vec<u8>) -> Custom<Config> {
        wrap(ConfigFile::from_slice(&contents))
    }

    fn hh_config_file_is_empty(config: Custom<Config>) -> bool {
        config.0.is_empty()
    }

    fn hh_config_file_print_to_stderr(config: Custom<Config>) {
        for (key, value) in config.0.iter() {
            eprintln!("{} = {}", key, value);
        }
        use std::io::Write;
        let _ = std::io::stderr().flush();
    }

    fn hh_config_file_apply_overrides(
        config: Custom<Config>,
        overrides: Custom<Config>,
    ) -> Custom<Config> {
        let mut config = config.0.clone();
        config.apply_overrides(&overrides.0);
        wrap(config)
    }

    fn hh_config_file_to_json(config: Custom<Config>) -> Result<String, String> {
        config.0.to_json().map_err(|e| e.to_string())
    }

    fn hh_config_file_of_list(list: Vec<(String, String)>) -> Custom<Config> {
        wrap(list.into_iter().collect())
    }

    fn hh_config_file_keys(config: Custom<Config>) -> Vec<String> {
        config.0.keys().map(|s| s.to_owned()).collect()
    }

    fn hh_config_file_get_string_opt(config: Custom<Config>, key: String) -> Option<String> {
        config.0.get_str(&key).map(|s| s.to_owned())
    }

    fn hh_config_file_get_int_opt(
        config: Custom<Config>,
        key: String,
    ) -> Option<Result<isize, String>> {
        config.0.get_int(&key)
            .map(|r| r.map_err(|e| e.to_string()))
    }

    fn hh_config_file_get_float_opt(
        config: Custom<Config>,
        key: String,
    ) -> Option<Result<f64, String>> {
        config.0.get_float(&key)
            .map(|r| r.map_err(|e| e.to_string()))
    }

    fn hh_config_file_get_bool_opt(
        config: Custom<Config>,
        key: String,
    ) -> Option<Result<bool, String>> {
        config.0.get_bool(&key)
            .map(|r| r.map_err(|e| e.to_string()))
    }

    fn hh_config_file_get_string_list_opt(
        config: Custom<Config>,
        key: String,
    ) -> Option<Vec<String>> {
        config.0.get_str_list(&key)
            .map(|iter| iter.map(|s| s.to_owned()).collect())
    }
}
