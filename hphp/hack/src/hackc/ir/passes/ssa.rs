// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
use std::collections::hash_map::Entry;

use analysis::compute_predecessor_blocks;
use analysis::PredecessorCatchMode;
use analysis::PredecessorFlags;
use analysis::Predecessors;
use ir_core::func_builder::TransformInstr;
use ir_core::func_builder::TransformState;
use ir_core::instr::Special;
use ir_core::instr::Terminator;
use ir_core::instr::Tmp;
use ir_core::newtype::VarIdMap;
use ir_core::BlockId;
use ir_core::Func;
use ir_core::FuncBuilder;
use ir_core::Instr;
use ir_core::InstrId;
use ir_core::StringInterner;
use ir_core::ValueId;
use ir_core::VarId;
use itertools::Itertools;
use newtype::IdVec;

/*

This pass converts a Func to SSA form by eliminating GetVar and SetVar
instrs.

It uses an algorithm much like that described in "Simple and Efficient
Construction of Static Single Assignment Form",
https://pp.info.uni-karlsruhe.de/uploads/publikationen/braun13cc.pdf

The high-level idea is pretty simple: replace each GetVar with the
value most recently assigned by a SetVar, if known. If not, as in a
variable conditionally assigned inside an "if", then create a
Param in the GetVar's block and replace all uses of the GetVar with it.
Every predecessor block is then required to pass in its "ending" value for
that variable, which may recursively force that predecessor to get the value
from its predecessors and so on.

All block iteration is done using reverse postorder DFS, which is how
blocks are stored in every Func.

Detailed steps:

1) For each block, compute "local" dataflow facts using a simple forward scan:

   a) Identify variables assigned in that block, and the final value
      assigned to each one ("exit_value").
   b) Identify variables read before being written in that block, recording
      them in the keys of "entry_value"). These are "live on entry".

2) Propagate dataflow information globally through the CFG:

   If a variable is live on entry to a block, then it is also live on entry
   to every predecessor block that does not assign to that variable, and so
   on recursively. Update each block's entry_value keys appropriately.

3) Fill in each block's "entry_value" table with the incoming values
   of all live variables.

   The entry_value for each live variable depends on its exit_value in
   all predecessor blocks:

   a) If all predecessor blocks have the same known exit_value for that
      variable, then that is also the entry_value.
   b) If predecessor blocks disagree about the exit_value, or one of them
      hasn't computed that variable's exit_value yet (due to a loop),
      then create a Param and use that as the entry_value.
   c) Now that entry_value is complete, update exit_value. For each
      variable in entry_value:
      i) If it was assigned by that block, meaning it's already got an entry
         in exit_value, do nothing. The entry_value is overridden by the local
         assignment.
      ii) Else update that variable's exit_value to be its entry_value
          (the variable is just "passing through" the block without being
          touched).

   Note that this step can create unnecessary Params (those whose
   possible values consist only of a single non-Param value and
   possibly themselves). For example, each variable defined outside loops
   and accessed within them will pessimistically get a Param at
   the loop head, even if it is never assigned to inside the loop.

   We could of course clean up unnecessary Params here, but for
   code simplicity we will instead delegate that work to a separate
   Param optimization pass which we need anyway (e.g. other
   optimizations like DCE can make Params unnecessary long after
   SSA formation is complete).

4) Rewrite the function.

   The tables computed above give us enough information to rewrite
   the Func to eliminate GetVar and SetVar.

   For each block:

   a) Append any new Params to the block's params vec.
   b) For each instr in the block:
     - if SetVar, update entry_value with the assigned value. entry_value is now
       misleadingly named, but it's safe to clobber now. Discard the SetVar.
     - if GetVar, replace it with the current value in entry_value
     - If target has new Params, it must have multiple predecessors.
       We know the block branching to it must end with Br or ArgBr because
       there are no critical edges in the CFG. Convert it to a ArgBr and
       append the expanded list of arguments based on exit_value (which should
       be equivalent to entry_value).

*/

/// Extra per-block data needed by MakeSSA.
#[derive(Default)]
struct BlockInfo {
    // The value of each live local block entry.
    entry_value: VarIdMap<ValueId>,

