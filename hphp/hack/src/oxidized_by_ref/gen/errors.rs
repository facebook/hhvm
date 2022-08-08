// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<02b2a56254bcbab0fd4cafa2b457e3e4>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

pub use oxidized::errors::ErrorCode;
pub use oxidized::errors::Format;
pub use oxidized::errors::Phase;

pub use crate::error_codes::GlobalWriteCheck;
pub use crate::error_codes::Naming;
pub use crate::error_codes::NastCheck;
pub use crate::error_codes::Parsing;
pub use crate::error_codes::Typing;
#[allow(unused_imports)]
use crate::*;

/// Results of single file analysis.
pub type FileT<'a, A> = phase_map::PhaseMap<'a, &'a [A]>;

/// Results of multi-file analysis.
pub type FilesT<'a, A> = relative_path::map::Map<'a, FileT<'a, A>>;

pub type Error<'a> = user_error::UserError<'a, &'a pos::Pos<'a>, &'a pos_or_decl::PosOrDecl<'a>>;

pub type PerFileErrors<'a> = FileT<'a, &'a Error<'a>>;

pub type Errors<'a> = FilesT<'a, &'a Error<'a>>;
