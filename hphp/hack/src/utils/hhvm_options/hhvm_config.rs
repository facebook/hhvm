// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
use anyhow::Result;

#[derive(Debug, Default)]
pub struct HhvmConfig {
    pub hdf_config: hdf::Value,
    pub ini_config: hdf::Value,
}

impl HhvmConfig {
    pub fn get_str<'a>(&'a self, key: &str) -> Result<Option<String>> {
        self.get_helper(
            key,
            /*prepend_hhvm=*/ true,
            |config, key| Ok(config.get_str(key)?),
        )
    }

    pub fn get_bool(&self, key: &str) -> Result<Option<bool>> {
        self.get_helper(
            key,
            /*prepend_hhvm=*/ true,
            |config, key| Ok(config.get_bool(key)?),
        )
    }
    pub fn get_uint32(&self, key: &str) -> Result<Option<u32>> {
        self.get_helper(
            key,
            /*prepend_hhvm=*/ true,
            |config, key| Ok(config.get_uint32(key)?),
        )
    }

    fn get_helper<'a, T: 'a>(
        &'a self,
        key: &str,
        prepend_hhvm: bool,
        mut f: impl FnMut(&'a hdf::Value, &str) -> Result<Option<T>>,
    ) -> Result<Option<T>> {
        match f(&self.hdf_config, key)? {
            Some(value) => Ok(Some(value)),
            None => {
                let ini_name = Self::ini_name(key, prepend_hhvm);
                f(&self.ini_config, &ini_name)
            }
        }
    }

    fn ini_name(name: &str, prepend_hhvm: bool) -> String {
        // Based on IniName() in config.cpp this basically converts CamelCase to
        // snake_case.

        let mut out = String::new();
        if prepend_hhvm {
            out.push_str("hhvm.");
        }

        if name.is_empty() {
            return out;
        }

        let mut prev = ' ';
        let mut it = name.chars();
        let mut c = it.next().unwrap();
        for (idx, next) in it.enumerate() {
            if idx == 0 || !c.is_alphanumeric() {
                // This is the first character, or any `.` or `_ or punctuator is just output
                // with no special behavior.
                out.extend(c.to_lowercase());
            } else if c.is_uppercase() && prev.is_uppercase() && next.is_lowercase() {
                // Handle something like "SSLPort", and c = "P", which will then
                // put the underscore between the "L" and "P".
                out.push('_');
                out.extend(c.to_lowercase());
            } else if c.is_lowercase() && next.is_uppercase() {
                // Handle something like "PathDebug", and c = "h", which will
                // then put the underscore between the "h" and "D".
                out.extend(c.to_lowercase());
                out.push('_');
            } else {
                // Otherwise we just output as lower.
                out.extend(c.to_lowercase());
            }

            prev = c;
            c = next;
        }

        // Last character.
        out.extend(c.to_lowercase());

        out
    }
}

#[test]
fn test_ini_name() {
    assert_eq!(
        HhvmConfig::ini_name("Hack.Lang.AllowUnstableFeatures", true),
        "hhvm.hack.lang.allow_unstable_features"
    );
    assert_eq!(
        HhvmConfig::ini_name("Server.SSLPort", false),
        "server.ssl_port"
    );
    assert_eq!(HhvmConfig::ini_name("PathDebug", false), "path_debug");
}
