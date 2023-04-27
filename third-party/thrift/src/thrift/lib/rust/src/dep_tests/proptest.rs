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

use std::collections::BTreeMap;

use fbthrift_test_if::En;
use fbthrift_test_if::MainStruct;
use fbthrift_test_if::Small;
use fbthrift_test_if::SubStruct;
use fbthrift_test_if::Un;
use fbthrift_test_if::UnOne;
use fbthrift_test_if::UnTwo;
use proptest::prelude::*;

prop_compose! {
    pub(crate) fn gen_map()(
        m in prop::collection::btree_map(".*", 1..1000i32, 1..10)
    ) -> BTreeMap<String, i32> { m }
}

prop_compose! {
    pub(crate) fn gen_small()(
        num in 1..1000i32, two in 1..1000i64
    ) -> Small { Small {num, two, ..Default::default()} }
}

prop_compose! {
    pub(crate) fn gen_small_vec()(
        m in prop::collection::vec(gen_small(), 1..100)
    ) -> Vec<Small> { m }
}

prop_compose! {
    pub(crate) fn gen_substruct()(
        opt_def in prop::option::of(".*"),
        req_def in ".*",
        key_map in prop::option::of(prop::collection::btree_map(gen_small(), 1..1000i32, 1..10)),
        bin in prop::collection::vec(0..255u8, 1..200)
    ) -> SubStruct {SubStruct { optDef: opt_def, req_def, key_map, bin, ..Default::default()}}
}

pub(crate) fn gen_union() -> impl Strategy<Value = Un> {
    prop_oneof![
        any::<i32>().prop_map(|one| Un::un1(UnOne {
            one,
            ..Default::default()
        })),
        any::<i32>().prop_map(|two| Un::un2(UnTwo {
            two,
            ..Default::default()
        })),
    ]
}

pub(crate) fn gen_enum() -> impl Strategy<Value = En> {
    prop_oneof![Just(En::ONE), Just(En::TWO),]
}

prop_compose! {
    pub(crate) fn gen_main_struct()(
        foo in ".*",
        m in gen_map(),
        bar in ".*",
        l in gen_small_vec(),
        s in gen_substruct(),
        int_keys in prop::collection::btree_map(1..1000i32, 1..1000i32, 1..10),
        opt in prop::option::of(".*"),
        u in gen_union(),
        e in gen_enum()
    ) -> MainStruct {
        MainStruct {
            foo,
            m,
            bar,
            l,
            s,
            int_keys,
            opt,
            u,
            e,
            ..Default::default()
        }
    }
}
