// Copyright (c) 2021, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;
use framing::LineFeedEscaper;
use lazy_static::lazy_static;
use ocamlrep_ocamlpool::ocaml_ffi_with_arena;
use oxidized_by_ref::typing_defs_core;

ocaml_ffi_with_arena! {
    fn ty_to_stdout<'a>(
        arena: &'a Bump,
        ty: typing_defs_core::Ty<'_>,
    ) {
        lazy_static! {
            static ref LF_ESCAPER: LineFeedEscaper = LineFeedEscaper::new(false);
        }
        let op = bincode::config::Options::with_native_endian(bincode::options());
        use bincode::Options;
        let bs = op.serialize(&ty).unwrap();

        let bs = lz4::block::compress(&bs, None, true).unwrap();
        let bs = LF_ESCAPER.escape(&bs);
        use std::io::Write;
        let mut stdio = std::io::stdout();
        stdio.write_all(&bs).unwrap();
        stdio.write_all(b"\n").unwrap();
        stdio.flush().unwrap();
    }
}
