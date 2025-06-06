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

use bytes::Bytes;
use fbthrift::BinaryProtocol;
use fbthrift::Serialize;
use fbthrift::serialize;
use interface::TestSkipFull;
use interface::TestSkipNested;
use interface::TestSkipNestedDeep;

#[macro_export]
macro_rules! btreeset {
    ($($val:expr),* $(,)?) => {{
        let mut set = ::std::collections::BTreeSet::new();
        $( set.insert($val); )*
        set
    }};
}

#[macro_export]
macro_rules! btreemap {
    ($($key:expr => $value:expr),* $(,)?) => {{
        let mut map = ::std::collections::BTreeMap::new();
        $( map.insert($key, $value); )*
        map
    }};
}

pub fn serialize_test_skip_full() -> Bytes {
    let nested = TestSkipNested {
        l: vec![100; 100],
        s: btreeset![1, 2],
        m: btreemap!("a".to_owned() => "b".to_owned(), "c".to_owned() => "d".to_owned(), "e".to_owned() => "f".to_owned()),
        ..TestSkipNested::default()
    };
    let nested_deep = TestSkipNestedDeep {
        nested: nested.clone(),
        nested_list: vec![nested.clone(), nested.clone()],
        ..TestSkipNestedDeep::default()
    };
    let full = TestSkipFull {
        l: vec![1, 2, 3],
        s: btreeset![1, 2, 3],
        m_str: btreemap!("a".to_owned() => "b".to_owned(), "c".to_owned() => "d".to_owned()),
        nested: nested.clone(),
        nested_list: vec![nested.clone(), nested.clone()],
        bl: true,
        m_int: btreemap!(1 => 2, 3 => 4),
        m_nested: btreemap!(1 => nested),
        l_nested2: vec![nested_deep.clone(), nested_deep],
        l_double: vec![1.0; 20],
        l_float: vec![2.0; 30],
        b: 123,
        ..TestSkipFull::default()
    };

    serialize!(BinaryProtocol, |w| Serialize::rs_thrift_write(&full, w))
}
