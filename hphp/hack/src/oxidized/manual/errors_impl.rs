// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;

use crate::errors::*;
use crate::message::Message;
use crate::pos::Pos;
use crate::quickfix::Edits;
use crate::quickfix::Quickfix;
use crate::user_error::UserError;
use crate::user_error_flags::UserErrorFlags;

impl Default for UserErrorFlags {
    fn default() -> Self {
        Self {
            stripped_existential: false,
        }
    }
}

impl<PP, P> UserError<PP, P> {
    pub fn new(
        code: ErrorCode,
        claim: Message<PP>,
        reasons: Vec<Message<P>>,
        custom_msgs: Vec<String>,
        quickfixes: Vec<Quickfix<PP>>,
        flags: UserErrorFlags,
    ) -> Self {
        Self {
            code,
            claim,
            reasons,
            quickfixes,
            custom_msgs,
            is_fixmed: false,
            flags,
        }
    }

    pub fn pos(&self) -> &PP {
        let Message(pos, _msg) = &self.claim;
        pos
    }

    pub fn msg(&self) -> &bstr::BStr {
        let Message(_, msg) = &self.claim;
        msg.as_ref()
    }

    pub fn code(&self) -> ErrorCode {
        self.code
    }
}

impl<PP: Ord + FileOrd, P: Ord + FileOrd> UserError<PP, P> {
    // Intended to match the implementation of `compare` in `Errors.sort` in OCaml.
    pub fn cmp_impl(&self, other: &Self, by_phase: bool) -> Ordering {
        let Self {
            code: self_code,
            claim: Message(self_pos, self_msg),
            reasons: self_reasons,
            custom_msgs: _,
            quickfixes: _,
            is_fixmed: _,
            flags: _,
        } = self;
        let Self {
            code: other_code,
            claim: Message(other_pos, other_msg),
            reasons: other_reasons,
            custom_msgs: _,
            quickfixes: _,
            is_fixmed: _,
            flags: _,
        } = other;
        let compare_code = |self_code: ErrorCode, other_code: ErrorCode| {
            if by_phase {
                (self_code / 1000).cmp(&{ other_code / 1000 })
            } else {
                self_code.cmp(&other_code)
            }
        };
        // The primary sort order is by file of the claim (main message).
        self_pos
            .cmp_file(other_pos)
            // If the files are the same, sort by error code or phase, depending on parameter.
            .then(compare_code(*self_code, *other_code))
            // If the phases are the same, sort by position.
            .then(self_pos.cmp(other_pos))
            // If the positions are the same, sort by claim message text.
            .then(self_msg.cmp(other_msg))
            // If the claim message text is the same, compare the reason
            // messages (which contain further explanation for the error
            // reported in the claim message).
            .then(self_reasons.iter().cmp(other_reasons.iter()))
    }
}

pub trait FileOrd {
    fn cmp_file(&self, other: &Self) -> Ordering;
}

impl FileOrd for Pos {
    fn cmp_file(&self, other: &Self) -> Ordering {
        self.filename().cmp(other.filename())
    }
}

impl<PP: Ord + FileOrd, P: Ord + FileOrd> Ord for UserError<PP, P> {
    fn cmp(&self, other: &Self) -> Ordering {
        self.cmp_impl(other, false)
    }
}

impl<PP: Ord + FileOrd, P: Ord + FileOrd> PartialOrd for UserError<PP, P> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp_impl(other, false))
    }
}

impl Naming {
    pub fn fd_name_already_bound(p: Pos) -> Error {
        UserError::new(
            Self::FdNameAlreadyBound as isize,
            Message(p, "Field name already bound".into()),
            vec![],
            vec![],
            vec![],
            Default::default(),
        )
    }

    pub fn bad_builtin_type(p: Pos, name: &str, correct_name: &str) -> Error {
        UserError::new(
            Self::InvalidBuiltinType as isize,
            Message(
                p.clone(),
                format!("No such type `{}`, did you mean `{}`?", name, correct_name).into(),
            ),
            vec![],
            vec![],
            vec![Quickfix {
                title: format!("Change to `{}`", correct_name),
                edits: Edits::Eager(vec![(correct_name.into(), p)]),
            }],
            Default::default(),
        )
    }

