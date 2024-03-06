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
use ir_core::ValueId;

/// Given a simple CFG description, create a Func that matches it.
pub fn build_test_func(testcase: &[Block]) -> (Func, Arc<StringInterner>) {
    let strings = Arc::new(StringInterner::default());
    let func = build_test_func_with_strings(testcase, Arc::clone(&strings));
    (func, strings)
}

pub fn build_test_func_with_strings(testcase: &[Block], strings: Arc<StringInterner>) -> Func {
    // Create a function whose CFG matches testcase.

    FuncBuilder::build_func(strings, |fb| {
        let mut name_to_bid = HashMap::with_capacity_and_hasher(testcase.len(), Default::default());
        let mut name_to_vid = HashMap::default();
        for (i, block) in testcase.iter().enumerate() {
            let bid = if i == 0 {
                Func::ENTRY_BID
            } else {
                fb.alloc_bid()
            };
            name_to_bid.insert(block.name.clone(), bid);
            fb.start_block(bid);
            for pn in &block.params {
                let pid = fb.alloc_param();
                name_to_vid.insert(pn.clone(), pid);
            }
        }

        for block in testcase {
            fb.start_block(name_to_bid[&block.name]);
            block.emit(fb, &name_to_bid, &mut name_to_vid);
        }
    })
}

#[derive(Debug, Clone)]
pub struct Block {
    /// The name of the block for test lookup purposes.
    pub name: String,
    /// The pname of the block.
    pub pname: Option<String>,
    /// For each "call_target" we create a simple call instruction calling a
    /// function with that target name.
    pub call_targets: Vec<(String, Option<String>)>,
    /// What kind of terminator to use on this block.
    pub terminator: Terminator,
    /// The parameters this block expects.
    pub params: Vec<String>,
}

#[derive(Debug, Clone)]
pub enum Terminator {
    Ret,
    RetValue(String),
    Jmp(String),
    JmpArg(String, String),
    JmpOp(String, String),
    CallAsync {
        target: String,
        lazy: String,
        eager: String,
    },
}

impl Block {
    pub fn successors(&self) -> impl Iterator<Item = &String> {
        match &self.terminator {
            Terminator::Ret | Terminator::RetValue(_) => vec![].into_iter(),
            Terminator::Jmp(target) | Terminator::JmpArg(target, _) => vec![target].into_iter(),
            Terminator::JmpOp(a, b) => vec![a, b].into_iter(),
            Terminator::CallAsync { lazy, eager, .. } => vec![lazy, eager].into_iter(),
        }
    }

    fn emit(
        &self,
        fb: &mut FuncBuilder,
        name_to_bid: &HashMap<String, BlockId>,
        name_to_vid: &mut HashMap<String, ValueId>,
    ) {
        if let Some(pname) = self.pname.as_ref() {
            fb.cur_block_mut().pname_hint = Some(pname.to_string());
        }

        let bid_for = |name: &str| -> BlockId { *name_to_bid.get(name).unwrap() };

        let loc = LocId::NONE;

        for (target, name) in &self.call_targets {
            let target = FunctionId::from_str(target, &fb.strings);
            let iid = fb.emit(Instr::simple_call(target, &[], loc));
            if let Some(name) = name {
                name_to_vid.insert(name.clone(), iid);
            }
        }

        let null_iid = fb.emit_constant(Constant::Null);

        let terminator = match &self.terminator {
            Terminator::Ret => Instr::ret(null_iid, loc),
            Terminator::RetValue(arg) => {
                let arg = *name_to_vid.get(arg).unwrap();
                Instr::ret(arg, loc)
            }
            Terminator::Jmp(a) => {
                let a = bid_for(a);
                Instr::jmp(a, loc)
            }
            Terminator::JmpArg(a, arg) => {
                let a = bid_for(a);
                let arg = *name_to_vid.get(arg).unwrap();
                Instr::jmp_args(a, &[arg], loc)
            }
            Terminator::JmpOp(a, b) => {
                let a = bid_for(a);
                let b = bid_for(b);
                Instr::jmp_op(null_iid, instr::Predicate::NonZero, a, b, loc)
            }
            Terminator::CallAsync {
                target,
                lazy,
                eager,
            } => {
                let lazy = bid_for(lazy);
                let eager = bid_for(eager);
                let func = FunctionId::from_str(target, &fb.strings);
                let call = ir_core::Call {
                    operands: Box::new([]),
                    context: ir_core::UnitBytesId::EMPTY,
                    detail: instr::CallDetail::FCallFuncD { func },
                    flags: ir_core::FCallArgsFlags::default(),
                    num_rets: 1,
                    inouts: None,
                    readonly: None,
                    loc,
                };
                Instr::Terminator(instr::Terminator::CallAsync(Box::new(call), [lazy, eager]))
            }
        };
        fb.emit(terminator);
    }

    /// A simple block which jumps to a single successor.
    pub fn jmp(name: &str, successor: &str) -> Block {
        Self::ret(name).with_terminator(Terminator::Jmp(successor.to_string()))
    }

    /// A simple block which jumps to two successors.
    pub fn jmp_op(name: &str, successors: [&str; 2]) -> Block {
        Self::ret(name).with_terminator(Terminator::JmpOp(
            successors[0].to_string(),
            successors[1].to_string(),
        ))
    }

    pub fn jmp_arg(name: &str, successor: &str, value: &str) -> Block {
        Self::ret(name)
            .with_terminator(Terminator::JmpArg(successor.to_string(), value.to_string()))
    }

    pub fn ret(name: &str) -> Block {
        Block {
            call_targets: Vec::new(),
            name: name.to_owned(),
            params: Vec::new(),
            pname: Some(name.to_owned()),
            terminator: Terminator::Ret,
        }
    }

    pub fn ret_value(name: &str, param: &str) -> Block {
        Self::ret(name).with_terminator(Terminator::RetValue(param.to_owned()))
    }

    pub fn call_async(name: &str, target: &str, [lazy, eager]: [&str; 2]) -> Block {
        Self::ret(name).with_terminator(Terminator::CallAsync {
            target: target.to_owned(),
            lazy: lazy.to_owned(),
            eager: eager.to_owned(),
        })
    }

    pub fn unnamed(mut self) -> Self {
        self.pname = None;
        self
    }

    pub fn with_target(mut self) -> Self {
        self.call_targets = vec![(format!("{}_target", self.name), None)];
        self
    }

    pub fn with_named_target(mut self, name: &str) -> Self {
        let name = name.to_owned();
        self.call_targets = vec![(format!("{}_target", self.name), Some(name))];
        self
    }

    pub fn with_terminator(mut self, terminator: Terminator) -> Self {
        self.terminator = terminator;
        self
    }

    pub fn with_param(mut self, name: &str) -> Self {
        self.params = vec![name.to_owned()];
        self
    }
}

/// Structurally compare two Funcs.
pub fn assert_func_struct_eq(func_a: &Func, func_b: &Func, strings: &StringInterner) {
    if let Err(e) = cmp_func_struct_eq(func_a, func_b) {
        panic!(
            "Function mismatch: {}\n{}\n{}",
            e,
            print::DisplayFunc::new(func_a, true, strings),
            print::DisplayFunc::new(func_b, true, strings)
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

fn cmp_func_struct_eq(func_a: &Func, func_b: &Func) -> Result<(), String> {
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

fn cmp_block_struct_eq(
    func_a: &Func,
    bid_a: BlockId,
    func_b: &Func,
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
