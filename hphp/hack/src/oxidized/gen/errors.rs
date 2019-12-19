// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<0635bbb7dec587b586f0ebf3c7cd2c32>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;

use crate::error_codes;
use crate::phase_map;
use crate::pos;
use crate::relative_path;

pub use error_codes::Naming;
pub use error_codes::NastCheck;
pub use error_codes::Parsing;
pub use error_codes::Typing;

pub type ErrorCode = isize;

pub type Message<A> = (A, String);

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ord, PartialEq, PartialOrd)]
pub enum Phase {
    Init,
    Parsing,
    Naming,
    Decl,
    Typing,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
pub enum Severity {
    Warning,
    Error,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
pub enum Format {
    Context,
    Raw,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
pub enum NameContext {
    FunctionNamespace,
    ConstantNamespace,
    TypeNamespace,
    TraitContext,
    ClassContext,
    RecordContext,
}

pub type FileT<A> = phase_map::PhaseMap<Vec<A>>;

pub type FilesT<A> = relative_path::map::Map<FileT<A>>;

#[derive(Clone, Debug, OcamlRep)]
pub struct Error_<A>(pub ErrorCode, pub Vec<Message<A>>);

pub type Error = Error_<pos::Pos>;

#[derive(Clone, Debug, OcamlRep)]
pub struct AppliedFixme(pub pos::Pos, pub isize);

#[derive(Clone, Debug, OcamlRep)]
pub struct Errors(pub FilesT<Error>, pub FilesT<AppliedFixme>);