    pub fn method_needs_visibility(first_token_p: Pos, name_p: Pos) -> Error {
        // Create a zero width position at the start of the first token.
        let file = first_token_p.filename_rc();
        let mut p_span = first_token_p.to_raw_span();
        p_span.end = p_span.start;
        let fix_pos = Pos::from_raw_span(file, p_span);

        UserError::new(
            Self::MethodNeedsVisibility as isize,
            Message(
                name_p,
                "Methods need to be marked `public`, `private`, or `protected`.".into(),
            ),
            vec![],
            vec![],
            vec![
                Quickfix {
                    title: "Add `private` modifier".into(),
                    edits: Edits::Eager(vec![("private ".into(), fix_pos.clone())]),
                },
                Quickfix {
                    title: "Add `protected` modifier".into(),
                    edits: Edits::Eager(vec![("protected ".into(), fix_pos.clone())]),
                },
                Quickfix {
                    title: "Add `public` modifier".into(),
                    edits: Edits::Eager(vec![("public ".into(), (fix_pos.clone()))]),
                },
            ],
            Default::default(),
        )
    }

    pub fn unsupported_trait_use_as(p: Pos) -> Error {
        UserError::new(
            Self::UnsupportedTraitUseAs as isize,
            Message(
                p,
                "Trait use as is a PHP feature that is unsupported in Hack".into(),
            ),
            vec![],
            vec![],
            vec![],
            Default::default(),
        )
    }

    pub fn unsupported_instead_of(p: Pos) -> Error {
        UserError::new(
            Self::UnsupportedInsteadOf as isize,
            Message(
                p,
                "insteadof is a PHP feature that is unsupported in Hack".into(),
            ),
            vec![],
            vec![],
            vec![],
            Default::default(),
        )
    }
}

impl NastCheck {
    pub fn not_abstract_without_typeconst(p: Pos) -> Error {
        UserError::new(
            Self::NotAbstractWithoutTypeconst as isize,
            Message(
                p,
                "This type constant is not declared as abstract, it must have an assigned type"
                    .into(),
            ),
            vec![],
            vec![],
            vec![],
            Default::default(),
        )
    }

    pub fn multiple_xhp_category(p: Pos) -> Error {
        UserError::new(
            Self::MultipleXhpCategory as isize,
            Message(
                p,
                "XHP classes can only contain one category declaration".into(),
            ),
            vec![],
            vec![],
            vec![],
            Default::default(),
        )
    }

    pub fn partially_abstract_typeconst_definition(p: Pos, kind: &str) -> Error {
        UserError::new(
            Self::PartiallyAbstractTypeconstDefinition as isize,
            Message(
                p,
                format!(
                    "`{}` constraints are only legal on abstract type constants",
                    kind
                )
                .into(),
            ),
            vec![],
            vec![],
            vec![],
            Default::default(),
        )
    }
}

#[derive(Debug)]
pub struct ParseFormatError;

impl std::str::FromStr for Format {
    type Err = ParseFormatError;
    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "context" => Ok(Self::Context),
            "raw" => Ok(Self::Raw),
            "highlighted" => Ok(Self::Highlighted),
            "plain" => Ok(Self::Plain),
            _ => Err(ParseFormatError),
        }
    }
}

impl std::fmt::Display for Format {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Context => "context".fmt(f),
            Self::Raw => "raw".fmt(f),
            Self::Highlighted => "highlighted".fmt(f),
            Self::Plain => "plain".fmt(f),
        }
    }
}

impl std::error::Error for ParseFormatError {}

impl std::fmt::Display for ParseFormatError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        "Unrecognized error format".fmt(f)
    }
}

impl Default for Format {
    fn default() -> Self {
        Format::Plain
    }
}
