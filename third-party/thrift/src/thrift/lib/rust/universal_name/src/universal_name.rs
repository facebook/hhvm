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

pub fn get_universal_hash_prefix_sha_256(uri: &str, hash_bytes: i8) -> Result<Vec<u8>> {
    let hash = get_universal_hash(UniversalHashAlgorithm::Sha2_256, uri)?;
    Ok(get_universal_hash_prefix(&hash, hash_bytes))
}

#[cfg(test)]
mod tests {
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
}
