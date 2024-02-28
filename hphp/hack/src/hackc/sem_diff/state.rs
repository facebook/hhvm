#![allow(clippy::todo)]

use std::collections::hash_map::Entry;
use std::rc::Rc;

use anyhow::anyhow;
use anyhow::bail;
use anyhow::Result;
use ffi::Str;
use hash::HashMap;
use hhbc::AdataId;
use hhbc::ClassName;
use hhbc::Dummy;
use hhbc::FCallArgs;
use hhbc::IncDecOp;
use hhbc::Instruct;
use hhbc::IterArgs;
use hhbc::IterId;
use hhbc::Label;
use hhbc::Local;
use hhbc::LocalRange;
use hhbc::MemberKey;
use hhbc::NumParams;
use hhbc::Opcode;
use hhbc::Pseudo;
use hhbc::ReadonlyOp;
use hhbc::SetOpOp;
use hhbc::SrcLoc;
use hhbc::SwitchKind;
use hhbc::Targets;
use hhbc::TypedValue;
use hhbc_gen::InstrFlags;
use hhbc_gen::Outputs;
use itertools::Itertools;
use log::trace;
use newtype::BuildIdHasher;

use crate::body::Body;
use crate::instr_ptr::InstrPtr;
use crate::local_info::LocalInfo;
use crate::node;
use crate::node::Input;
use crate::node::Node;
use crate::node::NodeInstr;
use crate::sequence::Sequence;
use crate::value::Value;
use crate::value::ValueBuilder;

/// A State is an abstract interpreter over HHVM bytecode.
#[derive(Clone)]
pub(crate) struct State<'arena, 'a> {
    body: &'a Body<'arena>,
    debug_name: &'static str,
    pub(crate) ip: InstrPtr,
    pub(crate) iterators: IterIdMap<IterState>,
    pub(crate) locals: HashMap<Local, Value>,
    pub(crate) stack: Vec<Value>,
    adata: &'a HashMap<AdataId, &'arena TypedValue>,
}

impl<'arena, 'a> State<'arena, 'a> {
    pub(crate) fn new(
        body: &'a Body<'arena>,
        debug_name: &'static str,
        adata: &'a HashMap<AdataId, &'arena TypedValue>,
    ) -> Self {
        Self {
            body,
            debug_name,
            ip: InstrPtr::from_usize(0),
            iterators: Default::default(),
            locals: Default::default(),
            stack: Default::default(),
            adata,
        }
    }

