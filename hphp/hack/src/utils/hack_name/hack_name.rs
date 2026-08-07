// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! The character classes that make up a Hack name.

/// Whether `c` may start a Hack name: `_`, an ASCII letter, or any character at
/// or above `0x7f` (Hack names may contain non-ASCII characters).
pub fn is_name_nondigit(c: char) -> bool {
    c == '_' || c.is_ascii_alphabetic() || c >= '\u{7f}'
}

/// Whether `c` may appear after the first character of a Hack name. Same as
/// [`is_name_nondigit`], plus ASCII digits.
pub fn is_name_letter(c: char) -> bool {
    is_name_nondigit(c) || c.is_ascii_digit()
}

/// Whether `s` is a valid Hack identifier: non-empty, beginning with a name
/// nondigit and continuing with name letters.
///
/// Note what this excludes, since `:` and `\` are not name characters: XHP names
/// (`:foo`), qualified names (`A\B`), and anything containing `.` or `-`.
pub fn is_valid_identifier(s: &str) -> bool {
    let mut chars = s.chars();
    match chars.next() {
        Some(first) => is_name_nondigit(first) && chars.all(is_name_letter),
        None => false,
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn accepts_ordinary_identifiers() {
        for s in ["foo", "Foo", "_foo", "f", "_", "foo1", "f_1_B", "ünïcode"] {
            assert!(is_valid_identifier(s), "should accept {s:?}");
        }
    }

    #[test]
    fn rejects_non_identifiers() {
        // Empty, leading digit, and every character that is not a name char:
        // `.` (dotted directory names), `:` (XHP), `\` (qualified), `-`, `/`.
        for s in [
            "", "1bad", "9", ":xhp", "a.b", "proto.v1", r"A\B", "a-b", "a/b", "a b",
        ] {
            assert!(!is_valid_identifier(s), "should reject {s:?}");
        }
    }
}
