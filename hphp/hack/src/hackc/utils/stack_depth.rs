// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::VecDeque;

use ffi::Just;
use hhbc::DefaultValue;
use hhbc::Instruct;
use hhbc::Label;
use hhbc::Opcode;
use hhbc::Param;
use hhbc::Pseudo;
use hhbc::Targets;
use hhbc_gen::InstrFlags;
use hhbc_gen::OpcodeData;
use hhbc_gen::Outputs;
use log::debug;
use newtype::newtype_int;
use newtype::IdVec;
use thiserror::Error;

pub type Result<T, E = Error> = std::result::Result<T, E>;

/// Given an hhbc::Body compute the maximum stack needed when executing that
/// body. In general dead code will not be tracked however all catch blocks are
/// assumed to start out live.
pub fn compute_stack_depth(params: &[Param<'_>], body_instrs: &[Instruct<'_>]) -> Result<usize> {
    let mut csd = ComputeStackDepth {
        body_instrs,
        labels: Default::default(),
        work: Default::default(),
        handled: Default::default(),
        max_depth: 0,
        cur_depth: 0,
    };
    csd.run(params, body_instrs)?;
    Ok(csd.max_depth as usize)
}

#[derive(Error, Debug)]
pub enum Error {
    #[error("back edge stack depth mismatch")]
    MismatchDepth,
    #[error("stack underflow")]
    StackUnderflow,
    #[error("stack should be 0 after RetC or RetM")]
    UnexpectedExitWithStack,
    #[error("stack should be 1 after resume")]
    UnexpectedStackAfterResume,
    #[error("unexpected end of function")]
    UnexpectedFunctionEnd,
    #[error("unknown label")]
    UnknownLabel,
}

// An Addr is a newtype wrapper for the index into a Body's Instruct Vec.
newtype_int!(Addr, u32, AddrMap, AddrSet);

impl Addr {
    fn next(&mut self) {
        self.0 += 1;
    }
}

struct PendingWork {
    addr: Addr,
    depth: u32,
}

impl PendingWork {
    fn new(addr: Addr, depth: u32) -> Self {
        PendingWork { addr, depth }
    }
}

struct ComputeStackDepth<'a> {
    body_instrs: &'a [Instruct<'a>],
    labels: IdVec<Label, Addr>,
    work: VecDeque<PendingWork>,
    /// Mapping from handled label addresses to their depth at that
    /// location. This is a sparse map with entries only at label Addrs.
    handled: AddrMap<u32>,
    max_depth: u32,
    cur_depth: u32,
}

