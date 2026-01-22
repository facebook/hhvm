// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;

use arena_deserializer::DeserializeInArena;
use ocamlrep::FromOcamlRepIn;
use serde::Deserializer;

use crate::diagnostics::*;
use crate::explanation::Explanation;
use crate::message::Message;
use crate::pos::Pos;
use crate::quickfix::Edits;
use crate::quickfix::Quickfix;
use crate::typing_reason::Axiom;
use crate::user_diagnostic::Severity;
use crate::user_diagnostic::UserDiagnostic;

impl<PP, P> UserDiagnostic<PP, P> {
    pub fn new(
        severity: Severity,
        code: ErrorCode,
        claim: Message<PP>,
        reasons: Vec<Message<P>>,
        explanation: Explanation<P>,
        custom_msgs: Vec<String>,
        quickfixes: Vec<Quickfix<PP>>,
    ) -> Self {
        Self {
            severity,
            code,
            claim,
            reasons,
            explanation,
            quickfixes,
            custom_msgs,
            is_fixmed: false,
            function_pos: None,
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

impl<PP: Ord + FileOrd, P: Ord + FileOrd> UserDiagnostic<PP, P> {
    // Intended to match the implementation of `compare` in `Diagnostics.sort` in OCaml.
    pub fn cmp_impl(&self, other: &Self, ignore_codes: bool) -> Ordering {
        let Self {
            severity: self_severity,
            code: self_code,
            claim: Message(self_pos, self_msg),
            reasons: self_reasons,
            explanation: _,
            custom_msgs: _,
            quickfixes: _,
            is_fixmed: _,
            function_pos: _,
        } = self;
        let Self {
            severity: other_severity,
            code: other_code,
            claim: Message(other_pos, other_msg),
            reasons: other_reasons,
            explanation: _,
            custom_msgs: _,
            quickfixes: _,
            is_fixmed: _,
            function_pos: _,
        } = other;
        let self_phase = self_code / 1000;
        let other_phase = other_code / 1000;
        // /!\ KEEP IN SYNC WITH `Diagnostics.compare_internal` IN OCaml. /!\
        // The primary sort order is by file of the claim (main message).
        self_severity
            .cmp(other_severity)
            .then(self_pos.cmp_file(other_pos))
            // If the files are the same, sort by phase.
            .then(self_phase.cmp(&other_phase))
            // If the phases are the same, sort by position.
            .then(self_pos.cmp(other_pos))
            // If the positions are the same, sort by claim message text.
            .then(self_msg.cmp(other_msg))
            // If the claim message text is the same, compare the reason
            // messages (which contain further explanation for the error
            // reported in the claim message).
            .then(self_reasons.iter().cmp(other_reasons.iter()))
            // Sometimes with have the same errors with different codes. For
            // sorting stability, sort by code at the end.
            .then(if ignore_codes {
                Ordering::Equal
            } else {
                self_code.cmp(other_code)
            })
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

impl<PP: Ord + FileOrd, P: Ord + FileOrd> Ord for UserDiagnostic<PP, P> {
    fn cmp(&self, other: &Self) -> Ordering {
        self.cmp_impl(other, false)
    }
}

impl<PP: Ord + FileOrd, P: Ord + FileOrd> PartialOrd for UserDiagnostic<PP, P> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl Naming {
    pub fn fd_name_already_bound(p: Pos) -> Diagnostic {
        UserDiagnostic::new(
            Severity::Err,
            Self::FdNameAlreadyBound as isize,
            Message(p, "Field name already bound".into()),
            vec![],
            Explanation::Empty,
            vec![],
            vec![],
        )
    }

    pub fn bad_builtin_type(p: Pos, name: &str, correct_name: &str) -> Diagnostic {
        UserDiagnostic::new(
            Severity::Err,
            Self::InvalidBuiltinType as isize,
            Message(
                p.clone(),
                format!("No such type `{}`, did you mean `{}`?", name, correct_name).into(),
            ),
            vec![],
            Explanation::Empty,
            vec![],
            vec![Quickfix {
                title: format!("Change to `{}`", correct_name),
                edits: Edits::Eager(vec![(correct_name.into(), p)]),
                hint_styles: vec![],
            }],
        )
    }

    pub fn method_needs_visibility(first_token_p: Pos, name_p: Pos) -> Diagnostic {
        // Create a zero width position at the start of the first token.
        let file = first_token_p.filename_rc();
        let mut p_span = first_token_p.to_raw_span();
        p_span.end = p_span.start;
        let fix_pos = Pos::from_raw_span(file, p_span);

        UserDiagnostic::new(
            Severity::Err,
            Self::MethodNeedsVisibility as isize,
            Message(
                name_p,
                "Methods need to be marked `public`, `private`, `protected`, or `internal`.".into(),
            ),
            vec![],
            Explanation::Empty,
            vec![],
            vec![
                Quickfix {
                    title: "Add `private` modifier".into(),
                    edits: Edits::Eager(vec![("private ".into(), fix_pos.clone())]),
                    hint_styles: vec![],
                },
                Quickfix {
                    title: "Add `protected` modifier".into(),
                    edits: Edits::Eager(vec![("protected ".into(), fix_pos.clone())]),
                    hint_styles: vec![],
                },
                Quickfix {
                    title: "Add `public` modifier".into(),
                    edits: Edits::Eager(vec![("public ".into(), (fix_pos.clone()))]),
                    hint_styles: vec![],
                },
                Quickfix {
                    title: "Add `internal` modifier".into(),
                    edits: Edits::Eager(vec![("internal ".into(), (fix_pos.clone()))]),
                    hint_styles: vec![],
                },
            ],
        )
    }
}

impl NastCheck {
    pub fn not_abstract_without_typeconst(p: Pos) -> Diagnostic {
        UserDiagnostic::new(
            Severity::Err,
            Self::NotAbstractWithoutTypeconst as isize,
            Message(
                p,
                "This type constant is not declared as abstract, it must have an assigned type"
                    .into(),
            ),
            vec![],
            Explanation::Empty,
            vec![],
            vec![],
        )
    }

    pub fn multiple_xhp_category(p: Pos) -> Diagnostic {
        UserDiagnostic::new(
            Severity::Err,
            Self::MultipleXhpCategory as isize,
            Message(
                p,
                "XHP classes can only contain one category declaration".into(),
            ),
            vec![],
            Explanation::Empty,
            vec![],
            vec![],
        )
    }

    pub fn partially_abstract_typeconst_definition(p: Pos, kind: &str) -> Diagnostic {
        UserDiagnostic::new(
            Severity::Err,
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
            Explanation::Empty,
            vec![],
            vec![],
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
            Self::Extended => "extended".fmt(f),
        }
    }
}

impl std::error::Error for ParseFormatError {}

impl std::fmt::Display for ParseFormatError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        "Unrecognized error format".fmt(f)
    }
}

#[allow(clippy::derivable_impls)]
impl Default for Format {
    fn default() -> Self {
        Format::Plain
    }
}

impl<'a> FromOcamlRepIn<'a> for Axiom {
    fn from_ocamlrep_in(
        _value: ocamlrep::Value<'_>,
        _arena: &'a bumpalo::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
        panic!("Axioms are internal to inference and should not appear in decls")
    }
}

impl<'a> DeserializeInArena<'a> for &Axiom {
    fn deserialize_in_arena<D>(
        _arena: &'a bumpalo::Bump,
        _deserializer: D,
    ) -> Result<Self, D::Error>
    where
        D: Deserializer<'a>,
    {
        panic!("Axioms are internal to inference and should not appear in decls")
    }
}
