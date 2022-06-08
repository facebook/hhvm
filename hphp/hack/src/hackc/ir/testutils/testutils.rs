// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

//! This module contains shared functions for use by tests.

use core::{instr, literal::Literal, BlockId, Func, FuncBuilder, Instr, LocId};
use hash::HashMap;

/// Given a simple string-based CFG description, create a Func that matches it.
///
/// Each array entry looks like:
///
///    ("blockname", ["successor1", "successor2"])
pub fn build_test_func<'a>(testcase: &[(&str, Vec<&str>)]) -> Func<'a> {
    // Create a function whose CFG matches testcase.
    let loc = LocId::NONE;

    let mut fb = FuncBuilder::new();

    let mut name_to_bid = HashMap::with_capacity_and_hasher(testcase.len(), Default::default());
    for (i, (name, _edges)) in testcase.iter().enumerate() {
        name_to_bid.insert(
            *name,
            if i == 0 {
                Func::ENTRY_BID
            } else {
                fb.alloc_bid()
            },
        );
    }

    let zero_iid = fb.emit_literal(Literal::null(loc));

    for (name, edges) in testcase {
        fb.start_block(name_to_bid[name]);
        fb.cur_block_mut().pname_hint = Some(name.to_string());

        let e: Vec<BlockId> = edges
            .iter()
            .map(|block_name| match name_to_bid.get(block_name) {
                Some(&x) => x,
                None => panic!("No such block {}", block_name),
            })
            .collect();

        let terminator = match e.len() {
            0 => Instr::ret(zero_iid, loc),
            1 => Instr::jmp(e[0], loc),
            2 => Instr::jmp_op(zero_iid, instr::Predicate::NonZero, e[0], e[1], loc),
            _ => panic!("unhandled edge count"),
        };
        fb.emit(terminator);
    }

    fb.into_func()
}
