// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[macro_use]
extern crate ocaml;

use decl_rust as decl;

caml!(hello_world() {
    use ocaml::ToValue;
    return decl::hello_world().to_value();
});
