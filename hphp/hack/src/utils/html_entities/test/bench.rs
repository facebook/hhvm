// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![feature(test)]
#[cfg(test)]
extern crate test;

fn main() {
    let command =
        "buck run @mode/opt //hphp/hack/src/utils/html_entities/tests:bench-unittest -- --bench";
    println!("Run this bench suite with: {}", command);
}

#[cfg(test)]
mod tests {
    use html_entities::*;

    use test::Bencher;

    #[bench]
    fn bench_last(b: &mut Bencher) {
        b.iter(|| {
            for _i in 0..100 {
                decode("comp".as_bytes());
            }
        });
    }

    #[bench]
    fn bench_first(b: &mut Bencher) {
        b.iter(|| {
            for _i in 0..100 {
                decode("bsim".as_bytes());
            }
        });
    }
}
