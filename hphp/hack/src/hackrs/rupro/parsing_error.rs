// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use parser_core_types::syntax_error::SyntaxError;
use pos::RelativePath;

#[derive(Debug, Clone)]
pub enum ParsingError {
    FixmeFormat(oxidized::pos::Pos),
    HhIgnoreComment(oxidized::pos::Pos),
    ParsingError {
        pos: oxidized::pos::Pos,
        msg: String,
    },
    XhpParsingError {
        pos: oxidized::pos::Pos,
        msg: String,
    },
    IO {
        file: RelativePath,
        err: String,
    },
    NotAHackFile {
        file: RelativePath,
    },
    ParserFatal {
        pos: oxidized::pos::Pos,
        err: SyntaxError,
    },
    Other {
        file: RelativePath,
        msg: String,
    },
    StackExceeded {
        file: RelativePath,
        msg: String,
    },
}
