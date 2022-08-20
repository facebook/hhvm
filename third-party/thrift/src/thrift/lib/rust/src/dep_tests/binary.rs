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
use fbthrift::binary_protocol::deserialize;
use fbthrift::binary_protocol::serialize;
use fbthrift::ttype::TType;
use fbthrift_test_if::Un;
use proptest::prelude::*;

use crate::proptest::gen_main_struct;

#[test]
fn test_unknown_union() -> Result<()> {
    // Build the empty union
    let u = Un::default();

    let s = serialize(&u);
    // only TType::Stop
    assert_eq!(&[TType::Stop as u8], s.as_ref());

    // Assert that deserialize builds the exact some struct
    assert_eq!(u, deserialize(s).unwrap());

    // ...
    // extra weirdness
    // Build an explicit unknown
    let explicit_unknown = Un::UnknownField(100);
    let s2 = serialize(&explicit_unknown);
    // only Stop
    assert_eq!(&[TType::Stop as u8], s2.as_ref());

    Ok(())
}

proptest! {
#[test]
fn test_prop_serialize_deserialize(s in gen_main_struct()) {
    let processed = deserialize(serialize(&s)).unwrap();
    prop_assert_eq!(s, processed);
}
}
