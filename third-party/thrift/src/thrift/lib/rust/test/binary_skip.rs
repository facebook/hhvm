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

use std::io::Cursor;

use bytes::Bytes;
use criterion::Criterion;
use criterion::black_box;
use criterion::criterion_group;
use criterion::criterion_main;
use fbthrift::BinaryProtocol;
use fbthrift::Deserialize;
use fbthrift::Protocol;
use interface::TestSkipMinimal;
use test_helper::serialize_test_skip_full;

mod test_helper;

fn bench_fast_skip(c: &mut Criterion) {
    let bytes = serialize_test_skip_full();

    c.bench_function("binary_fast_skip", |b| {
        b.iter(|| {
            let buf = Cursor::new(black_box(bytes.clone()));
            let mut de = BinaryProtocol::<Bytes>::deserializer(buf);
            let minimal: TestSkipMinimal =
                Deserialize::rs_thrift_read(&mut de).expect("deserialization failed");

            black_box(minimal);
        });
    });
}

fn configure_criterion() -> Criterion {
    Criterion::default()
        .with_plots()
        .measurement_time(std::time::Duration::from_secs(5))
        .configure_from_args()
}

criterion_group! {
    name = benches;
    config = configure_criterion();
    targets = bench_fast_skip
}
criterion_main!(benches);
