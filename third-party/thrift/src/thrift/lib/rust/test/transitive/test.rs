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

use get_struct_name::types::GetStructName as GetStructNameThrift;
use get_struct_name::GetStructName;
use get_struct_name::Wrapper;
use thrift_test::types::Foo;

#[test]
fn test_struct_name() {
    assert_eq!(<Foo as GetStructName>::get_struct_name(), "Foo");
    assert_eq!(
        <GetStructNameThrift as GetStructName>::get_struct_name(),
        "GetStructName"
    );
}

#[test]
fn test_adapter() {
    let foo = Foo::default();

    assert_eq!(foo.bar, Wrapper("hello".to_string()));
}
