// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use depgraph_api::DeclName;

pub type Result<T, E = Error> = std::result::Result<T, E>;

/// A system error preventing us from proceeding with typechecking. When we
/// encounter one during a bulk typecheck, we should abort the check, report the
/// error to the user, and log the error to Scuba. In some circumstances (e.g.,
/// decl-consistency errors), we might attempt the bulk check again. Includes
/// decl-provider errors like file-not-found (even though it was listed in our
/// global symbol table), decl-consistency errors (i.e., we detected that a
/// source file on disk changed under our feet), etc.
///
/// This type should not be used for internal compiler errors (i.e., invariant
/// violations in our own logic). In OCaml, those are represented as exceptions
/// which are caught at the typing entrypoint and reported as a Hack error
/// (i.e., `Typing_error.invariant_violation`). In Rust, we should represent
/// these with a panic.
#[derive(thiserror::Error, Debug)]
pub enum Error {
    #[error("{0}")]
    DeclProvider(#[from] crate::typing_decl_provider::Error),
    #[error("Decl Not Found: {0:?}")]
    DeclNotFound(DeclName),
    #[error("An invariant expected after the naming phase was violated")]
    NamingInvariantViolated,
}
