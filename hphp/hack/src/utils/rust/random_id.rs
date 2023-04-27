// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// Generates a 10-digit random alphanumeric string using rand::thread_rng
pub fn short_string() -> String {
    generate_alphanumeric(10, rand::thread_rng())
}

fn generate_alphanumeric(len: usize, mut rng: rand::rngs::ThreadRng) -> String {
    use rand::Rng;
    const ALPHANUMERIC: &[u8] = b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    (0..len)
        .map(|_| ALPHANUMERIC[rng.gen_range(0..ALPHANUMERIC.len())] as char)
        .collect()
}
