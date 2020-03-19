// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_ocamlpool::ocaml_ffi_no_panic;
use options::Options;

extern crate fnv;
extern crate lru_cache;

use lru_cache::LruCache;

use std::cell::RefCell;

thread_local! {
  /// Cache for keeping 100 Last Recently Used results of configs_to_json_ffi
  static CACHE: RefCell<LruCache::<(String, String), String, fnv::FnvBuildHasher>> = {
    let hasher = fnv::FnvBuildHasher::default();
    RefCell::new(LruCache::with_hasher(100, hasher))
  }
}

ocaml_ffi_no_panic! {
    fn configs_to_json_ffi(
        jsons: Vec<String>,
        cli_args: Vec<String>,
    ) -> String {
        CACHE.with(|map| {
            // Note: combine CLI arguments without collision by joining on null:
            // https://doc.rust-lang.org/reference/tokens.html#ascii-escapes
            // (joining JSON objects ({...}) on "" is already unambiguous)
            let key = (jsons.join(""), cli_args.join("\0"));
            let mut map = map.borrow_mut();
            if !map.contains_key(&key) {
                // FIXME(hrust) ocaml_ffi_no_panic disallows "mut" in parameter
                // jsons.reverse();
                let jsons: Vec<&String> = jsons.iter().rev().collect();

                let val = Options::from_configs(&jsons, &cli_args)
                .expect("bug in deserializing Hhbc_options from Rust")
                .to_string();
                map.insert(key, val.clone());
                val
            } else {
                map.get_mut(&key).unwrap().to_owned()
            }
        })
    }
}
