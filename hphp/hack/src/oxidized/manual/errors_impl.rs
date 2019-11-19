// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::errors::*;
use crate::pos::Pos;

impl<P> Error_<P> {
    pub fn new(code: ErrorCode, messages: Vec<Message<P>>) -> Self {
        Error_(code, messages)
    }
}

impl Errors {
    pub fn empty() -> Self {
        Errors(FilesT::new(), FilesT::new())
    }
}

impl Naming {
    pub fn fd_name_already_bound(p: Pos) -> Error {
        Error::new(
            Self::FdNameAlreadyBound as isize,
            vec![(p, "Field name already bound".into())],
        )
    }

    pub fn method_needs_visibility(p: Pos) -> Error {
        Error::new(
            Self::MethodNeedsVisibility as isize,
            vec![(
                p,
                "Methods need to be marked public, private, or protected.".into(),
            )],
        )
    }

    pub fn unsupported_trait_use_as(p: Pos) -> Error {
        Error::new(
            Self::UnsupportedTraitUseAs as isize,
            vec![(
                p,
                "Trait use as is a PHP feature that is unsupported in Hack".into(),
            )],
        )
    }

    pub fn unsupported_instead_of(p: Pos) -> Error {
        Error::new(
            Self::UnsupportedInsteadOf as isize,
            vec![(
                p,
                "insteadof is a PHP feature that is unsupported in Hack".into(),
            )],
        )
    }

    pub fn invalid_trait_use_as_visibility(p: Pos) -> Error {
        Error::new(
            Self::InvalidTraitUseAsVisibility as isize,
            vec![(
                p,
                "Cannot redeclare trait method's visibility in this manner".into(),
            )],
        )
    }
}

impl NastCheck {
    pub fn not_abstract_without_typeconst(p: Pos) -> Error {
        Error::new(
            Self::NotAbstractWithoutTypeconst as isize,
            vec![(
                p,
                "This type constant is not declared as abstract, it must have an assigned type"
                    .into(),
            )],
        )
    }

    pub fn multiple_xhp_category(p: Pos) -> Error {
        Error::new(
            Self::MultipleXhpCategory as isize,
            vec![(
                p,
                "XHP classes can only contain one category declaration".into(),
            )],
        )
    }
}
