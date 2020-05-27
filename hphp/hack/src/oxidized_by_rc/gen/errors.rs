// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<63675561385d79edc101dcfe5737239c>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_rc/regen.sh

use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::error_codes::Naming;
pub use crate::error_codes::NastCheck;
pub use crate::error_codes::Parsing;
pub use crate::error_codes::Typing;

pub use oxidized::errors::ErrorCode;

/// We use `Pos.t message` on the server and convert to `Pos.absolute message`
/// before sending it to the client
pub type Message<A> = (A, std::rc::Rc<String>);

pub use oxidized::errors::Phase;

pub use oxidized::errors::Severity;

pub use oxidized::errors::Format;

pub use oxidized::errors::NameContext;

/// Results of single file analysis.
pub type FileT<A> = phase_map::PhaseMap<Vec<A>>;

/// Results of multi-file analysis.
pub type FilesT<A> = relative_path::map::Map<FileT<A>>;

#[derive(Clone, Debug, Eq, Hash, PartialEq, Serialize, ToOcamlRep)]
pub struct Error_<A>(pub oxidized::errors::ErrorCode, pub Vec<Message<A>>);

pub type Error = Error_<std::rc::Rc<pos::Pos>>;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct AppliedFixme(pub std::rc::Rc<pos::Pos>, pub isize);

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Errors(pub FilesT<Error>, pub FilesT<AppliedFixme>);
