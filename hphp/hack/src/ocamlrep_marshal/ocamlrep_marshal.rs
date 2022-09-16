// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![feature(extern_types, new_uninit, let_chains)]

mod deser; // deserialize; c.f 'runtime/intern.c'
mod intext; // c.f. 'runtime/caml/intext.h'
mod ser; // serialize; c.f. 'runtime/extern.c'

pub use deser::input_value;
pub use ser::output_value;
pub use ser::ExternFlags;
