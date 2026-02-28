// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use rc_pos::with_erased_lines::WithErasedLines;

use crate::quickfix::Edits;
use crate::quickfix::HintStyle;
use crate::quickfix::Quickfix;

impl<P: WithErasedLines> WithErasedLines for Edits<P> {
    fn with_erased_lines(self) -> Self {
        match self {
            Self::ClassishEnd { new_text, name } => Self::ClassishEnd { new_text, name },
            Self::Eager(v) => Self::Eager(
                v.into_iter()
                    .map(|(s, p)| (s, p.with_erased_lines()))
                    .collect(),
            ),
            Self::AddFunctionAttribute {
                function_pos,
                attribute_name,
            } => Self::AddFunctionAttribute {
                function_pos: function_pos.with_erased_lines(),
                attribute_name,
            },
        }
    }
}

impl<P: WithErasedLines> WithErasedLines for HintStyle<P> {
    fn with_erased_lines(self) -> Self {
        match self {
            Self::HintStyleHint(p) => Self::HintStyleHint(p.with_erased_lines()),
            Self::HintStyleSilent(p) => Self::HintStyleSilent(p.with_erased_lines()),
        }
    }
}

impl<P: WithErasedLines> WithErasedLines for Quickfix<P> {
    fn with_erased_lines(self) -> Self {
        let Self {
            title,
            edits,
            hint_styles,
        } = self;
        Self {
            title,
            edits: edits.with_erased_lines(),
            hint_styles: hint_styles
                .into_iter()
                .map(|hs| hs.with_erased_lines())
                .collect(),
        }
    }
}
