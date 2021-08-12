// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[derive(Clone, Copy, Debug)]
#[repr(C)]
pub struct HhasPos {
    pub line_begin: usize,
    pub line_end: usize,
    pub col_begin: usize,
    pub col_end: usize,
}
impl Default for HhasPos {
    fn default() -> HhasPos {
        let (line_begin, line_end, col_begin, col_end) = (1, 1, 0, 0);
        HhasPos {
            line_begin,
            line_end,
            col_begin,
            col_end,
        }
    }
}
impl std::convert::From<oxidized::pos::Pos> for HhasPos {
    fn from(p: oxidized::pos::Pos) -> HhasPos {
        let (line_begin, line_end, col_begin, col_end) = if p.is_none() || !p.is_valid() {
            (1, 1, 0, 0)
        } else {
            p.info_pos_extended()
        };
        HhasPos {
            line_begin,
            line_end,
            col_begin,
            col_end,
        }
    }
}

/// Span, emitted as prefix to classes and functions
#[derive(Clone, Copy, Debug, Default)]
#[repr(C)]
pub struct HhasSpan(pub usize, pub usize);

impl HhasSpan {
    pub fn from_pos(pos: &oxidized::pos::Pos) -> HhasSpan {
        let (line_begin, line_end, _, _) = pos.info_pos_extended();
        HhasSpan(line_begin, line_end)
    }
}

// For cbindgen
#[allow(clippy::needless_lifetimes)]
#[no_mangle]
pub unsafe extern "C" fn no_call_compile_only_USED_TYPES_hhas_pos<'arena>(_: HhasPos, _: HhasSpan) {
    unimplemented!()
}