impl ComputeStackDepth<'_> {
    fn run(&mut self, params: &[Param<'_>], body_instrs: &[Instruct<'_>]) -> Result<()> {
        debug!("ComputeStackDepth::run");
        self.precompute(body_instrs);

        // The "normal" entrypoint.
        self.work.push_back(PendingWork::new(Addr(0), 0));

        // A depth-0 entrypoint for each default value.
        for param in params {
            if let Just(DefaultValue { label, .. }) = param.default_value.as_ref() {
                let addr = self.labels[*label];
                self.work.push_back(PendingWork::new(addr, 0));
            }
        }

        while let Some(PendingWork { addr, depth }) = self.work.pop_front() {
            self.process_block(addr, depth)?;
        }

        Ok(())
    }

    /// Process a block of execution.
    fn process_block(&mut self, mut addr: Addr, depth: u32) -> Result<()> {
        debug!("  -- block at {addr:?}, depth {depth}");
        self.cur_depth = depth;
        loop {
            if let Some(instr) = self.body_instrs.get(addr.0 as usize) {
                if !matches!(instr, Instruct::Pseudo(Pseudo::SrcLoc(..))) {
                    debug!(
                        "    {addr:?}, {depth}: instr {instr:?}",
                        depth = self.cur_depth
                    );
                }
                let finished = self.process_instruct(addr, instr)?;
                addr.next();
                if finished {
                    break;
                }
            } else {
                // We fell off the end of the function - this isn't allowed!
                return Err(Error::UnexpectedFunctionEnd);
            }
        }
        Ok(())
    }

    /// Push `n` values onto the stack.
    fn push_n(&mut self, n: u32) {
        self.cur_depth += n;
        self.max_depth = std::cmp::max(self.max_depth, self.cur_depth);
    }

    /// Pop `n` values off the stack.
    fn pop_n(&mut self, n: u32) -> Result<()> {
        if n > self.cur_depth {
            return Err(Error::StackUnderflow);
        }
        self.cur_depth -= n;
        Ok(())
    }

    /// Register a control-flow jump to a new address. This doesn't
    /// (automatically) termintate the current block of execution.
    fn jmp(&mut self, target: Addr) -> Result<()> {
        if let Some(expected_depth) = self.handled.get(&target) {
            // We already handled this target - it had better have the same
            // depth as we saw last time.
            if self.cur_depth != *expected_depth {
                return Err(Error::MismatchDepth);
            }
        } else {
            self.work
                .push_back(PendingWork::new(target, self.cur_depth));
        }
        Ok(())
    }

    /// Register a control-flow jump to a new label. This doesn't
    /// (automatically) terminate the current block of execution.
    fn jmp_label(&mut self, target: Label) -> Result<()> {
        let target = self.labels.get(target).ok_or(Error::UnknownLabel)?;
        self.jmp(*target)
    }

    /// Process an Instruct. Returns `true` if this Instruct ends the current block.
    fn process_instruct(&mut self, addr: Addr, instr: &Instruct<'_>) -> Result<bool> {
        let end_block = match instr {
            Instruct::Pseudo(
                Pseudo::Break
                | Pseudo::Continue
                | Pseudo::SrcLoc(..)
                | Pseudo::TryCatchBegin
                | Pseudo::TryCatchEnd
                | Pseudo::TryCatchMiddle,
            ) => false,
            Instruct::Pseudo(Pseudo::Label(_)) => {
                if let Some(old_depth) = self.handled.insert(addr, self.cur_depth) {
                    if old_depth != self.cur_depth {
                        return Err(Error::MismatchDepth);
                    }
                    // We've already handled this label - stop processing this
                    // block.
                    true
                } else {
                    false
                }
            }
            Instruct::Opcode(Opcode::MemoGet(target, _)) => {
                // MemoGet jumps to its target BEFORE pushing a value.
                self.jmp_label(*target)?;
                self.push_n(1);
                false
            }
            Instruct::Opcode(Opcode::MemoGetEager([target0, target1], _, _)) => {
                // MemoGetEager jumps to its first target BEFORE pushing a value.
                self.jmp_label(*target0)?;
                self.push_n(1);
                // ... but jumps to its second AFTER pushing a value.
                self.jmp_label(*target1)?;
                false
            }
            Instruct::Opcode(opcode) => {
                // "standard" opcode handling
                let opcode_data = lookup_data_for_opcode(opcode);

                // Pop inputs.
                let n = opcode.num_inputs();
                self.pop_n(n as u32)?;

                // Push outputs.
                let n = num_outputs(opcode, opcode_data);
                self.push_n(n);

                if opcode_data.flags.contains(InstrFlags::CF) {
                    // If the Instruct has a control flow make sure to jump to
                    // the targets.
                    let targets = opcode.targets();
                    for target in targets {
                        self.jmp_label(*target)?;
                    }
                }

                match opcode {
                    Opcode::RetC | Opcode::RetCSuspended | Opcode::RetM(_) | Opcode::NativeImpl => {
                        // Stack depth should be 0 after RetC or RetM.
                        if self.cur_depth != 0 {
                            return Err(Error::UnexpectedExitWithStack);
                        }
                    }
                    Opcode::CreateCont | Opcode::Await | Opcode::Yield | Opcode::YieldK => {
                        // Stack depth should be 1 after resume from suspend.
                        if self.cur_depth != 1 {
                            return Err(Error::UnexpectedStackAfterResume);
                        }
                    }
                    _ => {}
                }

                // If the Instruct is a terminator make sure to end this block
                // here.
                opcode_data.flags.contains(InstrFlags::TF)
            }
        };
        Ok(end_block)
    }

    /// Precompute label addresses and catch addresses.
    fn precompute(&mut self, body_instrs: &[Instruct<'_>]) {
        for (idx, instr) in body_instrs.iter().enumerate() {
            let addr = Addr(idx as u32);

            match instr {
                Instruct::Pseudo(Pseudo::Label(label)) => {
                    if self.labels.len() <= label.as_usize() {
                        self.labels.resize(label.as_usize() + 1, Addr(0));
                    }
                    self.labels[*label] = addr;
                }
                Instruct::Pseudo(Pseudo::TryCatchMiddle) => {
                    // Add the catch block as a pending action.
                    // Catch frames always start with just the thrown object on the stack.
                    self.work.push_back(PendingWork::new(addr, 1));
                }
                _ => {}
            }
        }
    }
}

fn num_outputs(opcode: &Opcode<'_>, opcode_data: &OpcodeData) -> u32 {
    match &opcode_data.outputs {
        Outputs::NOV => 0,
        Outputs::Fixed(n) => n.len() as u32,
        Outputs::FCall => match opcode {
            Opcode::FCallClsMethod(fca, _, _)
            | Opcode::FCallClsMethodM(fca, _, _, _)
            | Opcode::FCallClsMethodD(fca, _, _)
            | Opcode::FCallClsMethodS(fca, _, _)
            | Opcode::FCallClsMethodSD(fca, _, _, _)
            | Opcode::FCallCtor(fca, _)
            | Opcode::FCallFunc(fca)
            | Opcode::FCallFuncD(fca, _)
            | Opcode::FCallObjMethod(fca, _, _)
            | Opcode::FCallObjMethodD(fca, _, _, _) => fca.num_rets,
            _ => unreachable!(),
        },
    }
}

fn lookup_data_for_opcode(opcode: &Opcode<'_>) -> &'static OpcodeData {
    hhbc_gen::opcode_data().get(opcode.variant_index()).unwrap()
}
