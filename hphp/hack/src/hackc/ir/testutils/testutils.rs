// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

//! This module contains shared functions for use by tests.

use std::sync::Arc;

use hash::HashMap;
use hash::HashSet;
use ir_core::constant::Constant;
use ir_core::instr;
use ir_core::BlockId;
use ir_core::Func;
use ir_core::FuncBuilder;
use ir_core::FunctionId;
use ir_core::HasEdges;
use ir_core::Instr;
use ir_core::LocId;
use ir_core::StringInterner;

/// Given a simple string-based CFG description, create a Func that matches it.
///
/// Each array entry looks like:
///
///    ("blockname", ["call_target1", "call_target2"], ["successor1", "successor2"])
///
/// where <instr count> is the number of non-terminal instructions to insert.
///
pub fn build_test_func<'a>(
    testcase: &[(&str, Vec<&str>, Vec<&str>)],
) -> (Func<'a>, StringInterner) {
    let mut strings = StringInterner::default();
    let func = build_test_func_with_strings(testcase, &mut strings);
    (func, strings)
}

pub fn build_test_func_with_strings<'a>(
    testcase: &[(&str, Vec<&str>, Vec<&str>)],
    strings: &mut StringInterner,
) -> Func<'a> {
    // Create a function whose CFG matches testcase.
    let loc = LocId::NONE;

    let tmp_strings = Arc::new(StringInterner::default());
    let func = FuncBuilder::build_func(Arc::clone(&tmp_strings), |fb| {
        let mut name_to_bid = HashMap::with_capacity_and_hasher(testcase.len(), Default::default());
        for (i, (name, _, _)) in testcase.iter().enumerate() {
            name_to_bid.insert(
                *name,
                if i == 0 {
                    Func::ENTRY_BID
                } else {
                    fb.alloc_bid()
                },
            );
        }

        let null_iid = fb.emit_constant(Constant::Null);

        for (name, call_targets, edges) in testcase {
            fb.start_block(name_to_bid[name]);
            fb.cur_block_mut().pname_hint = Some(name.to_string());

            let e: Vec<BlockId> = edges
                .iter()
                .map(|block_name| match name_to_bid.get(block_name) {
                    Some(&x) => x,
                    None => panic!("No such block {}", block_name),
                })
                .collect();

            for target in call_targets {
                let target = FunctionId::from_str(target, strings);
                fb.emit(Instr::simple_call(target, &[], loc));
            }

            let terminator = match e.len() {
                0 => Instr::ret(null_iid, loc),
                1 => Instr::jmp(e[0], loc),
                2 => Instr::jmp_op(null_iid, instr::Predicate::NonZero, e[0], e[1], loc),
                _ => panic!("unhandled edge count"),
            };
            fb.emit(terminator);
        }
    });
    assert!(tmp_strings.is_empty());
    func
}

/// Structurally compare two Funcs.
pub fn assert_func_struct_eq<'a>(func_a: &Func<'a>, func_b: &Func<'a>, strings: &StringInterner) {
    if let Err(e) = cmp_func_struct_eq(func_a, func_b) {
        panic!(
            "Function mismatch: {}\n{}\n{}",
            e,
            print::DisplayFunc(func_a, true, strings),
            print::DisplayFunc(func_b, true, strings)
        );
    }
}

macro_rules! cmp_eq {
    ($a:expr, $b:expr, $($rest:tt)+) => {
        if $a == $b {
            Ok(())
        } else {
            Err(format!($($rest)+))
        }
    };
}

fn cmp_func_struct_eq<'a>(func_a: &Func<'a>, func_b: &Func<'a>) -> Result<(), String> {
    let mut block_eq: HashSet<(BlockId, BlockId)> = HashSet::default();

    let mut pending_cmp: Vec<(BlockId, BlockId)> = vec![(Func::ENTRY_BID, Func::ENTRY_BID)];
    cmp_eq!(
        func_a.params.len(),
        func_b.params.len(),
        "param length mismatch",
    )?;
    for (param_a, param_b) in func_a.params.iter().zip(func_b.params.iter()) {
        match (
            param_a.default_value.as_ref(),
            param_b.default_value.as_ref(),
        ) {
            (Some(dv_a), Some(dv_b)) => {
                pending_cmp.push((dv_a.init, dv_b.init));
            }
            (None, None) => {}
            _ => panic!("Mismatch in default value index"),
        }
    }

    while let Some((bid_a, bid_b)) = pending_cmp.pop() {
        block_eq.insert((bid_a, bid_b));
        cmp_block_struct_eq(func_a, bid_a, func_b, bid_b)?;

        let term_a = func_a.terminator(bid_a);
        let term_b = func_b.terminator(bid_b);
        cmp_eq!(
            term_a.edges().len(),
            term_b.edges().len(),
            "mismatched terminator edge count",
        )?;
        for (edge_a, edge_b) in term_a.edges().iter().zip(term_b.edges().iter()) {
            pending_cmp.push((*edge_a, *edge_b));
        }
    }

    Ok(())
}

fn cmp_block_struct_eq<'a>(
    func_a: &Func<'a>,
    bid_a: BlockId,
    func_b: &Func<'a>,
    bid_b: BlockId,
) -> Result<(), String> {
    let block_a = func_a.block(bid_a);
    let block_b = func_b.block(bid_b);

    cmp_eq!(
        block_a.params.len(),
        block_b.params.len(),
        "block param len mismatch",
    )?;
    cmp_eq!(
        block_a.iids.len(),
        block_b.iids.len(),
        "block iids len mismatch",
    )?;
    cmp_eq!(
        &block_a.pname_hint,
        &block_b.pname_hint,
        "pname mismatch in ({}, {})",
        bid_a,
        bid_b
    )?;
    // TODO: check tcid

    for (iid_a, iid_b) in block_a
        .iids
        .iter()
        .copied()
        .zip(block_b.iids.iter().copied())
    {
        cmp_eq!(
            std::mem::discriminant(func_a.instr(iid_a)),
            std::mem::discriminant(func_b.instr(iid_b)),
            "instr mismatch",
        )?;
    }

    Ok(())
}
