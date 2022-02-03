#![allow(dead_code)]
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
//#![feature(const_fn_trait_bound)]

/// Are we JIT, AOT, or compiling code that needs to work with both?
#[derive(Debug, Clone, Copy)]
pub enum CompilerMode {
    Aot,
    Both,
    Jit,
}

impl Default for CompilerMode {
    fn default() -> Self {
        // Default to being noncommital.
        CompilerMode::Both
    }
}

#[derive(Debug)]
pub struct HhvmConfig {
    pub hdf_config: hdf::Value,
    pub ini_config: hdf::Value,
}

impl HhvmConfig {
    pub fn get_str<'a>(&'a self, key: &str) -> Option<&'a str> {
        self.get_helper(
            key,
            /*prepend_hhvm=*/ true,
            |config, key| config.get_str(key),
        )
    }

    pub fn get_bool(&self, key: &str) -> Option<bool> {
        self.get_helper(
            key,
            /*prepend_hhvm=*/ true,
            |config, key| config.get_bool(key).ok().flatten(),
        )
    }

    pub fn enumerate<'a>(&'a self, key: &str) -> Vec<&'a str> {
        let mut result: Vec<&str> = Vec::new();
        if let Some(value) = self.hdf_config.get(key) {
            for k in value.keys() {
                result.push(k);
            }
        }

        let ini_name = Self::ini_name(key, /*prepend_hhvm=*/ true);
        if let Some(value) = self.ini_config.get(&ini_name) {
            for k in value.keys() {
                result.push(k);
            }
        }

        result
    }

    fn get_helper<'a, T: 'a, F: Fn(&'a hdf::Value, &str) -> Option<T>>(
        &'a self,
        key: &str,
        prepend_hhvm: bool,
        f: F,
    ) -> Option<T> {
        if let Some(value) = f(&self.hdf_config, key) {
            return Some(value);
        }

        let ini_name = Self::ini_name(key, prepend_hhvm);
        f(&self.ini_config, &ini_name)
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

impl std::default::Default for HhvmConfig {
    fn default() -> Self {
        Self {
            hdf_config: hdf::Value::new(),
            ini_config: hdf::Value::new(),
        }
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

#[derive(Clone, Debug)]
pub struct ParseConfig {
    /// -vHack.Lang.AllowUnstableFeatures=1
    pub allow_unstable_features: bool,
}