    // The value of each live local block exit.
    exit_value: VarIdMap<ValueId>,

    // Extra block Params this block needs.
    extra_params: Vec<(
        VarId,   // Declare
        InstrId, // Param
    )>,
}

/// Tracks values assigned to this variable.
/// If only one value is ever assigned, we don't need to make Params for it.
#[derive(Copy, Clone)]
enum SetValue {
    One(ValueId),
    Multiple,
}

#[derive(Copy, Clone)]
struct Decl {
    /// If we need extra per-variable info like types this is the place to store
    /// it.
    set_value: SetValue,
}

struct MakeSSA<'a> {
    predecessors: Predecessors,

    block_info: IdVec<BlockId, BlockInfo>,

    // Values from Local::Declare we have seen.
    decls: VarIdMap<Decl>,

    strings: &'a StringInterner,
}

impl<'a> MakeSSA<'a> {
    fn new(func: &Func, strings: &'a StringInterner) -> MakeSSA<'a> {
        let predecessors = compute_predecessor_blocks(
            func,
            PredecessorFlags {
                catch: PredecessorCatchMode::Throw,
                ..Default::default()
            },
        );
        let block_info: IdVec<BlockId, BlockInfo> =
            (0..func.blocks.len()).map(|_| Default::default()).collect();
        MakeSSA {
            predecessors,
            block_info,
            decls: Default::default(),
            strings,
        }
    }

    fn run(&mut self, func: &mut Func) {
        self.analyze_dataflow(func);
        self.create_params(func);
        self.rewrite_instrs(func);
    }

    // Steps (1) and (2).
    fn analyze_dataflow(&mut self, func: &Func) {
        // Stack of dataflow information to propagate, in lieu of recursion.
        // BlockId needs the value of Declare's InstrId on entry because
        // Get wants it (directly or indirectly).
        let mut dataflow_stack: Vec<(BlockId, InstrId, VarId)> = Vec::new();

        // First analyze each block locally to see what it reads and writes.
        for bid in func.block_ids() {
            let info = &mut self.block_info[bid];

            for &iid in &func.blocks[bid].iids {
                match func.instrs[iid] {
                    Instr::Special(Special::Tmp(Tmp::GetVar(var))) => {
                        note_live(&mut dataflow_stack, info, bid, iid, var)
                    }
                    Instr::Special(Special::Tmp(Tmp::SetVar(var, value))) => {
                        info.exit_value.insert(var, value);

                        // See which values are ever assigned to this variable.
                        match self.decls.entry(var) {
                            Entry::Vacant(e) => {
                                // First assignment we've ever seen to this variable.
                                e.insert(Decl {
                                    set_value: SetValue::One(value),
                                });
                            }
                            Entry::Occupied(mut e) => {
                                let decl = e.get_mut();
                                match decl.set_value {
                                    SetValue::One(old_iid) if old_iid != value => {
                                        // Two distinct InstrId are assigned to this variable,
                                        // so we may need Params later.
                                        decl.set_value = SetValue::Multiple;
                                    }
                                    SetValue::Multiple | SetValue::One(_) => {}
                                }
                            }
                        }
                    }
                    _ => {}
                }
            }
        }

        // Propagate local variable liveness around the CFG.
        while let Some((bid, get_local_iid, var)) = dataflow_stack.pop() {
            if bid == Func::ENTRY_BID {
                // Uh oh, the entry block needs the value of a local variable on
                // entry. That must mean it's used uninitialized by
                // get_local_iid.
                panic!(
                    "Local variable may be used uninitialized by '{}' in\n{}",
                    print::FmtInstr(func, self.strings, get_local_iid),
                    print::DisplayFunc::new(func, true, self.strings),
                );
            };

            for &pred_bid in &self.predecessors[&bid] {
                note_live(
                    &mut dataflow_stack,
                    &mut self.block_info[pred_bid],
                    pred_bid,
                    get_local_iid,
                    var,
                );
            }
        }
    }

    // Step (3).
    fn create_params(&mut self, func: &mut Func) {
        // Guarantee each var live at block entry has a value we can use.
        //
        // In loops this will sometimes create degenerate Params, which
        // are always passed the same value or themselves. We clean those up
        // elsewhere rather than trying to get too clever here.

        for bid in func.block_ids() {
            let pred_bids = &self.predecessors[&bid];

            // Sort to make param creation order deterministic. Also, we need
            // a snapshot of this map anyway since we are going to mutate it.
            let mut entry_values = self.block_info[bid]
                .entry_value
                .keys()
                .copied()
                .collect_vec();
            entry_values.sort_unstable();

            for var in entry_values {
                // Optimize the trivial case where we know every predecessor
                // passes the same value. This won't help for loop headers
                // because we haven't processed the predecessors yet, but it
                // catches easier cases.
                let mut value = ValueId::none();

                let decl = &self.decls[&var];

                match decl.set_value {
                    SetValue::One(v) => {
                        // Only one value is ever assigned to this
                        // local. There's no need to examine dataflow at all, we
                        // don't need a Param.
                        value = v;
                    }
                    SetValue::Multiple => {
                        for &p in pred_bids {
                            match self.block_info[p].exit_value.get(&var) {
                                Some(pred_value) => {
                                    if value.is_none() {
                                        // This is the first value we've seen;
                                        // compare to the rest.
                                        value = *pred_value
                                    } else if value != *pred_value {
                                        // Getting two different values passed
                                        // in, make a Param.
                                        value = ValueId::none();
                                        break;
                                    }
                                }
                                None => {
                                    if p != bid {
                                        // Predecessor doesn't know var's value
                                        // yet, so give up.  But we can ignore
                                        // the case where a block branches to
                                        // itself without reassigning the
                                        // variable, because that has no effect
                                        // on whether the input value is fixed.
                                        value = ValueId::none();
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                let info = &mut self.block_info[bid];

                if value.is_none() {
                    // We aren't sure what the value is. So make a Param.
                    let iid = func.alloc_instr(Instr::param());
                    value = ValueId::from_instr(iid);
                    info.extra_params.push((var, iid));
                };

                info.entry_value.insert(var, value);

                // If we already have an exit_value, meaning the variable is
                // assigned in this block, don't override it. But if not, the
                // exit_value is the entry_value.
                info.exit_value.entry(var).or_insert(value);
            }
        }
    }

    // Step (4).
    fn rewrite_instrs(&mut self, func: &mut Func) {
        FuncBuilder::borrow_func_no_strings(func, |rw| {
            for bid in rw.func.block_ids() {
                let info = &mut self.block_info[bid];

                // Record any additional Params.
                if !info.extra_params.is_empty() {
                    let bparams = &mut rw.func.blocks[bid].params;
                    for (_, param) in &info.extra_params {
                        bparams.push(*param);
                    }
                    bparams.shrink_to_fit();
                }

                // Walk the block and process Local instrs.
                rw.rewrite_block(bid, self);
            }
        });
    }
}

impl<'a> TransformInstr for MakeSSA<'a> {
    fn apply(
        &mut self,
        _iid: InstrId,
        i: Instr,
        rw: &mut FuncBuilder,
        _state: &mut TransformState,
    ) -> Instr {
        let bid = rw.cur_bid();

        match i {
            Instr::Terminator(Terminator::Jmp(..) | Terminator::JmpArgs(..)) => {
                // Pass args for new Params, if any, to the target block.
                let (target, args, loc): (_, &[ValueId], _) = match i {
                    Instr::Terminator(Terminator::JmpArgs(target, ref args, loc)) => {
                        (target, args, loc)
                    }
                    Instr::Terminator(
                        Terminator::Enter(target, loc) | Terminator::Jmp(target, loc),
                    ) => (target, &[], loc),
                    _ => unreachable!(),
                };

                let target_info = &self.block_info[target];
                if target_info.extra_params.is_empty() {
                    i
                } else {
                    let mut args = args.to_vec();

                    for (var, _) in &target_info.extra_params {
                        let value = rw.find_replacement(self.block_info[bid].entry_value[var]);
                        args.push(value);
                    }

                    assert!(!args.is_empty());
                    let t = Terminator::JmpArgs(target, args.into(), loc);
                    Instr::Terminator(t)
                }
            }
            Instr::Special(Special::Tmp(Tmp::SetVar(var, value))) => {
                self.block_info[bid].entry_value.insert(var, value);
                Instr::tombstone()
            }
            Instr::Special(Special::Tmp(Tmp::GetVar(var))) => {
                let value = self.block_info[bid].entry_value[&var];
                Instr::copy(value)
            }
            i => i,
        }
    }
}

/// Record that the variable in var is used in block 'bid'.  We track
/// which instr uses it (get_local_iid), rather than just the fact that
/// the var is used, only for error reporting purposes.
fn note_live(
    dataflow_stack: &mut Vec<(BlockId, InstrId, VarId)>,
    info: &mut BlockInfo,
    bid: BlockId,
    get_local_iid: InstrId,
    var: VarId,
) {
    // NOTE: We temporarily stash get_local_iid in this table, to report
    // errors, but later we will replace it with the actual value.
    if !info.exit_value.contains_key(&var)
        && info
            .entry_value
            .insert(var, ValueId::from_instr(get_local_iid))
            .is_none()
    {
        // Recursively propagate liveness to all predecessors.
        dataflow_stack.push((bid, get_local_iid, var))
    }
}

pub(crate) fn is_ssa(func: &Func) -> bool {
    !func
        .body_instrs()
        .any(|i| matches!(i, Instr::Special(Special::Tmp(..))))
}

pub fn run(func: &mut Func, strings: &StringInterner) -> bool {
    if is_ssa(func) {
        false
    } else {
        let mut pass = MakeSSA::new(func, strings);
        pass.run(func);
        true
    }
}

#[cfg(test)]
mod test {
    use std::sync::Arc;

    use ir_core::instr::HasOperands;
    use ir_core::instr::Predicate;
    use ir_core::instr::Terminator;
    use ir_core::Constant;
    use ir_core::FuncBuilder;
    use ir_core::FunctionName;
    use ir_core::LocId;
    use ir_core::StringInterner;

    use super::*;

    #[test]
    fn already_ssa() {
        let loc = LocId::NONE;
        let strings = Arc::new(StringInterner::default());
        let mut func = FuncBuilder::build_func(Arc::clone(&strings), |builder| {
            // %0 = call("my_fn", [42])
            // %1 = ret null
            let value = builder.emit_constant(Constant::Int(42));
            let null = builder.emit_constant(Constant::Null);
            let id = FunctionName::intern("my_fn");
            builder.emit(Instr::simple_call(id, &[value], loc));
            builder.emit(Instr::ret(null, loc));
        });
        verify::verify_func(&func, &Default::default(), &strings);
        assert!(is_ssa(&func));
        let res = run(&mut func, &strings);
        assert!(!res);
    }

    #[test]
    fn basic() {
        let loc = LocId::NONE;
        let strings = Arc::new(StringInterner::default());
        let mut func = FuncBuilder::build_func(Arc::clone(&strings), |builder| {
            // %0 = declare
            // %1 = set(%0, 42)
            // %2 = get(%0)
            // call("my_fn", [%2])
            // ret null
            let var = VarId::from_usize(0);
            let value = builder.emit_constant(Constant::Int(42));
            builder.emit(Instr::set_var(var, value));
            let null = builder.emit_constant(Constant::Null);
            let value = builder.emit(Instr::get_var(var));
            let id = FunctionName::intern("my_fn");
            builder.emit(Instr::simple_call(id, &[value], loc));
            builder.emit(Instr::ret(null, loc));
        });
        verify::verify_func(&func, &Default::default(), &strings);
        assert!(!is_ssa(&func));
        let res = run(&mut func, &strings);
        assert!(res);

        assert_eq!(func.blocks.len(), 1);
        let mut it = func.body_instrs();

        let instr = it.next();
        assert!(matches!(instr, Some(Instr::Call(..))));
        let ops = instr.unwrap().operands();
        assert_eq!(ops.len(), 1);
        assert!(matches!(
            ops[0].constant().map(|lit| func.constant(lit)),
            Some(Constant::Int(42))
        ));

        assert!(matches!(
            it.next(),
            Some(Instr::Terminator(Terminator::Ret(..)))
        ));
        assert!(it.next().is_none());
    }

    #[test]
    fn diamond() {
        let loc = LocId::NONE;
        let strings = Arc::new(StringInterner::default());
        let mut func = FuncBuilder::build_func(Arc::clone(&strings), |builder| {
            //   %0 = declare
            //   %1 = declare
            //   %2 = declare
            //        set(%0, 42)
            //        set(%1, 314)
            //        if true jmp b1 else b2
            // b1:
            //        set(%1, 123)
            //        set(%2, 1)
            //        jmp b3
            // b2:
            //        set(%2, 2)
            //        jmp b3
            // b3:
            //   %3 = get(%0)
            //   %4 = get(%1)
            //   %4 = get(%2)
            //        call("my_fn", [%3, %4, %5])
            //        ret null
            let var0 = VarId::from_usize(0);
            let var1 = VarId::from_usize(1);
            let var2 = VarId::from_usize(2);

            let value = builder.emit_constant(Constant::Int(42));
            builder.emit(Instr::set_var(var0, value));

            let value = builder.emit_constant(Constant::Int(314));
            builder.emit(Instr::set_var(var1, value));

            let true_bid = builder.alloc_bid();
            let false_bid = builder.alloc_bid();
            let join_bid = builder.alloc_bid();
            let value = builder.emit_constant(Constant::Bool(true));
            builder.emit(Instr::jmp_op(
                value,
                Predicate::NonZero,
                true_bid,
                false_bid,
                loc,
            ));

            builder.start_block(true_bid);

            let value = builder.emit_constant(Constant::Int(123));
            builder.emit(Instr::set_var(var1, value));

            let value = builder.emit_constant(Constant::Int(1));
            builder.emit(Instr::set_var(var2, value));

            builder.emit(Instr::jmp(join_bid, loc));

            builder.start_block(false_bid);

            let value = builder.emit_constant(Constant::Int(2));
            builder.emit(Instr::set_var(var2, value));

            builder.emit(Instr::jmp(join_bid, loc));

            builder.start_block(join_bid);

            let null = builder.emit_constant(Constant::Null);
            let value0 = builder.emit(Instr::get_var(var0));
            let value1 = builder.emit(Instr::get_var(var1));
            let value2 = builder.emit(Instr::get_var(var2));
            let id = FunctionName::intern("my_fn");
            builder.emit(Instr::simple_call(id, &[value0, value1, value2], loc));
            builder.emit(Instr::ret(null, loc));
        });
        verify::verify_func(&func, &Default::default(), &strings);
        assert!(!is_ssa(&func));
        let res = run(&mut func, &strings);
        assert!(res);
        crate::clean::run(&mut func);

        // b0:
        //   %0 = jmp if nonzero #2 to b1 else b2
        // b1:
        //   %1 = jmp to b3 with (#3, #4)
        // b2:
        //   %2 = jmp to b3 with (#1, #5)
        // b3(%3, %4):
        //   %5 = call direct "my_fn"(#0, %3, %4) num_rets(0) NONE
        //   %6 = ret #6

        let mut it = func.body_instrs();

        assert!(match it.next() {
            Some(Instr::Terminator(Terminator::JmpOp {
                targets: [true_bid, false_bid],
                ..
            })) if true_bid.as_usize() == 1 && false_bid.as_usize() == 2 => true,
            _ => false,
        });
        assert!(match it.next() {
            Some(Instr::Terminator(Terminator::JmpArgs(bid, values, _)))
                if bid.as_usize() == 3 && values.len() == 2 =>
                true,
            _ => false,
        });
        assert!(match it.next() {
            Some(Instr::Terminator(Terminator::JmpArgs(bid, values, _)))
                if bid.as_usize() == 3 && values.len() == 2 =>
                true,
            _ => false,
        });
        assert!(match it.next() {
            Some(i @ Instr::Call(..)) if i.operands().len() == 3 => true,
            _ => false,
        });
        assert!(matches!(
            it.next(),
            Some(Instr::Terminator(Terminator::Ret(..)))
        ));
        assert!(it.next().is_none());
    }
}
