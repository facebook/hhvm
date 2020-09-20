// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{
    errors::ErrorCode,
    i_map::IMap,
    pos::Pos,
    scoured_comments::{Fixmes, ScouredComments},
};

impl ScouredComments {
    pub fn new() -> Self {
        ScouredComments {
            comments: vec![],
            fixmes: IMap::new(),
            misuses: IMap::new(),
            error_pos: vec![],
        }
    }

    pub fn get_fixme(&self, pos: &Pos, code: ErrorCode) -> Option<&Pos> {
        let line = pos.info_pos().0 as isize;
        self.fixmes.get(&line).and_then(|m| m.get(&code))
    }

    pub fn add_to_fixmes(&mut self, line: isize, code: isize, pos: Pos) {
        Self::add(&mut self.fixmes, line, code, pos)
    }

    pub fn add_to_misuses(&mut self, line: isize, code: isize, pos: Pos) {
        Self::add(&mut self.misuses, line, code, pos)
    }

    pub fn add_format_error(&mut self, pos: Pos) {
        self.error_pos.push(pos);
    }

    fn add(m: &mut Fixmes, line: isize, code: isize, pos: Pos) {
        match m.get_mut(&line) {
            None => {
                let mut code_to_pos = IMap::new();
                code_to_pos.insert(code, pos);
                m.insert(line, code_to_pos);
            }
            Some(code_to_pos) => {
                code_to_pos.insert(code, pos);
            }
        }
    }
}