    /// Return an Input for a Value where the Input represents a non-COW
    /// datatype.
    fn non_cow(&self, value: Value) -> Input<'arena> {
        match value {
            Value::Constant(c) => Input::Constant(c),
            Value::Defined(v) => Input::Read(v),
            Value::Undefined => Input::Undefined,
        }
    }

    /// Compute the ownership for a Value and return an Input representing that
    /// Value with its ownership.
    fn reffy(&self, value: Value) -> Input<'arena> {
        match value {
            Value::Constant(c) => Input::Constant(c),
            Value::Defined(v) => {
                let mut count = 0;
                for &sv in &self.stack {
                    count += (sv == value) as usize;
                }
                for &lv in self.locals.values() {
                    count += (lv == value) as usize;
                }

                match count {
                    0 => Input::Unowned(v),
                    1 => Input::Owned(v),
                    _ => Input::Shared(v),
                }
            }
            Value::Undefined => Input::Undefined,
        }
    }

    /// Run the abstract interpreter and collect a sequence of checkpoint
    /// instructions along with their abstract inputs. The sequence ends when
    /// the interpreter reaches a terminal instruction (like a RetC) or a
    /// try/catch change. Any conditional control flows (like JmpZ or throws)
    /// are also returned.
    pub(crate) fn collect(
        mut self,
        value_builder: &mut ValueBuilder<'arena>,
    ) -> Result<StateCollect<'arena, 'a>> {
        let debug_name = format!("{}{}", self.debug_name, self.ip);
        trace!("--- Collecting sequence {}", debug_name);
        self.debug_state();

        let mut builder = InstrSeqBuilder {
            seq: Default::default(),
            forks: Default::default(),
            value_builder,
        };
        let mut seen_ip: HashMap<InstrPtr, usize> = HashMap::default();
        let mut catch_ip = self.body.lookup_catch(self.ip);
        let mut loopback = None;

        loop {
            if self.is_done() {
                trace!("  - ends at {} with is_done()", self.ip);
                break;
            }
            match seen_ip.entry(self.ip) {
                Entry::Occupied(e) => {
                    // If we've already seen this IP then record where we looped
                    // back to.
                    let target = *e.get();
                    loopback = Some(target);
                    trace!("  - ends at {} with loopback to {}", self.ip, target);
                    break;
                }
                Entry::Vacant(e) => {
                    // Record where this IP ends up in the sequence.
                    e.insert(builder.seq.len());
                }
            }

            // A Sequence must not cross try/catch blocks BUT if we haven't
            // actually recorded any instrs yet then we can restart with the new
            // catch block.
            let new_catch_ip = self.body.lookup_catch(self.ip);
            if catch_ip != new_catch_ip {
                if builder.seq.is_empty() {
                    catch_ip = new_catch_ip;
                } else {
                    trace!("  - ends at {} with new Catch Block", self.ip);
                    builder.forks.push(self.clone());
                    break;
                }
            }
            self.debug();
            self.step(&mut builder)?;
            self.debug_state();
        }

        trace!(
            "  - Forks: [{}]",
            builder
                .forks
                .iter()
                .map(|state| state.ip.to_string())
                .join(", ")
        );

        let seq = Sequence {
            instrs: builder.seq,
            debug_name,
            loopback,
        };

        let catch_state = if !catch_ip.is_none() {
            let state = self.clone_for_catch(catch_ip);
            trace!("  - Catch State:");
            state.debug_state();
            Some(state)
        } else {
            None
        };

        Ok(StateCollect {
            seq,
            forks: builder.forks,
            catch_state,
        })
    }

    fn fork(&self, builder: &mut InstrSeqBuilder<'arena, 'a, '_>, targets: &[Label]) {
        self.fork_and_adjust(builder, targets, |_, _, _| {})
    }

    fn fork_and_adjust<F>(
        &self,
        builder: &mut InstrSeqBuilder<'arena, 'a, '_>,
        targets: &[Label],
        and_then: F,
    ) where
        F: Fn(&mut Self, &mut InstrSeqBuilder<'arena, 'a, '_>, usize),
    {
        for (idx, target) in targets.iter().enumerate() {
            let mut fork = self.clone_with_jmp(target);
            // Some Instructs modify the stack on the receiving end
            and_then(&mut fork, builder, idx);
            trace!(
                "  FORK to label {} ({})",
                target,
                self.body.label_to_ip.get(target).unwrap()
            );
            fork.debug_state();
            builder.forks.push(fork);
        }
    }

    pub(crate) fn clone_with_jmp(&self, target: &Label) -> Self {
        let ip = *self.body.label_to_ip.get(target).unwrap();
        Self {
            body: self.body,
            debug_name: self.debug_name,
            ip,
            iterators: self.iterators.clone(),
            locals: self.locals.clone(),
            stack: self.stack.clone(),
            adata: self.adata,
        }
    }

    pub(crate) fn clone_for_catch(&self, ip: InstrPtr) -> Self {
        Self {
            body: self.body,
            debug_name: self.debug_name,
            ip,
            iterators: self.iterators.clone(),
            locals: self.locals.clone(),
            stack: vec![],
            adata: self.adata,
        }
    }

    pub(crate) fn is_done(&self) -> bool {
        self.ip.is_none()
    }

    pub(crate) fn debug(&self) {
        if let Some(instr) = self.instr() {
            let catch = if let Some(ip) = self.body.lookup_catch(self.ip).into_option() {
                format!(", catch {ip}")
            } else {
                "".to_string()
            };
            trace!("  {ip:4} INSTR: {instr:?}{catch}", ip = self.ip.to_string(),);
        } else {
            trace!("  DONE");
        }
    }

    fn debug_local_name(&self, local: Local) -> String {
        if !local.is_valid() {
            "NV".to_string()
        } else if let Some(name) = self.body.local_name(local) {
            format!("{}({})", name.as_str(), local)
        } else {
            format!("{}", local)
        }
    }

    pub(crate) fn debug_state(&self) {
        trace!(
            "  STACK: [{}], LOCALS: [{}], ITER: [{}]",
            self.stack.iter().map(ToString::to_string).join(", "),
            self.locals
                .iter()
                .sorted_by_key(|k| k.0)
                .map(|(k, v)| { format!("{} => {}", self.debug_local_name(*k), v) })
                .join(", "),
            self.iterators
                .iter()
                .sorted_by_key(|x| x.0)
                .map(|(idx, iter_state)| {
                    let key = self.debug_local_name(iter_state.key);
                    let value = self.debug_local_name(iter_state.value);
                    let base = iter_state.base;
                    format!("{idx} => {{ K: {key}, V: {value}, B: {base} }}")
                })
                .join(", "),
        );
    }

    /// Step the abstract virtual machine forward one instruction. See
    /// Self::step_default_handler() for the common handler.
    fn step(&mut self, builder: &mut InstrSeqBuilder<'arena, 'a, '_>) -> Result<()> {
        let instr = self.instr().unwrap();

        match *instr {
            Instruct::Opcode(
                ref opcode @ Opcode::ClsCnsD(_, _)
                | ref opcode @ Opcode::CnsE(_)
                | ref opcode @ Opcode::Dict(_)
                | ref opcode @ Opcode::Dir
                | ref opcode @ Opcode::Double(_)
                | ref opcode @ Opcode::EnumClassLabel(..)
                | ref opcode @ Opcode::False
                | ref opcode @ Opcode::File
                | ref opcode @ Opcode::FuncCred
                | ref opcode @ Opcode::Int(_)
                | ref opcode @ Opcode::Keyset(_)
                | ref opcode @ Opcode::LateBoundCls
                | ref opcode @ Opcode::LazyClass(..)
                | ref opcode @ Opcode::Method
                | ref opcode @ Opcode::NewCol(..)
                | ref opcode @ Opcode::NewDictArray(_)
                | ref opcode @ Opcode::Null
                | ref opcode @ Opcode::NullUninit
                | ref opcode @ Opcode::ParentCls
                | ref opcode @ Opcode::SelfCls
                | ref opcode @ Opcode::String(_)
                | ref opcode @ Opcode::This
                | ref opcode @ Opcode::True
                | ref opcode @ Opcode::Vec(_),
            ) => self.step_constant(builder, opcode)?,

            Instruct::Opcode(
                Opcode::BaseC(..)
                | Opcode::BaseGC(..)
                | Opcode::BaseH
                | Opcode::BaseGL(..)
                | Opcode::BaseL(..)
                | Opcode::BaseSC(..),
            ) => self.step_member_op(builder)?,

            Instruct::Opcode(Opcode::CreateCl(num_params, classname)) => {
                self.step_create_cl(builder, num_params, &classname)?;
            }

            Instruct::Opcode(
                Opcode::Dim(..)
                | Opcode::QueryM(..)
                | Opcode::SetM(..)
                | Opcode::IncDecM(..)
                | Opcode::SetOpM(..)
                | Opcode::UnsetM(..)
                | Opcode::SetRangeM(..),
            ) => {
                unreachable!();
            }
            Instruct::Opcode(Opcode::Dup) => {
                let tmp = self.stack_pop();
                self.stack_push(tmp);
                self.stack_push(tmp);
            }
            Instruct::Opcode(Opcode::CGetL(local)) => {
                self.stack_push(self.local_get(&local));
            }
            Instruct::Opcode(Opcode::CGetL2(local)) => {
                let tmp = self.stack_pop();
                self.stack_push(self.local_get(&local));
                self.stack_push(tmp);
            }
            Instruct::Opcode(Opcode::CGetQuietL(local)) => {
                self.stack_push(self.local_get(&local));
            }
            Instruct::Opcode(Opcode::CUGetL(local)) => {
                self.stack_push(self.local_get(&local));
            }
            Instruct::Opcode(Opcode::IncDecL(local, op)) => {
                self.step_inc_dec_l(builder, local, op);
            }
            Instruct::Opcode(Opcode::IsUnsetL(_local)) => todo!(),

            Instruct::Opcode(Opcode::PopL(local)) => {
                self.step_pop_l(builder, local);
            }
            Instruct::Opcode(Opcode::PushL(local)) => {
                self.step_push_l(local);
            }
            Instruct::Opcode(Opcode::SetL(local)) => {
                self.step_set_l(builder, local);
            }
            Instruct::Opcode(Opcode::SetOpL(local, set_op_op)) => {
                self.step_set_op_l(builder, local, set_op_op);
            }
            Instruct::Opcode(Opcode::UnsetL(local)) => {
                self.locals.remove(&local);
            }

            Instruct::Opcode(Opcode::LIterFree(..)) => todo!(),
            Instruct::Opcode(Opcode::LIterInit(..)) => todo!(),
            Instruct::Opcode(Opcode::LIterNext(..)) => todo!(),

            Instruct::Opcode(Opcode::MemoGetEager(targets, _, range)) => {
                self.step_memo_get_eager(builder, targets, range)?;
            }

            Instruct::Opcode(Opcode::IterInit(ref iter_args, target)) => {
                self.step_iter_init(builder, iter_args, target);
            }
            Instruct::Opcode(Opcode::IterFree(iter_id)) => self.step_iter_free(iter_id),
            Instruct::Opcode(Opcode::IterNext(ref iter_args, target)) => {
                self.step_iter_next(builder, iter_args, target)
            }

            Instruct::Opcode(Opcode::Switch(bounded, base, ref targets)) => {
                self.step_switch(builder, bounded, base, targets)
            }
            Instruct::Opcode(Opcode::SSwitch(ref cases, ref targets)) => {
                self.step_s_switch(builder, cases, targets)
            }

            Instruct::Opcode(Opcode::Enter(label) | Opcode::Jmp(label)) => {
                // Jmp and JmpNS need to update the next IP.
                // TODO: We should probably store the fact that a surprise check
                // jump occurred during the Sequence.
                let ip = *self.body.label_to_ip.get(&label).unwrap();
                self.ip = ip;
                return Ok(());
            }

            Instruct::Opcode(
                ref opcode @ Opcode::FCallClsMethod(..)
                | ref opcode @ Opcode::FCallClsMethodD(..)
                | ref opcode @ Opcode::FCallClsMethodM(..)
                | ref opcode @ Opcode::FCallClsMethodS(..)
                | ref opcode @ Opcode::FCallClsMethodSD(..)
                | ref opcode @ Opcode::FCallCtor(..)
                | ref opcode @ Opcode::FCallFunc(..)
                | ref opcode @ Opcode::FCallFuncD(..)
                | ref opcode @ Opcode::FCallObjMethod(..)
                | ref opcode @ Opcode::FCallObjMethodD(..),
            ) => self.step_fcall_handler(builder, opcode)?,

            Instruct::Opcode(ref opcode) => self.step_default_handler(builder, opcode)?,

            Instruct::Pseudo(
                Pseudo::TryCatchBegin | Pseudo::TryCatchMiddle | Pseudo::TryCatchEnd,
            ) => {}

            Instruct::Pseudo(Pseudo::SrcLoc(_)) => {}
            Instruct::Pseudo(Pseudo::Break) => todo!(),
            Instruct::Pseudo(Pseudo::Continue) => todo!(),
            Instruct::Pseudo(Pseudo::Label(..)) => {}
        }

        debug_assert!(
            match instr {
                Instruct::Opcode(opcode) => {
                    let data = crate::body::lookup_data_for_opcode(opcode);
                    self.ip.is_none() || !data.flags.contains(InstrFlags::TF)
                }
                Instruct::Pseudo(_) => true,
            },
            "Instr {:?} is marked as TF but didn't clear 'ip'",
            instr
        );

        self.ip = self.next_ip(self.ip);

        Ok(())
    }

    fn step_member_op(&mut self, builder: &mut InstrSeqBuilder<'arena, 'a, '_>) -> Result<()> {
        let mut inputs = Vec::new();
        let src_loc = Rc::clone(self.body.ip_to_loc(self.ip));

        //-- let mutates_stack_base = self.member_op_mutates_stack_base();
        let base_op = self.step_base(builder, &mut inputs)?;

        trace!(
            "  + {ip:4} INSTR: {instr:?}",
            ip = self.ip.to_string(),
            instr = self.instr()
        );
        let mut intermediate_ops = Vec::new();
        while let Some(iop) = self.step_dim(&mut inputs)? {
            intermediate_ops.push(iop);
            trace!(
                "  + {ip:4} INSTR: {instr:?}",
                ip = self.ip.to_string(),
                instr = self.instr()
            );
        }

        let final_op = self.step_final(&mut inputs)?;

        let is_write = final_op.is_write();
        if !is_write {
            inputs[0] = inputs[0].to_read_only();
        }

        let pushes_value = final_op.pushes_value();

        // TODO: if it's a COW then we probably should have the base replace its
        // stack/local value with a new computed value.

        let instr = NodeInstr::MemberOp(node::MemberOp {
            base_op,
            intermediate_ops,
            final_op,
        });

        if pushes_value {
            let output = builder.compute_value(&instr, 0, &inputs);
            self.stack_push(output);
        }

        self.seq_push_with_loc(builder, instr, inputs, src_loc);
        Ok(())
    }

    fn next_ip(&self, ip: InstrPtr) -> InstrPtr {
        ip.next(self.body.hhbc_body.body_instrs.len())
    }

    fn step_base(
        &mut self,
        builder: &mut InstrSeqBuilder<'arena, 'a, '_>,
        inputs: &mut Vec<Input<'arena>>,
    ) -> Result<node::BaseOp> {
        let instr = self.instr().unwrap();
        let src_loc = Rc::clone(self.body.ip_to_loc(self.ip));
        self.ip = self.next_ip(self.ip);

        Ok(match *instr {
            Instruct::Opcode(Opcode::BaseC(idx, mode)) => {
                // Get base from value.
                let base = self.stack_get_n(idx as usize)?;
                inputs.push(self.reffy(base));
                node::BaseOp::Base(mode, ReadonlyOp::Any, src_loc)
            }
            Instruct::Opcode(Opcode::BaseGC(idx, mode)) => {
                // Get base from global name.
                let base = self.stack_get_n(idx as usize)?;
                inputs.push(self.reffy(base));
                node::BaseOp::BaseG(mode, src_loc)
            }
            Instruct::Opcode(Opcode::BaseL(local, mode, readonly)) => {
                // Get base from local.
                let base = self.local_get(&local);
                inputs.push(self.reffy(base));
                node::BaseOp::Base(mode, readonly, src_loc)
            }
            Instruct::Opcode(Opcode::BaseH) => {
                // Get base from $this.
                let base = builder.compute_value(&NodeInstr::Opcode(Opcode::This), 0, &[]);
                inputs.push(self.reffy(base));
                node::BaseOp::BaseH(src_loc)
            }
            Instruct::Opcode(Opcode::BaseSC(prop, cls, mode, readonly)) => {
                // Get base from static property.
                let prop = self.stack_get_n(prop as usize)?;
                let cls = self.stack_get_n(cls as usize)?;
                inputs.push(self.non_cow(prop));
                inputs.push(self.non_cow(cls));
                node::BaseOp::BaseSC(mode, readonly, src_loc)
            }
            _ => todo!("Op: {instr:?}"),
        })
    }

    fn step_dim(
        &mut self,
        inputs: &mut Vec<Input<'arena>>,
    ) -> Result<Option<node::IntermediateOp<'arena>>> {
        // Loop because we may have to skip intermediate SrcLoc.
        loop {
            let instr = self.instr();
            match instr {
                Some(Instruct::Pseudo(Pseudo::SrcLoc(..))) => {
                    self.ip = self.next_ip(self.ip);
                }
                Some(Instruct::Opcode(Opcode::Dim(mode, key))) => {
                    let src_loc = Rc::clone(self.body.ip_to_loc(self.ip));
                    self.ip = self.next_ip(self.ip);

                    let key = self.push_member_key_inputs(inputs, key)?;
                    return Ok(Some(node::IntermediateOp {
                        key,
                        mode: *mode,
                        src_loc,
                    }));
                }
                _ => return Ok(None),
            }
        }
    }

    fn step_final(&mut self, inputs: &mut Vec<Input<'arena>>) -> Result<node::FinalOp<'arena>> {
        let instr = self
            .instr()
            .ok_or_else(|| anyhow!("Early end in MemberOp sequence"))?;
        let src_loc = Rc::clone(self.body.ip_to_loc(self.ip));

        // purposely don't increment self.ip - it will be incremented by the
        // outer 'step' fn.

        let final_op = match *instr {
            Instruct::Opcode(Opcode::QueryM(n_stack, query_op, ref key)) => {
                let key = self.push_member_key_inputs(inputs, key)?;
                self.stack_pop_n(n_stack as usize)?;
                node::FinalOp::QueryM(key, query_op, src_loc)
            }
            Instruct::Opcode(Opcode::SetM(n_stack, ref key)) => {
                let key = self.push_member_key_inputs(inputs, key)?;
                let value = self.stack_pop();
                inputs.push(self.reffy(value));
                self.stack_pop_n(n_stack as usize)?;
                node::FinalOp::SetM(key, src_loc)
            }
            Instruct::Opcode(Opcode::SetOpM(n_stack, op, ref key)) => {
                let key = self.push_member_key_inputs(inputs, key)?;
                let value = self.stack_pop();
                inputs.push(self.reffy(value));
                self.stack_pop_n(n_stack as usize)?;
                node::FinalOp::SetOpM(key, op, src_loc)
            }
            Instruct::Opcode(Opcode::SetRangeM(n_stack, sz, op)) => {
                let s1 = self.stack_pop();
                inputs.push(self.reffy(s1));
                let s2 = self.stack_pop();
                inputs.push(self.reffy(s2));
                let s3 = self.stack_pop();
                inputs.push(self.reffy(s3));
                self.stack_pop_n(n_stack as usize)?;
                node::FinalOp::SetRangeM(sz, op, src_loc)
            }
            Instruct::Opcode(Opcode::IncDecM(n_stack, op, ref key)) => {
                let key = self.push_member_key_inputs(inputs, key)?;
                self.stack_pop_n(n_stack as usize)?;
                node::FinalOp::IncDecM(key, op, src_loc)
            }
            Instruct::Opcode(Opcode::UnsetM(n_stack, ref key)) => {
                let key = self.push_member_key_inputs(inputs, key)?;
                self.stack_pop_n(n_stack as usize)?;
                node::FinalOp::UnsetM(key, src_loc)
            }
            _ => todo!("Op: {instr:?}"),
        };

        Ok(final_op)
    }

    fn step_create_cl(
        &mut self,
        builder: &mut InstrSeqBuilder<'arena, 'a, '_>,
        num_params: NumParams,
        classname: &ClassName<'arena>,
    ) -> Result<()> {
        let mut inputs = self
            .stack_pop_n(num_params as usize)?
            .into_iter()
            .map(|v| self.reffy(v))
            .collect_vec();
        inputs.push(Input::Class(classname.unsafe_into_string()));
        let instr = NodeInstr::Opcode(Opcode::CreateCl(num_params, classname.clone()));
        let output = builder.compute_value(&instr, 0, &inputs);
        self.stack_push(output);
        Ok(())
    }

    fn step_constant(
        &mut self,
        builder: &mut InstrSeqBuilder<'arena, 'a, '_>,
        opcode: &Opcode<'arena>,
    ) -> Result<()> {
        let clean_instr = NodeInstr::Opcode(clean_opcode(opcode));
        debug_assert_eq!(opcode.num_inputs(), 0);
        debug_assert!(matches!(LocalInfo::for_opcode(opcode), LocalInfo::None));
        debug_assert!(opcode.targets().is_empty());
        let data = crate::body::lookup_data_for_opcode(opcode);
        debug_assert!(match &data.outputs {
            Outputs::Fixed(n) => n.len() == 1,
            _ => false,
        });
        debug_assert!(
            !is_checkpoint_instr(&clean_instr),
            "checkpoint: {clean_instr:?}",
        );
        debug_assert!(!data.flags.contains(InstrFlags::TF));

        // For a constant the outputs are based entirely on the input instr.
        let output = match opcode {
            Opcode::Dict(id) | Opcode::Keyset(id) | Opcode::Vec(id) => {
                // But for an array-based constant we want to use the array data as an input.
                builder.compute_value(&clean_instr, 0, &[Input::ConstantArray(self.adata[id])])
            }
            _ => builder.compute_constant(&clean_instr),
        };
        self.stack_push(output);

        Ok(())
    }

    fn step_default_handler(
        &mut self,
        builder: &mut InstrSeqBuilder<'arena, 'a, '_>,
        opcode: &Opcode<'arena>,
    ) -> Result<()> {
        // Start by computing the inputs for this opcode. For a 'default'
        // opcode we can handle stack and locals.
        let mut inputs: Vec<Input<'arena>> = self
            .stack_pop_n(opcode.num_inputs())?
            .into_iter()
            .map(|v| self.reffy(v))
            .collect_vec();

        let local_info = LocalInfo::for_opcode(opcode);
        match local_info {
            LocalInfo::None | LocalInfo::Write(_) => {}
            LocalInfo::Read(ref local) | LocalInfo::Mutate(ref local) => {
                inputs.push(self.reffy(self.local_get(local)));
            }
            LocalInfo::ReadRange(range) => {
                for local in range.iter() {
                    inputs.push(self.reffy(self.local_get(&local)));
                }
            }
        }

        // For any targets that the opcode refers to, create a state fork to
        // handle that later.
        self.fork(builder, opcode.targets());

        // Mock up a "clean" version of the Instruct with the Locals and Labels
        // removed (set to Local::INVALID and Label::INVALID).
        let clean_instr = NodeInstr::Opcode(clean_opcode(opcode));

        // Figure out how many stack outputs we'll need. For 'default'
        // instructions this should be 0 or 1.
        let data = crate::body::lookup_data_for_opcode(opcode);
        let n = match &data.outputs {
            Outputs::NOV => 0,
            Outputs::Fixed(outputs) => outputs.len(),
            Outputs::FCall => unreachable!(),
        };

        // The outputs are based on: (the "cleaned" instr, inputs, the output
        // index)
        let outputs: Vec<Value> = (0..n)
            .map(|i| builder.compute_value(&clean_instr, i, &inputs))
            .collect();

        self.stack_push_n(&outputs);

        // If the instr writes to a local then do that now.
        match local_info {
            LocalInfo::None | LocalInfo::Read(_) | LocalInfo::ReadRange(_) => {}
            LocalInfo::Write(ref local) | LocalInfo::Mutate(ref local) => {
                let output = builder.compute_value(&clean_instr, n, &inputs);
                self.local_set(local, output);
            }
        }

        // Finally if this is a checkpoint instr (where the two sequences must
        // match up) then push it onto the sequence list.
        if is_checkpoint_instr(&clean_instr) {
            self.seq_push(builder, clean_instr, inputs);
        }

        if data.flags.contains(InstrFlags::TF) {
            // This opcode ends the sequence (like RetC).
            self.ip = InstrPtr::None;
        }

        Ok(())
    }

    fn push_member_key_inputs(
        &mut self,
        inputs: &mut Vec<Input<'arena>>,
        member_key: &MemberKey<'arena>,
    ) -> Result<MemberKey<'arena>> {
        match *member_key {
            MemberKey::EI(..)
            | MemberKey::ET(..)
            | MemberKey::PT(..)
            | MemberKey::QT(..)
            | MemberKey::W => {}
            MemberKey::EC(idx, _) | MemberKey::PC(idx, _) => {
                inputs.push(self.non_cow(self.stack_get_n(idx as usize)?));
            }
            MemberKey::EL(local, _) | MemberKey::PL(local, _) => {
                inputs.push(self.non_cow(self.local_get(&local)));
            }
        }
        Ok(clean_member_key(member_key))
    }

    fn step_fcall_handler(
        &mut self,
        builder: &mut InstrSeqBuilder<'arena, 'a, '_>,
        opcode: &Opcode<'arena>,
    ) -> Result<()> {
        let data = crate::body::lookup_data_for_opcode(opcode);

        fn sanitize_fca(
            FCallArgs {
                flags,
                async_eager_target: _,
                num_args,
                num_rets,
                inouts,
                readonly,
                context,
            }: &FCallArgs,
        ) -> FCallArgs {
            // Turn a non-empty, all-false slice into an empty slice.
            let inouts = if !inouts.is_empty() && !inouts.iter().any(|x| *x) {
                Default::default()
            } else {
                inouts.clone()
            };
            // Turn a non-empty, all-false slice into an empty slice.
            let readonly = if !readonly.is_empty() && !readonly.iter().any(|x| *x) {
                Default::default()
            } else {
                readonly.clone()
            };
            FCallArgs {
                flags: *flags,
                async_eager_target: Label::INVALID,
                num_args: *num_args,
                num_rets: *num_rets,
                inouts,
                readonly,
                context: *context,
            }
        }

        let (fca, opcode) = match *opcode {
            Opcode::FCallClsMethod(ref fca, hint, log) => {
                (fca, Opcode::FCallClsMethod(sanitize_fca(fca), hint, log))
            }
            Opcode::FCallClsMethodD(ref fca, class, method) => (
                fca,
                Opcode::FCallClsMethodD(sanitize_fca(fca), class, method),
            ),
            Opcode::FCallClsMethodM(ref fca, hint, log, method) => (
                fca,
                Opcode::FCallClsMethodM(sanitize_fca(fca), hint, log, method),
            ),
            Opcode::FCallClsMethodS(ref fca, hint, clsref) => (
                fca,
                Opcode::FCallClsMethodS(sanitize_fca(fca), hint, clsref),
            ),
            Opcode::FCallClsMethodSD(ref fca, hint, clsref, method) => (
                fca,
                Opcode::FCallClsMethodSD(sanitize_fca(fca), hint, clsref, method),
            ),
            Opcode::FCallCtor(ref fca, hint) => (fca, Opcode::FCallCtor(sanitize_fca(fca), hint)),
            Opcode::FCallFunc(ref fca) => (fca, Opcode::FCallFunc(sanitize_fca(fca))),
            Opcode::FCallFuncD(ref fca, name) => (fca, Opcode::FCallFuncD(sanitize_fca(fca), name)),
            Opcode::FCallObjMethod(ref fca, hint, obj_method_op) => (
                fca,
                Opcode::FCallObjMethod(sanitize_fca(fca), hint, obj_method_op),
            ),
            Opcode::FCallObjMethodD(ref fca, hint, obj_method_op, method) => (
                fca,
                Opcode::FCallObjMethodD(sanitize_fca(fca), hint, obj_method_op, method),
            ),
            _ => unreachable!(),
        };

        let num_inouts = fca.num_inouts();
        let num_inputs = opcode.num_inputs();
        let inputs = self
            .stack_pop_n(num_inputs)?
            .into_iter()
            .take(num_inputs - num_inouts)
            .map(|v| self.reffy(v))
            .collect_vec();

        let instr = NodeInstr::Opcode(opcode);

        if fca.async_eager_target != Label::INVALID {
            // We're an async call...
            self.fork_and_adjust(
                builder,
                &[fca.async_eager_target],
                |fork, builder, target_idx| {
                    // Async will push the eager value onto the stack before
                    // returning.
                    // (target_idx == 0..num_rets) are the "normal" fcall return.
                    // (target_idx == num_rets) is the "eager async" fcall return.
                    let value =
                        builder.compute_value(&instr, target_idx + fca.num_rets as usize, &inputs);
                    fork.stack_push(value);
                },
            );
        }

        let n = match &data.outputs {
            Outputs::NOV => 0,
            Outputs::Fixed(outputs) => outputs.len(),
            Outputs::FCall => fca.num_rets as usize,
        };

        let outputs: Vec<Value> = (0..n)
            .map(|i| builder.compute_value(&instr, i, &inputs))
            .collect();

        self.stack_push_n(&outputs);
        self.seq_push(builder, instr, inputs);

        Ok(())
    }

    fn step_inc_dec_l(
        &mut self,
        builder: &mut InstrSeqBuilder<'arena, 'a, '_>,
        local: Local,
        inc_dec_op: IncDecOp,
    ) {
        let pre_value = self.local_get(&local);
        // For comparison we need to strip the local itself out of the opcode.
        let instr = NodeInstr::Opcode(Opcode::IncDecL(Local::INVALID, inc_dec_op));
        let post_value = builder.compute_value(&instr, 0, &[self.reffy(pre_value)]);
        self.local_set(&local, post_value);
        let value = apply_inc_dec_op(inc_dec_op, pre_value, post_value);
        self.stack_push(value);
        let inputs = vec![self.reffy(pre_value)];
        self.seq_push(builder, instr, inputs);
    }

    fn step_iter_free(&mut self, iter_id: IterId) {
        // IterFree just clears the iterator state - we don't need the
        // two sides to clear the iterator at the same time.
        self.iterators.remove(&iter_id);
    }

    fn step_iter_init(
        &mut self,
        builder: &mut InstrSeqBuilder<'arena, 'a, '_>,
        iter_args: &IterArgs,
        target: Label,
    ) {
        // IterArgs { iter_id: IterId, key_id: Local, val_id: Local }
        let base = self.stack_pop();
        let inputs = vec![self.reffy(base)];

        self.fork_and_adjust(builder, &[target], |_, _, _| {
            // The fork implicitly does a IterFree - but we haven't yet
            // registered our iterator - so we don't have to clear it.
        });

        let instr = NodeInstr::Opcode(Opcode::IterInit(IterArgs::default(), Label::INVALID));
        let key_value = builder.compute_value(&instr, 0, &inputs);
        let value_value = builder.compute_value(&instr, 1, &inputs);
        self.seq_push(builder, instr, inputs);
        self.iterators.insert(
            iter_args.iter_id,
            IterState {
                key: iter_args.key_id,
                value: iter_args.val_id,
                base,
            },
        );
        if iter_args.key_id != Local::INVALID {
            self.local_set(&iter_args.key_id, key_value);
        }
        self.local_set(&iter_args.val_id, value_value);
    }

    fn step_iter_next(
        &mut self,
        builder: &mut InstrSeqBuilder<'arena, 'a, '_>,
        iter_args: &IterArgs,
        target: Label,
    ) {
        if let Some(iterator) = self.iterators.get(&iter_args.iter_id) {
            let inputs = vec![self.reffy(iterator.base)];

            let instr = NodeInstr::Opcode(Opcode::IterNext(IterArgs::default(), Label::INVALID));

            self.fork_and_adjust(builder, &[target], |fork, builder, _| {
                if iter_args.key_id != Local::INVALID {
                    let key_value = builder.compute_value(&instr, 0, &inputs);
                    fork.local_set(&iter_args.key_id, key_value);
                }

                let value_value = builder.compute_value(&instr, 1, &inputs);
                fork.local_set(&iter_args.val_id, value_value);
            });

            self.seq_push(builder, instr, inputs);

            // Implicit IterFree...
            self.iterators.remove(&iter_args.iter_id);
        }
    }

    fn step_memo_get_eager(
        &mut self,
        builder: &mut InstrSeqBuilder<'arena, 'a, '_>,
        targets: [Label; 2],
        range: LocalRange,
    ) -> Result<()> {
        let inputs = range
            .iter()
            .map(|local| self.reffy(self.local_get(&local)))
            .collect_vec();

        let clean_instr = NodeInstr::Opcode(Opcode::MemoGetEager(
            [Label::INVALID, Label::INVALID],
            Dummy::DEFAULT,
            LocalRange::EMPTY,
        ));

        self.fork_and_adjust(builder, &targets, |fork, builder, target_idx| {
            // target_idx == 0 - no value present
            // target_idx == 1 - suspended wait-handle
            if target_idx == 1 {
                let output = builder.compute_value(&clean_instr, 1 + target_idx, &inputs);
                fork.stack_push(output);
            }
        });
        // no fork - eagerly returned value present

        let output = builder.compute_value(&clean_instr, 0, &inputs);
        self.stack_push(output);

        self.seq_push(builder, clean_instr, inputs);

        Ok(())
    }

    fn step_pop_l(&mut self, builder: &mut InstrSeqBuilder<'arena, 'a, '_>, local: Local) {
        let value = self.stack_pop();
        self.local_set(&local, value);

        if self.is_decl_var(&local) {
            let inputs = vec![self.reffy(value)];
            let instr = NodeInstr::Opcode(Opcode::SetL(Local::INVALID));
            self.seq_push(builder, instr, inputs);
        }
    }

    fn step_push_l(&mut self, local: Local) {
        let value = self.local_get(&local);
        self.stack_push(value);
        self.local_set(&local, Value::Undefined);
    }

    fn step_s_switch(
        &mut self,
        builder: &mut InstrSeqBuilder<'arena, 'a, '_>,
        cases: &[Str<'arena>],
        targets: &[Label],
    ) {
        let value = self.stack_pop();

        self.fork(builder, targets.as_ref());

        let inputs = vec![self.reffy(value)];
        let instr = NodeInstr::Opcode(Opcode::SSwitch(cases.to_vec().into(), vec![].into()));
        self.seq_push(builder, instr, inputs);
        self.ip = InstrPtr::None;
    }

    fn step_switch(
        &mut self,
        builder: &mut InstrSeqBuilder<'arena, 'a, '_>,
        bounded: SwitchKind,
        base: i64,
        targets: &[Label],
    ) {
        let value = self.stack_pop();

        self.fork(builder, targets.as_ref());

        let inputs = vec![self.reffy(value)];
        let instr = NodeInstr::Opcode(Opcode::Switch(bounded, base, vec![].into()));
        self.seq_push(builder, instr, inputs);
        self.ip = InstrPtr::None;
    }

    fn step_set_l(&mut self, builder: &mut InstrSeqBuilder<'arena, 'a, '_>, local: Local) {
        let value = self.stack_top();
        self.local_set(&local, value);

        if self.is_decl_var(&local) {
            let inputs = vec![self.reffy(value)];
            let instr = NodeInstr::Opcode(Opcode::SetL(Local::INVALID));
            self.seq_push(builder, instr, inputs);
        }
    }

    fn step_set_op_l(
        &mut self,
        builder: &mut InstrSeqBuilder<'arena, 'a, '_>,
        local: Local,
        set_op_op: SetOpOp,
    ) {
        let value = self.stack_pop();
        let local_value = self.local_get(&local);

        let inputs = vec![self.reffy(local_value), self.reffy(value)];
        let instr = NodeInstr::Opcode(Opcode::SetOpL(Local::INVALID, set_op_op));
        let output = builder.compute_value(&instr, 0, &inputs);
        self.seq_push(builder, instr, inputs);

        self.local_set(&local, output);
        self.stack_push(output);
    }

    fn seq_push(
        &self,
        builder: &mut InstrSeqBuilder<'arena, 'a, '_>,
        instr: NodeInstr<'arena>,
        inputs: impl Into<Box<[Input<'arena>]>>,
    ) {
        let src_loc = Rc::clone(self.body.ip_to_loc(self.ip));
        self.seq_push_with_loc(builder, instr, inputs, src_loc);
    }

    fn seq_push_with_loc(
        &self,
        builder: &mut InstrSeqBuilder<'arena, 'a, '_>,
        instr: NodeInstr<'arena>,
        inputs: impl Into<Box<[Input<'arena>]>>,
        src_loc: Rc<SrcLoc>,
    ) {
        let inputs = inputs.into();
        trace!(
            "    SEQ w/ INPUTS [{}]",
            inputs.iter().map(|inp| inp.to_string()).join(", ")
        );
        builder.seq.push(Node {
            instr,
            inputs,
            src_loc,
        });
    }

    pub(crate) fn instr(&self) -> Option<&'arena Instruct<'arena>> {
        self.instr_at(self.ip)
    }

    pub(crate) fn instr_at(&self, ip: InstrPtr) -> Option<&'arena Instruct<'arena>> {
        ip.into_option().and_then(|ip| {
            let idx = ip.as_usize();
            self.body.hhbc_body.body_instrs.get(idx)
        })
    }

    pub(crate) fn stack_get_n(&self, n: usize) -> Result<Value> {
        let idx = self.stack.len() - 1 - n;
        self.stack
            .get(idx)
            .copied()
            .ok_or_else(|| anyhow!("Invalid stack index {idx} (max {})", self.stack.len()))
    }

    pub(crate) fn stack_push(&mut self, value: Value) {
        self.stack.push(value);
    }

    pub(crate) fn stack_push_n(&mut self, v: &[Value]) {
        self.stack.extend(v);
    }

    pub(crate) fn stack_pop(&mut self) -> Value {
        self.stack.pop().unwrap()
    }

    pub(crate) fn stack_pop_n(&mut self, n: usize) -> Result<Vec<Value>> {
        if n > self.stack.len() {
            bail!("Invalid stack length {n} (max {})", self.stack.len());
        }
        Ok(self.stack.split_off(self.stack.len() - n))
    }

    pub(crate) fn stack_top(&self) -> Value {
        self.stack.last().copied().unwrap_or(Value::Undefined)
    }

    pub(crate) fn local_set(&mut self, local: &Local, value: Value) {
        assert!(local != &Local::INVALID);
        if value.is_undefined() {
            self.locals.remove(local);
        } else {
            self.locals.insert(*local, value);
        }
    }

    pub(crate) fn local_get(&self, local: &Local) -> Value {
        self.locals.get(local).copied().unwrap_or(Value::Undefined)
    }

    fn is_decl_var(&self, local: &Local) -> bool {
        let idx = local.idx as usize;
        let params_len = self.body.hhbc_body.params.len();
        let decl_vars_len = self.body.hhbc_body.decl_vars.len();
        params_len <= idx && idx < params_len + decl_vars_len
    }
}

#[derive(Clone)]
pub(crate) struct IterState {
    pub(crate) key: Local,
    pub(crate) value: Local,
    pub(crate) base: Value,
}

type IterIdMap<V> = std::collections::HashMap<IterId, V, BuildIdHasher<u32>>;

struct InstrSeqBuilder<'arena, 'a, 'b> {
    seq: Vec<Node<'arena>>,
    forks: Vec<State<'arena, 'a>>,
    value_builder: &'b mut ValueBuilder<'arena>,
}

impl<'arena, 'a, 'b> InstrSeqBuilder<'arena, 'a, 'b> {
    fn compute_value(
        &mut self,
        instr: &NodeInstr<'arena>,
        idx: usize,
        inputs: &[Input<'arena>],
    ) -> Value {
        // The Instruct used to compute the value shouldn't have any
        // non-comparable bits: Local or Target
        for local in LocalInfo::for_node(instr).locals().iter() {
            assert!(!local.is_valid());
        }
        for target in instr.targets() {
            assert!(!target.is_valid());
        }
        self.value_builder
            .compute_value(instr, idx, inputs.to_vec().into_boxed_slice())
    }

    fn compute_constant(&mut self, instr: &NodeInstr<'arena>) -> Value {
        // The Instruct used to compute the value shouldn't have any
        // non-comparable bits: Local or Target
        for local in LocalInfo::for_node(instr).locals().iter() {
            assert!(!local.is_valid());
        }
        for target in instr.targets() {
            assert!(!target.is_valid());
        }
        self.value_builder.compute_constant(instr)
    }
}

pub(crate) struct StateCollect<'arena, 'a> {
    pub(crate) seq: Sequence<'arena>,
    pub(crate) forks: Vec<State<'arena, 'a>>,
    pub(crate) catch_state: Option<State<'arena, 'a>>,
}

/// Returns true if the Instruct is one where we should stop and compare state
/// between the two sides. Common examples are places where non-local
/// side-effects happen such as calls.
///
/// Some guiding principals:
///   - Instructs that push constants onto the stack should return false.
///   - Instructs that simply manipulate the stack and/or locals (PopC, SetL)
///     should return false.
///   - By default other Instructs should return true.
fn is_checkpoint_instr(instr: &NodeInstr<'_>) -> bool {
    match instr {
        // Constants
        NodeInstr::Opcode(
            Opcode::ClsCnsD(_, _)
            | Opcode::CnsE(_)
            | Opcode::CreateSpecialImplicitContext
            | Opcode::Dict(..)
            | Opcode::Dir
            | Opcode::Double(..)
            | Opcode::EnumClassLabel(..)
            | Opcode::False
            | Opcode::File
            | Opcode::FuncCred
            | Opcode::Int(..)
            | Opcode::Keyset(..)
            | Opcode::LateBoundCls
            | Opcode::LazyClass(..)
            | Opcode::LazyClassFromClass
            | Opcode::Method
            | Opcode::NewCol(..)
            | Opcode::NewDictArray(_)
            | Opcode::Null
            | Opcode::NullUninit
            | Opcode::ParentCls
            | Opcode::SelfCls
            | Opcode::String(..)
            | Opcode::This
            | Opcode::True
            | Opcode::Vec(..),
        ) => false,

        // Stack/Local manipulation
        NodeInstr::Opcode(
            Opcode::CGetL(..)
            | Opcode::CGetL2(..)
            | Opcode::Dup
            | Opcode::Lt
            | Opcode::Mod
            | Opcode::PopC
            | Opcode::PopL(..)
            | Opcode::PushL(..)
            | Opcode::Same
            | Opcode::SetL(..)
            | Opcode::UnsetL(..),
        ) => false,

        // Is operations
        NodeInstr::Opcode(
            Opcode::InstanceOf
            | Opcode::InstanceOfD(..)
            | Opcode::IsLateBoundCls
            | Opcode::IsTypeC(..)
            | Opcode::IsTypeL(..)
            | Opcode::IsTypeStructC(..)
            | Opcode::IsUnsetL(..)
            | Opcode::IssetG
            | Opcode::IssetL(..)
            | Opcode::IssetS,
        ) => false,

        // Simple control flow
        NodeInstr::Opcode(Opcode::Enter(_) | Opcode::Jmp(_) | Opcode::Nop) => false,

        // Verify Instructions
        NodeInstr::Opcode(
            Opcode::VerifyImplicitContextState
            | Opcode::VerifyOutType(..)
            | Opcode::VerifyParamType(..)
            | Opcode::VerifyParamTypeTS(..)
            | Opcode::VerifyRetNonNullC
            | Opcode::VerifyRetTypeC
            | Opcode::VerifyRetTypeTS,
        ) => true,

        // Other Opcodes
        NodeInstr::Opcode(Opcode::Concat | Opcode::ConcatN(..)) => false,

        NodeInstr::Opcode(
            Opcode::AKExists
            | Opcode::Add
            | Opcode::AddElemC
            | Opcode::AddNewElemC
            | Opcode::ArrayIdx
            | Opcode::ArrayMarkLegacy
            | Opcode::ArrayUnmarkLegacy
            | Opcode::AssertRATL(..)
            | Opcode::AssertRATStk(..)
            | Opcode::Await
            | Opcode::AwaitAll(..)
            | Opcode::BareThis(..)
            | Opcode::BaseC(..)
            | Opcode::BaseGC(..)
            | Opcode::BaseGL(..)
            | Opcode::BaseH
            | Opcode::BaseL(..)
            | Opcode::BaseSC(..)
            | Opcode::BitAnd
            | Opcode::BitNot
            | Opcode::BitOr
            | Opcode::BitXor
            | Opcode::BreakTraceHint
            | Opcode::CGetCUNop
            | Opcode::CGetG
            | Opcode::CGetQuietL(..)
            | Opcode::CGetS(..)
            | Opcode::CUGetL(..)
            | Opcode::CastBool
            | Opcode::CastDict
            | Opcode::CastDouble
            | Opcode::CastInt
            | Opcode::CastKeyset
            | Opcode::CastString
            | Opcode::CastVec
            | Opcode::ChainFaults
            | Opcode::CheckProp(..)
            | Opcode::CheckClsReifiedGenericMismatch
            | Opcode::CheckClsRGSoft
            | Opcode::CheckThis
            | Opcode::ClassGetC(..)
            | Opcode::ClassGetTS
            | Opcode::ClassHasReifiedGenerics
            | Opcode::ClassName
            | Opcode::Clone
            | Opcode::ClsCns(..)
            | Opcode::ClsCnsL(..)
            | Opcode::Cmp
            | Opcode::ColFromArray(..)
            | Opcode::CombineAndResolveTypeStruct(..)
            | Opcode::ContCheck(..)
            | Opcode::ContCurrent
            | Opcode::ContEnter
            | Opcode::ContGetReturn
            | Opcode::ContKey
            | Opcode::ContRaise
            | Opcode::ContValid
            | Opcode::CreateCl(..)
            | Opcode::CreateCont
            | Opcode::DblAsBits
            | Opcode::Dim(..)
            | Opcode::Div
            | Opcode::EnumClassLabelName
            | Opcode::Eq
            | Opcode::Eval
            | Opcode::Exit
            | Opcode::FCallClsMethod(..)
            | Opcode::FCallClsMethodD(..)
            | Opcode::FCallClsMethodM(..)
            | Opcode::FCallClsMethodS(..)
            | Opcode::FCallClsMethodSD(..)
            | Opcode::FCallCtor(..)
            | Opcode::FCallFunc(..)
            | Opcode::FCallFuncD(..)
            | Opcode::FCallObjMethod(..)
            | Opcode::FCallObjMethodD(..)
            | Opcode::Fatal(..)
            | Opcode::GetClsRGProp
            | Opcode::GetMemoKeyL(..)
            | Opcode::Gt
            | Opcode::Gte
            | Opcode::HasReifiedParent
            | Opcode::Idx
            | Opcode::IncDecG(..)
            | Opcode::IncDecL(..)
            | Opcode::IncDecM(..)
            | Opcode::IncDecS(..)
            | Opcode::Incl
            | Opcode::InclOnce
            | Opcode::InitProp(..)
            | Opcode::IterFree(..)
            | Opcode::IterInit(..)
            | Opcode::IterNext(..)
            | Opcode::JmpNZ(..)
            | Opcode::JmpZ(..)
            | Opcode::LIterFree(..)
            | Opcode::LIterInit(..)
            | Opcode::LIterNext(..)
            | Opcode::LockObj
            | Opcode::Lte
            | Opcode::MemoGet(..)
            | Opcode::MemoGetEager(..)
            | Opcode::MemoSet(..)
            | Opcode::MemoSetEager(..)
            | Opcode::Mul
            | Opcode::NSame
            | Opcode::NativeImpl
            | Opcode::Neq
            | Opcode::NewKeysetArray(..)
            | Opcode::NewObj
            | Opcode::NewObjD(..)
            | Opcode::NewObjS(..)
            | Opcode::NewPair
            | Opcode::NewStructDict(..)
            | Opcode::NewVec(..)
            | Opcode::Not
            | Opcode::OODeclExists(..)
            | Opcode::PopU
            | Opcode::PopU2
            | Opcode::Pow
            | Opcode::Print
            | Opcode::QueryM(..)
            | Opcode::RaiseClassStringConversionNotice
            | Opcode::RecordReifiedGeneric
            | Opcode::Req
            | Opcode::ReqDoc
            | Opcode::ReqOnce
            | Opcode::ResolveClass(..)
            | Opcode::ResolveClsMethod(..)
            | Opcode::ResolveClsMethodD(..)
            | Opcode::ResolveClsMethodS(..)
            | Opcode::ResolveFunc(..)
            | Opcode::ResolveMethCaller(..)
            | Opcode::ResolveRClsMethod(..)
            | Opcode::ResolveRClsMethodD(..)
            | Opcode::ResolveRClsMethodS(..)
            | Opcode::ResolveRFunc(..)
            | Opcode::RetC
            | Opcode::RetCSuspended
            | Opcode::RetM(..)
            | Opcode::SSwitch { .. }
            | Opcode::Select
            | Opcode::SetG
            | Opcode::SetImplicitContextByValue
            | Opcode::SetM(..)
            | Opcode::SetOpG(..)
            | Opcode::SetOpL(..)
            | Opcode::SetOpM(..)
            | Opcode::SetOpS(..)
            | Opcode::SetRangeM(..)
            | Opcode::SetS(..)
            | Opcode::Shl
            | Opcode::Shr
            | Opcode::Silence(..)
            | Opcode::Sub
            | Opcode::Switch(..)
            | Opcode::Throw
            | Opcode::ThrowAsTypeStructException(_)
            | Opcode::ThrowNonExhaustiveSwitch
            | Opcode::UGetCUNop
            | Opcode::UnsetG
            | Opcode::UnsetM(..)
            | Opcode::WHResult
            | Opcode::Yield
            | Opcode::YieldK,
        ) => true,

        NodeInstr::MemberOp(_) => true,
    }
}

fn apply_inc_dec_op(inc_dec_op: IncDecOp, pre_value: Value, post_value: Value) -> Value {
    match inc_dec_op {
        IncDecOp::PreInc | IncDecOp::PreDec => post_value,
        IncDecOp::PostInc | IncDecOp::PostDec => pre_value,
        _ => unreachable!(),
    }
}

fn clean_member_key<'a>(key: &MemberKey<'a>) -> MemberKey<'a> {
    match *key {
        MemberKey::EI(..)
        | MemberKey::ET(..)
        | MemberKey::PT(..)
        | MemberKey::QT(..)
        | MemberKey::W => *key,
        MemberKey::EC(_, op) | MemberKey::EL(_, op) => {
            // Clean EL -> EC because once we've scrubbed the input we don't
            // care where the value came from.
            MemberKey::EC(0, op)
        }
        MemberKey::PC(_, op) | MemberKey::PL(_, op) => {
            // Clean EL -> EC because once we've scrubbed the input we don't
            // care where the value came from.
            MemberKey::PC(0, op)
        }
    }
}

