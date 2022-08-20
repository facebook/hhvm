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

use std::collections::HashSet;

use anyhow::Result;
use cxx::let_cxx_string;
pub use ffi::UniversalHashAlgorithm;

use crate::bridge::ffi;

pub fn get_universal_hash(alg: UniversalHashAlgorithm, uri: &str) -> Result<Vec<u8>> {
    let_cxx_string!(uri = uri);

    ffi::getUniversalHash(alg, &uri)
        .map(|hash| hash.as_bytes().to_vec())
        .map_err(anyhow::Error::from)
}

pub fn get_universal_hash_prefix(universal_hash: &[u8], hash_bytes: i8) -> Vec<u8> {
    let_cxx_string!(universal_hash = universal_hash);

    ffi::getUniversalHashPrefix(&universal_hash, hash_bytes)
        .as_bytes()
        .to_vec()
}

pub fn ensure_registered(
    universal_hash_registry: &HashSet<Vec<u8>>,
    hash_prefix: &[u8],
) -> Result<()> {
    let num_matched = universal_hash_registry
        .iter()
        .filter(|hash| matches_universal_hash(hash, hash_prefix))
        .count();
    match num_matched {
        0 => Err(anyhow::anyhow!(
            "No hash found with prefix {:?}",
            hash_prefix
        )),
        1 => Ok(()),
        _ => Err(anyhow::anyhow!(
            "Multiple hashes found with prefix {:?}",
            hash_prefix
        )),
    }
}

fn matches_universal_hash(universal_hash: &[u8], prefix: &[u8]) -> bool {
    let_cxx_string!(universal_hash = universal_hash);
    let_cxx_string!(prefix = prefix);

    ffi::matchesUniversalHash(&universal_hash, &prefix)
}

#[cfg(test)]
mod tests {
    use maplit::hashset;

    use super::*;

    #[test]
    fn test_get_universal_hash() {
        assert_eq!(
            get_universal_hash(UniversalHashAlgorithm::Sha2_256, "foo.com/my/type").unwrap(),
            b"\tat$\x9c\xef\xad\xb5\xea\rE;\xcb3\xadTv\x01\xfb\xfe\xc4\xb2\xd7\x95\x92N\xebg\xd4[\xe6F",
        );
    }

    #[test]
    fn test_get_universal_hash_prefix() {
        let hash = b"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

        assert_eq!(get_universal_hash_prefix(hash, 0), b"",);

        assert_eq!(get_universal_hash_prefix(hash, 8), b"aaaaaaaa",);

        assert_eq!(get_universal_hash_prefix(hash, 32), hash.to_vec(),);

        assert_eq!(get_universal_hash_prefix(hash, 33), hash.to_vec(),);
    }

    #[test]
    fn test_match_universal_hash() {
        assert!(!matches_universal_hash(
            b"0123456789ABCDEF0123456789ABCDEF",
            b""
        ));

        assert!(!matches_universal_hash(
            b"0123456789ABCDEF0123456789ABCDEF",
            b"1"
        ));

        assert!(matches_universal_hash(
            b"0123456789ABCDEF0123456789ABCDEF",
            b"0"
        ));

        assert!(matches_universal_hash(
            b"0123456789ABCDEF0123456789ABCDEF",
            b"0123456789ABCDEF"
        ));

        assert!(matches_universal_hash(
            b"0123456789ABCDEF0123456789ABCDEF",
            b"0123456789ABCDEF0123456789ABCDEF"
        ));

        assert!(!matches_universal_hash(
            b"0123456789ABCDEF0123456789ABCDEF",
            b"0123456789ABCDEF0123456789ABCDEF0",
        ));
    }

    #[test]
    fn test_ensure_registered() {
        let universal_hash_registry = hashset! {
            b"DEADBEEF".to_vec(),
            b"0123456789ABCDEF0123456789ABCDEF".to_vec(),
            b"0123456789ABCDEF".to_vec(),
        };

        // Test no matching prefix
        assert!(ensure_registered(&universal_hash_registry, b"").is_err());

        assert!(ensure_registered(&universal_hash_registry, b"12345").is_err());

        // Test multiple matches
        assert!(ensure_registered(&universal_hash_registry, b"012345").is_err());

        // Test single matches
        assert!(ensure_registered(&universal_hash_registry, b"DEAD").is_ok());
    }
}
