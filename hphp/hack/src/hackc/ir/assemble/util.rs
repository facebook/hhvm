// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Result;
use anyhow::bail;

pub(crate) fn unescape(string: &str) -> Result<Vec<u8>> {
    let mut output = Vec::new();
    #[derive(Copy, Clone)]
    enum State {
        Unescaped,
        Escaped,
        Hex0,
        Hex1(u8),
    }
    let mut state = State::Unescaped;
    for c in string.chars() {
        match (state, c) {
            (State::Unescaped, '\\') => {
                state = State::Escaped;
            }
            (State::Unescaped, c) => {
                output.push(c as u8);
            }
            (State::Escaped, 'x') => {
                state = State::Hex0;
            }
            (State::Escaped, '\\') => {
                state = State::Unescaped;
                output.push(b'\\');
            }
            (State::Escaped, 'n') => {
                state = State::Unescaped;
                output.push(b'\n');
            }
            (State::Escaped, 't') => {
                state = State::Unescaped;
                output.push(b'\t');
            }
            (State::Escaped, 'r') => {
                state = State::Unescaped;
                output.push(b'\r');
            }
            (State::Escaped, '\"') => {
                state = State::Unescaped;
                output.push(b'\"');
            }
            (State::Escaped, _) => {
                bail!("Bad escape value");
            }
            (State::Hex0, x) if x.is_ascii_hexdigit() => {
                state = State::Hex1((x.to_digit(16).unwrap() as u8) << 4);
            }
            (State::Hex0, _) => {
                bail!("Bad hex value");
            }
            (State::Hex1(ub), x) if x.is_ascii_hexdigit() => {
                state = State::Unescaped;
                output.push(ub + (x.to_digit(16).unwrap() as u8));
            }
            (State::Hex1(_), _) => {
                bail!("Bad hex value");
            }
        }
    }
    Ok(output)
}