fn clean_opcode<'arena>(opcode: &Opcode<'arena>) -> Opcode<'arena> {
    match *opcode {
        Opcode::ClsCnsL(_) => Opcode::ClsCnsL(Local::INVALID),
        Opcode::IsTypeL(_, op) => Opcode::IsTypeL(Local::INVALID, op),
        Opcode::JmpZ(_) => Opcode::JmpZ(Label::INVALID),
        Opcode::JmpNZ(_) => Opcode::JmpNZ(Label::INVALID),
        Opcode::MemoGet(_, _) => Opcode::MemoGet(Label::INVALID, LocalRange::EMPTY),
        Opcode::MemoSet(_) => Opcode::MemoSet(LocalRange::EMPTY),

        // Opcodes with no immediates are already clean.
        Opcode::AKExists
        | Opcode::Add
        | Opcode::AddElemC
        | Opcode::AddNewElemC
        | Opcode::ArrayIdx
        | Opcode::ArrayMarkLegacy
        | Opcode::ArrayUnmarkLegacy
        | Opcode::Await
        | Opcode::BaseH
        | Opcode::BitAnd
        | Opcode::BitNot
        | Opcode::BitOr
        | Opcode::BitXor
        | Opcode::BreakTraceHint
        | Opcode::CGetCUNop
        | Opcode::CGetG
        | Opcode::CastBool
        | Opcode::CastDict
        | Opcode::CastDouble
        | Opcode::CastInt
        | Opcode::CastKeyset
        | Opcode::CastString
        | Opcode::CastVec
        | Opcode::ChainFaults
        | Opcode::CheckClsReifiedGenericMismatch
        | Opcode::CheckClsRGSoft
        | Opcode::CheckThis
        | Opcode::ClassGetC(_)
        | Opcode::ClassGetTS
        | Opcode::ClassHasReifiedGenerics
        | Opcode::ClassName
        | Opcode::Clone
        | Opcode::Cmp
        | Opcode::Concat
        | Opcode::ContCurrent
        | Opcode::ContEnter
        | Opcode::ContGetReturn
        | Opcode::ContKey
        | Opcode::ContRaise
        | Opcode::ContValid
        | Opcode::CreateCont
        | Opcode::CreateSpecialImplicitContext
        | Opcode::DblAsBits
        | Opcode::Dir
        | Opcode::Div
        | Opcode::Dup
        | Opcode::EnumClassLabelName
        | Opcode::Eq
        | Opcode::Eval
        | Opcode::Exit
        | Opcode::False
        | Opcode::File
        | Opcode::FuncCred
        | Opcode::GetClsRGProp
        | Opcode::Gt
        | Opcode::Gte
        | Opcode::HasReifiedParent
        | Opcode::Idx
        | Opcode::Incl
        | Opcode::InclOnce
        | Opcode::InstanceOf
        | Opcode::IsLateBoundCls
        | Opcode::IssetG
        | Opcode::IssetS
        | Opcode::LateBoundCls
        | Opcode::LazyClassFromClass
        | Opcode::LockObj
        | Opcode::Lt
        | Opcode::Lte
        | Opcode::Method
        | Opcode::Mod
        | Opcode::Mul
        | Opcode::NSame
        | Opcode::NativeImpl
        | Opcode::Neq
        | Opcode::NewObj
        | Opcode::NewPair
        | Opcode::Nop
        | Opcode::Not
        | Opcode::Null
        | Opcode::NullUninit
        | Opcode::ParentCls
        | Opcode::PopC
        | Opcode::PopU
        | Opcode::PopU2
        | Opcode::Pow
        | Opcode::Print
        | Opcode::RaiseClassStringConversionNotice
        | Opcode::RecordReifiedGeneric
        | Opcode::Req
        | Opcode::ReqDoc
        | Opcode::ReqOnce
        | Opcode::RetC
        | Opcode::RetCSuspended
        | Opcode::Same
        | Opcode::Select
        | Opcode::SelfCls
        | Opcode::SetG
        | Opcode::SetImplicitContextByValue
        | Opcode::Shl
        | Opcode::Shr
        | Opcode::Sub
        | Opcode::This
        | Opcode::Throw
        | Opcode::ThrowNonExhaustiveSwitch
        | Opcode::True
        | Opcode::UGetCUNop
        | Opcode::UnsetG
        | Opcode::VerifyImplicitContextState
        | Opcode::VerifyRetNonNullC
        | Opcode::VerifyRetTypeC
        | Opcode::VerifyRetTypeTS
        | Opcode::WHResult
        | Opcode::Yield
        | Opcode::YieldK => opcode.clone(),

        Opcode::CGetL(_) => Opcode::CGetL(Local::INVALID),
        Opcode::CGetL2(_) => Opcode::CGetL2(Local::INVALID),
        Opcode::CGetQuietL(_) => Opcode::CGetQuietL(Local::INVALID),
        Opcode::CUGetL(_) => Opcode::CUGetL(Local::INVALID),
        Opcode::GetMemoKeyL(_) => Opcode::GetMemoKeyL(Local::INVALID),
        Opcode::IsUnsetL(_) => Opcode::IsUnsetL(Local::INVALID),
        Opcode::IssetL(_) => Opcode::IssetL(Local::INVALID),
        Opcode::PopL(_) => Opcode::PopL(Local::INVALID),
        Opcode::PushL(_) => Opcode::PushL(Local::INVALID),
        Opcode::SetL(_) => Opcode::SetL(Local::INVALID),
        Opcode::UnsetL(_) => Opcode::UnsetL(Local::INVALID),

        Opcode::AwaitAll(_) => Opcode::AwaitAll(LocalRange::EMPTY),
        Opcode::Enter(_) => Opcode::Enter(Label::INVALID),
        Opcode::Jmp(_) => Opcode::Jmp(Label::INVALID),
        Opcode::MemoSetEager(_) => Opcode::MemoSetEager(LocalRange::EMPTY),

        Opcode::BareThis(_)
        | Opcode::CGetS(_)
        | Opcode::CheckProp(_)
        | Opcode::ClsCns(_)
        | Opcode::CnsE(_)
        | Opcode::ColFromArray(_)
        | Opcode::CombineAndResolveTypeStruct(_)
        | Opcode::ConcatN(_)
        | Opcode::ContCheck(_)
        | Opcode::Double(_)
        | Opcode::EnumClassLabel(_)
        | Opcode::FCallFunc(_)
        | Opcode::Fatal(_)
        | Opcode::IncDecG(_)
        | Opcode::IncDecS(_)
        | Opcode::InstanceOfD(_)
        | Opcode::Int(_)
        | Opcode::IsTypeC(_)
        | Opcode::IsTypeStructC(_, _)
        | Opcode::ThrowAsTypeStructException(_)
        | Opcode::IterFree(_)
        | Opcode::LazyClass(_)
        | Opcode::NewCol(_)
        | Opcode::NewDictArray(_)
        | Opcode::NewKeysetArray(_)
        | Opcode::NewObjD(_)
        | Opcode::NewObjS(_)
        | Opcode::NewStructDict(_)
        | Opcode::NewVec(_)
        | Opcode::OODeclExists(_)
        | Opcode::ResolveClass(_)
        | Opcode::ResolveClsMethod(_)
        | Opcode::ResolveFunc(_)
        | Opcode::ResolveMethCaller(_)
        | Opcode::ResolveRClsMethod(_)
        | Opcode::ResolveRFunc(_)
        | Opcode::RetM(_)
        | Opcode::SetOpG(_)
        | Opcode::SetOpS(_)
        | Opcode::SetS(_)
        | Opcode::String(_)
        | Opcode::VerifyOutType(_)
        | Opcode::VerifyParamType(_)
        | Opcode::VerifyParamTypeTS(_) => opcode.clone(),

        Opcode::Dict(_) => Opcode::Dict(AdataId::INVALID),
        Opcode::Keyset(_) => Opcode::Keyset(AdataId::INVALID),
        Opcode::Vec(_) => Opcode::Vec(AdataId::INVALID),

        Opcode::MemoGetEager(_, dummy, _) => {
            Opcode::MemoGetEager([Label::INVALID, Label::INVALID], dummy, LocalRange::EMPTY)
        }
        Opcode::Silence(_, op) => Opcode::Silence(Local::INVALID, op),

        Opcode::BaseC(_, m_op_mode) => Opcode::BaseC(0, m_op_mode),
        Opcode::BaseGC(_, m_op_mode) | Opcode::BaseGL(_, m_op_mode) => {
            // BaseGC and BaseGL are equal if the base ends up with the same
            // value.
            Opcode::BaseGL(Local::INVALID, m_op_mode)
        }
        Opcode::BaseL(_, m_op_mode, ro_op) => Opcode::BaseL(Local::INVALID, m_op_mode, ro_op),
        Opcode::BaseSC(_, _, m_op_mode, ro_op) => Opcode::BaseSC(0, 0, m_op_mode, ro_op),

        Opcode::Dim(m_op_mode, ref key) => Opcode::Dim(m_op_mode, clean_member_key(key)),

        Opcode::IncDecM(_, inc_dec_op, ref key) => {
            Opcode::IncDecM(0, inc_dec_op, clean_member_key(key))
        }
        Opcode::QueryM(_, query_m_op, ref key) => {
            Opcode::QueryM(0, query_m_op, clean_member_key(key))
        }
        Opcode::SetM(_, ref key) => Opcode::SetM(0, clean_member_key(key)),
        Opcode::SetOpM(_, set_op_op, ref key) => {
            Opcode::SetOpM(0, set_op_op, clean_member_key(key))
        }
        Opcode::UnsetM(_, ref key) => Opcode::UnsetM(0, clean_member_key(key)),

        Opcode::AssertRATL(_, _)
        | Opcode::AssertRATStk(_, _)
        | Opcode::ClsCnsD(_, _)
        | Opcode::CreateCl(_, _)
        | Opcode::FCallClsMethod(_, _, _)
        | Opcode::FCallClsMethodD(_, _, _)
        | Opcode::FCallClsMethodM(_, _, _, _)
        | Opcode::FCallClsMethodS(_, _, _)
        | Opcode::FCallClsMethodSD(_, _, _, _)
        | Opcode::FCallCtor(_, _)
        | Opcode::FCallFuncD(_, _)
        | Opcode::FCallObjMethod(_, _, _)
        | Opcode::FCallObjMethodD(_, _, _, _)
        | Opcode::IncDecL(_, _)
        | Opcode::InitProp(_, _)
        | Opcode::IterInit(_, _)
        | Opcode::IterNext(_, _)
        | Opcode::LIterFree(_, _)
        | Opcode::LIterInit(_, _, _)
        | Opcode::LIterNext(_, _, _)
        | Opcode::ResolveClsMethodD(_, _)
        | Opcode::ResolveClsMethodS(_, _)
        | Opcode::ResolveRClsMethodD(_, _)
        | Opcode::ResolveRClsMethodS(_, _)
        | Opcode::SetOpL(_, _)
        | Opcode::SetRangeM(_, _, _)
        | Opcode::Switch(_, _, _)
        | Opcode::SSwitch { .. } => {
            debug_assert!(
                LocalInfo::for_opcode(opcode).is_none(),
                "opcode: {:?}",
                opcode
            );
            debug_assert!(opcode.targets().is_empty(), "opcode: {:?}", opcode);
            opcode.clone()
        }
    }
}
