// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use bit_set::BitSet;
use bit_vec::BitVec;
use env::emitter::Emitter;
use hash::HashMap;
use hhbc::Flow;
use hhbc::Instruct;
use hhbc::Label;
use hhbc::Local;
use hhbc::Locals;
use hhbc::MOpMode;
use hhbc::MemberKey;
use hhbc::Opcode;
use hhbc::Pseudo;
use hhbc::ReadonlyOp;
use hhbc::StringId;
use hhbc::Targets;
use instruction_sequence::InstrSeq;
use priority_queue::PriorityQueue;
use smallvec::smallvec;
use smallvec::SmallVec;

struct Block<'b> {
    id: usize,
    rpo_id: Option<i32>,
    instrs: &'b [Instruct],
    catch: Option<usize>,
    preds: Vec<usize>,
    succs: Vec<usize>,
}

impl<'b> Block<'b> {
    fn new(id: usize, instrs: &'b [Instruct]) -> Self {
        Self {
            id,
            rpo_id: None,
            instrs,
            catch: None,
            preds: Vec::new(),
            succs: Vec::new(),
        }
    }

    fn is_catch(&self) -> bool {
        match self.instrs.first() {
            Some(Instruct::Pseudo(Pseudo::TryCatchMiddle)) => true,
            _ => false,
        }
    }
}

type LocalVec = SmallVec<[Local; 3]>;

/// LocalAnalysis for an instruction indicates which actions the instruction
/// performs on locals. The may_read set is an upper bound on the locations
/// read, while all other fields represent lower bounds.
struct LocalAnalysis {
    must_write: LocalVec,
    must_unset: LocalVec,
    may_read: LocalVec,
    must_read: LocalVec,
}

impl Default for LocalAnalysis {
    fn default() -> LocalAnalysis {
        LocalAnalysis {
            must_write: LocalVec::new(),
            must_unset: LocalVec::new(),
            may_read: LocalVec::new(),
            must_read: LocalVec::new(),
        }
    }
}

fn analyze_locals(inst: &Instruct) -> LocalAnalysis {
    match &inst {
        Instruct::Pseudo(_) => LocalAnalysis::default(),
        Instruct::Opcode(Opcode::PushL(loc)) => LocalAnalysis {
            must_unset: smallvec![*loc],
            may_read: smallvec![*loc],
            ..Default::default()
        },
        Instruct::Opcode(Opcode::PopL(loc) | Opcode::SetL(loc)) => LocalAnalysis {
            must_write: smallvec![*loc],
            ..Default::default()
        },
        Instruct::Opcode(Opcode::UnsetL(loc)) => LocalAnalysis {
            must_unset: smallvec![*loc],
            ..Default::default()
        },
        Instruct::Opcode(
            Opcode::CGetL(loc)
            | Opcode::CGetL2(loc)
            | Opcode::IsTypeL(loc, _)
            | Opcode::GetMemoKeyL(loc)
            | Opcode::IncDecL(loc, _),
        ) => LocalAnalysis {
            may_read: smallvec![*loc],
            must_read: smallvec![*loc],
            ..Default::default()
        },
        Instruct::Opcode(Opcode::BaseL(loc, mode, _ro)) if *mode == MOpMode::Warn => {
            LocalAnalysis {
                may_read: smallvec![*loc],
                must_read: smallvec![*loc],
                ..Default::default()
            }
        }
        Instruct::Opcode(
            Opcode::Dim(_, MemberKey::EL(loc, _) | MemberKey::PL(loc, _))
            | Opcode::QueryM(_, _, MemberKey::EL(loc, _) | MemberKey::PL(loc, _))
            | Opcode::SetM(_, MemberKey::EL(loc, _) | MemberKey::PL(loc, _))
            | Opcode::IncDecM(_, _, MemberKey::EL(loc, _) | MemberKey::PL(loc, _))
            | Opcode::SetOpM(_, _, MemberKey::EL(loc, _) | MemberKey::PL(loc, _))
            | Opcode::UnsetM(_, MemberKey::EL(loc, _) | MemberKey::PL(loc, _)),
        ) => LocalAnalysis {
            may_read: smallvec![*loc],
            must_read: smallvec![*loc],
            ..Default::default()
        },
        Instruct::Opcode(op @ Opcode::SetRangeM(..)) => {
            let locals: LocalVec = op.locals().into_iter().filter(|l| l.is_valid()).collect();
            LocalAnalysis {
                may_read: locals.clone(),
                must_read: locals,
                ..Default::default()
            }
        }

        // The default conservative analysis is to put all referenced locals
        // into the may_read set, and leave all of the must_* sets empty.
        Instruct::Opcode(op) => LocalAnalysis {
            may_read: op.locals().into_iter().filter(|l| l.is_valid()).collect(),
            ..Default::default()
        },
    }
}

fn instr_may_throw(inst: &Instruct) -> bool {
    match &inst {
        Instruct::Opcode(
            Opcode::PopL(_)
            | Opcode::PushL(_)
            | Opcode::SetL(_)
            | Opcode::UnsetL(_)
            | Opcode::IssetL(_)
            | Opcode::IsUnsetL(_)
            | Opcode::Int(_)
            | Opcode::Double(_)
            | Opcode::String(_)
            | Opcode::Dict(_)
            | Opcode::Vec(_)
            | Opcode::Keyset(_)
            | Opcode::NewDictArray(_)
            | Opcode::NewStructDict(_)
            | Opcode::NewVec(_)
            | Opcode::NewCol(_)
            | Opcode::LazyClass(_)
            | Opcode::EnumClassLabel(_)
            | Opcode::CreateCl(..)
            | Opcode::CUGetL(_)
            | Opcode::CGetQuietL(_)
            | Opcode::CheckProp(_)
            | Opcode::NewPair
            | Opcode::File
            | Opcode::Dir
            | Opcode::FuncCred
            | Opcode::DblAsBits
            | Opcode::PopC
            | Opcode::PopU
            | Opcode::PopU2
            | Opcode::CGetCUNop
            | Opcode::UGetCUNop
            | Opcode::Nop
            | Opcode::Dup
            | Opcode::Null
            | Opcode::NullUninit
            | Opcode::True
            | Opcode::False,
        ) => false,
        Instruct::Pseudo(_) => false,
        _ => true,
    }
}

/// Update anticipated locals, consider each instruction I, along with
///     may_read(I) -> locals which may be read by I
///     must_unset(I) -> locals that must be unset by I
///     must_write(I) -> locals that must be written by I,
///     ant_catch(I) -> the anticipated locals at the catch block for I, or the
///                     empty set if there is no catch for I or I does not throw
///
/// ant = ant & ~(must_write(I) | must_unset(I)) | may_read(I) | ant_catch(I)
fn update_ant(inst: &Instruct, ant: &mut BitSet, catch_ant: Option<&BitSet>) {
    let analysis = analyze_locals(inst);
    analysis
        .must_write
        .iter()
        .chain(analysis.must_unset.iter())
        .for_each(|loc| {
            ant.remove(loc.idx as usize);
        });
    analysis.may_read.iter().for_each(|loc| {
        ant.insert(loc.idx as usize);
    });

    match &catch_ant {
        Some(s) if instr_may_throw(inst) => ant.union_with(s),
        _ => (),
    }
}

/// Update the available locals, consider each instruction I, along with
///     must_read(I) -> locals which must be read by I
///     must_unset(I) -> locals that must be unset by I
///     must_write(I) -> locals that must be written by I,
///     avl_catch -> the available locals at the catch block following a branch
///                      from I and all preceding instructions which may throw
///
/// Note that avl_catch is performed prior to updating avl as any throw from I
/// is presumed to happen prior to writes performed by I occurring.
///
/// avl_catch = avl_catch & avl
/// avl = (avl | must_write(I) | must_read(I)) & ~must_unset(I)
fn update_avl(inst: &Instruct, avl: &mut BitSet, catch_avl: &mut Option<&mut BitSet>) {
    match catch_avl {
        Some(ref mut s) if instr_may_throw(inst) => s.intersect_with(avl),
        _ => (),
    }
    let analysis = analyze_locals(inst);
    analysis
        .must_write
        .iter()
        .chain(analysis.must_read.iter())
        .for_each(|loc| {
            avl.insert(loc.idx as usize);
        });
    analysis.must_unset.iter().for_each(|loc| {
        avl.remove(loc.idx as usize);
    });
}

struct BlockData {
    /// Locals known to be available (not unset) at the beginning of the block
    avl_in: BitSet,

    /// Locals known to be available at the end of the block
    avl_out: BitSet,

    /// Locals with pending (anticipated) reads at the beginning of the block
    ant_in: BitSet,

    /// Locals with pending reads at the end of the block
    ant_out: BitSet,

    /// Locals known to be available at every point in this block that can raise
    /// an exception (None if this block has no catch associated)
    avl_catch: Option<BitSet>,
}

struct BlockAnalysis {
    avl: Vec<BitSet>,
    ant: Vec<BitSet>,
}

impl BlockData {
    fn new(nlocals: usize, has_catch: bool) -> Self {
        let avl_catch = if has_catch {
            Some(BitSet::with_capacity(nlocals))
        } else {
            None
        };
        Self {
            avl_in: BitSet::with_capacity(nlocals),
            avl_out: BitSet::with_capacity(nlocals),
            ant_in: BitSet::with_capacity(nlocals),
            ant_out: BitSet::with_capacity(nlocals),
            avl_catch,
        }
    }

    fn new_entry(nlocals: usize, nparams: usize, has_catch: bool) -> Self {
        let mut avl_in = BitSet::with_capacity(nlocals);
        avl_in.union_with(&BitSet::from_bit_vec(BitVec::from_elem(nparams, true)));
        let avl_catch = if has_catch {
            Some(BitSet::with_capacity(nlocals))
        } else {
            None
        };
        Self {
            avl_in,
            avl_out: BitSet::with_capacity(nlocals),
            ant_in: BitSet::with_capacity(nlocals),
            ant_out: BitSet::with_capacity(nlocals),
            avl_catch,
        }
    }

    /// Iterate block calling update_avl with each instruction using avl_in as
    /// the initial state and record avl_out and avl_catch following the final
    /// instruction:
    ///
    /// avl_out = avl_in
    /// avl_catch = avl_in
    /// For I in block:
    ///   avl_catch = avl_catch & avl_out
    ///   avl_out = (avl_out | must_write(I) | must_read(I)) & ~must_unset(I)
    fn update_avl(&self, block: &Block<'_>, first: bool) -> (Option<BitSet>, Option<BitSet>) {
        // new_avl: the avl bits exiting this block (avl_out)
        let mut new_avl = self.avl_in.clone();

        // new_avl_catch: the intersection of avl bits on every exception edge
        // from this block to the catch block
        let mut new_avl_catch = match block.catch {
            Some(_) => Some(self.avl_in.clone()),
            _ => None,
        };

        block.instrs.iter().for_each(|inst| {
            update_avl(inst, &mut new_avl, &mut new_avl_catch.as_mut());
        });
        let avl_out =
            if !first && self.avl_out.is_subset(&new_avl) && new_avl.is_subset(&self.avl_out) {
                None
            } else {
                Some(new_avl)
            };
        let avl_catch = match (new_avl_catch, &self.avl_catch) {
            (Some(new), Some(old)) if first || !old.is_subset(&new) || !new.is_subset(old) => {
                Some(new)
            }
            _ => None,
        };
        (avl_out, avl_catch)
    }

    /// Reverse iterate block calling update_ant with each instruction using
    /// ant_out as an initial state and updating ant_in following the final
    /// instruction:
    ///
    /// ant_in = ant_out
    /// For I in reverse(block):
    ///   ant_in = ant_in & ~(must_write(I) | must_unset(I)) | may_read(I) | ant_catch(I)
    fn update_ant(&self, block: &Block<'_>, ant_catch: Option<&BitSet>) -> Option<BitSet> {
        let mut new_ant = self.ant_out.clone();
        block.instrs.iter().rev().for_each(|inst| {
            update_ant(inst, &mut new_ant, ant_catch);
        });
        if self.ant_in.is_subset(&new_ant) && new_ant.is_subset(&self.ant_in) {
            None
        } else {
            Some(new_ant)
        }
    }

    /// For each instruction I in the block construct avl[I] and ant[I], which
    /// record the current set of anticipated and available locals at that
    /// instruction:
    ///
    /// avl[First(block)] = avl_in
    /// ant[Last(block)] = ant_out
    /// avl[Succ(I)] = (avl[I] | must_write(I) | must_read(I)) & ~must_unset(I)
    /// ant[Pred(I)] = ant[I] & ~(must_write(I) | must_unset(I)) | may_read(I) | ant_catch(I)
    fn analyze_block(&self, block: &Block<'_>, catch_ant: Option<&BitSet>) -> BlockAnalysis {
        let mut avl_bits = self.avl_in.clone();
        let mut ant_bits = self.ant_out.clone();
        let avl = block
            .instrs
            .iter()
            .map(|inst| {
                let prev = avl_bits.clone();
                update_avl(inst, &mut avl_bits, &mut None);
                prev
            })
            .collect();
        let ant: Vec<BitSet> = block
            .instrs
            .iter()
            .rev()
            .map(|inst| {
                let prev = ant_bits.clone();
                update_ant(inst, &mut ant_bits, catch_ant);
                prev
            })
            .collect();
        BlockAnalysis {
            avl,
            ant: ant.into_iter().rev().collect(),
        }
    }
}

fn max_local(instrs: &[Instruct]) -> usize {
    instrs
        .iter()
        .map(|i| match &i {
            Instruct::Opcode(op) => op
                .locals()
                .iter()
                .map(|loc| if loc.is_valid() { loc.idx } else { 0 })
                .max()
                .unwrap_or(0),
            _ => 0,
        })
        .max()
        .unwrap_or(0) as usize
}

/// make_cfg converts a list of instructions into a Vec of basic blocks with
/// reverse post-order ID numbers. The ordering of the blocks in the returned
/// Vec preserves the program ordering prior to this partitioning.
///
/// Basic blocks are broken at terminal instructions, instructions inducing
/// control flow, labels, and the special TryCatch pseudo-instructions which
/// mark regions covered by catch blocks and the catch blocks which they branch
/// to for exception handling.
fn make_cfg<'b>(instrs: &'b [Instruct]) -> Vec<Block<'b>> {
    let mut blocks = Vec::new();
    let mut label_map: HashMap<Label, usize> = HashMap::default();
    let mut start = 0;

    for (idx, inst) in instrs.iter().enumerate() {
        match inst {
            Instruct::Opcode(op) if op.is_flow() || op.is_terminal() => {
                blocks.push(Block::new(blocks.len(), &instrs[start..idx + 1]));
                start = idx + 1;
            }
            Instruct::Pseudo(
                Pseudo::Label(_)
                | Pseudo::TryCatchBegin
                | Pseudo::TryCatchMiddle
                | Pseudo::TryCatchEnd,
            ) if start != idx => {
                blocks.push(Block::new(blocks.len(), &instrs[start..idx]));
                start = idx;
            }
            Instruct::Pseudo(Pseudo::Continue | Pseudo::Break) => {
                panic!("Must run try/finally rewriter before analyzing flow")
            }
            _ => {}
        }

        if let Instruct::Pseudo(Pseudo::Label(l)) = inst {
            label_map.insert(*l, blocks.len());
        }
    }

    if blocks.is_empty() {
        return blocks;
    }

    let mut post_catch_map: HashMap<usize, usize> = HashMap::default();
    let mut catches = Vec::new();
    let mut last_catch = Vec::new();
    for idx in 0..blocks.len() {
        match blocks[idx].instrs.first() {
            Some(Instruct::Pseudo(Pseudo::TryCatchBegin)) => {
                let mut nest = 0;
                let catch = blocks[idx + 1..]
                    .iter()
                    .find(|blk| match &blk.instrs.first() {
                        Some(Instruct::Pseudo(Pseudo::TryCatchBegin)) => {
                            nest += 1;
                            false
                        }
                        Some(Instruct::Pseudo(Pseudo::TryCatchMiddle)) if nest == 0 => true,
                        Some(Instruct::Pseudo(Pseudo::TryCatchEnd)) => {
                            nest -= 1;
                            false
                        }
                        _ => false,
                    });
                catches.push(catch.unwrap().id);
            }
            Some(Instruct::Pseudo(Pseudo::TryCatchMiddle)) => {
                catches.pop();
                last_catch.push(idx);
            }
            Some(Instruct::Pseudo(Pseudo::TryCatchEnd)) => {
                post_catch_map.insert(last_catch.pop().unwrap(), idx);
            }
            _ => (),
        }
        if blocks[idx].instrs.iter().any(instr_may_throw) {
            if let Some(c) = catches.last() {
                blocks[idx].catch = Some(*c);
            }
        }
    }

    let mut stack: Vec<usize> = vec![0];
    let mut visited = BitSet::with_capacity(blocks.len());
    let mut rpo_id = blocks.len() as i32;
    while let Some(cur) = stack.last().copied() {
        if visited.insert(cur) {
            let fallthrough = match &blocks[cur].catch {
                Some(c) if *c == cur + 1 => post_catch_map[c],
                _ => cur + 1,
            };
            let mut enqueue = |idx| {
                if !visited.contains(idx) {
                    stack.push(idx);
                }
            };
            match &blocks[cur].instrs.last() {
                Some(Instruct::Opcode(op)) => {
                    if !op.is_terminal() && fallthrough < blocks.len() {
                        enqueue(fallthrough);
                        blocks[fallthrough].preds.push(cur);
                        blocks[cur].succs.push(fallthrough);
                    }
                    if op.is_flow() {
                        for target in op.targets() {
                            let idx = label_map[target];
                            enqueue(idx);
                            blocks[idx].preds.push(cur);
                            blocks[cur].succs.push(idx);
                        }
                    }
                }
                _ => {
                    if fallthrough < blocks.len() && !visited.contains(fallthrough) {
                        enqueue(fallthrough);
                        blocks[fallthrough].preds.push(cur);
                        blocks[cur].succs.push(fallthrough);
                    }
                }
            }
            if let Some(c) = blocks[cur].catch {
                enqueue(c);
                blocks[c].preds.push(cur);
            }
        } else {
            stack.pop();
            if blocks[cur].rpo_id.is_none() {
                blocks[cur].rpo_id = Some(rpo_id);
                rpo_id -= 1;
            }
        }
    }

    blocks
}

/// Perform a reverse dataflow analysis computing the ant_in and ant_out states
/// for each block in the CFG (See update_ant for dataflow equations)
fn compute_ant<'b>(blocks: &[Block<'b>], data: &mut [BlockData]) {
    let mut pq = PriorityQueue::new();
    let mut in_q = BitSet::with_capacity(blocks.len());

    for (id, block) in blocks.iter().enumerate() {
        if let Some(rpo_id) = block.rpo_id {
            pq.push(id, rpo_id);
            in_q.insert(id);
        }
    }

    while let Some((id, _rpo_id)) = pq.pop() {
        in_q.remove(id);

        let ant_catch = blocks[id].catch.as_ref().map(|c| &data[*c].ant_in);

        if let Some(ant_in) = data[id].update_ant(&blocks[id], ant_catch) {
            for pred in blocks[id].preds.iter().copied() {
                if blocks[id].is_catch() {
                    if in_q.insert(pred) {
                        pq.push(pred, blocks[pred].rpo_id.unwrap());
                    }
                } else if !ant_in.is_subset(&data[pred].ant_out) {
                    data[pred].ant_out.union_with(&ant_in);
                    if in_q.insert(pred) {
                        pq.push(pred, blocks[pred].rpo_id.unwrap());
                    }
                }
            }
            data[id].ant_in = ant_in;
        }
    }
}

/// Perform a forward dataflow analysis computing the avl_in and avl_out states
/// for each block in the CFG (See update_avl for dataflow equations)
fn compute_avl<'b>(blocks: &[Block<'b>], data: &mut [BlockData]) {
    let mut pq = PriorityQueue::new();
    let mut in_q = BitSet::with_capacity(blocks.len());
    let mut seen = BitSet::with_capacity(blocks.len());

    for (id, block) in blocks.iter().enumerate() {
        if let Some(rpo_id) = block.rpo_id {
            pq.push(id, -rpo_id);
            in_q.insert(id);
        }
    }

    while let Some((id, _rpo_id)) = pq.pop() {
        in_q.remove(id);
        let (out, catch) = data[id].update_avl(&blocks[id], seen.insert(id));

        let mut update_succ = |mut avl_new: BitSet, succ: usize, data: &mut [BlockData]| {
            for pred in blocks[succ].preds.iter().copied() {
                if pred != id && seen.contains(pred) {
                    let is_catch = blocks[pred].catch == Some(succ);
                    if is_catch {
                        avl_new.intersect_with(data[pred].avl_catch.as_ref().unwrap());
                    }
                    if !is_catch || blocks[pred].succs.iter().any(|b| *b == succ) {
                        avl_new.intersect_with(&data[pred].avl_out);
                    }
                }
            }
            if !avl_new.is_subset(&data[succ].avl_in) || !data[succ].avl_in.is_subset(&avl_new) {
                if in_q.insert(succ) {
                    pq.push(succ, -blocks[succ].rpo_id.unwrap());
                }
                data[succ].avl_in = avl_new;
            }
        };

        if let Some(avl_out) = out {
            for succ in blocks[id].succs.iter().copied() {
                update_succ(avl_out.clone(), succ, data);
            }
            data[id].avl_out = avl_out;
        }

        if let Some(ref avl_catch) = catch {
            let cid = blocks[id].catch.unwrap();
            update_succ(avl_catch.clone(), cid, data);
            data[id].avl_catch = catch;
        }
    }
}

/// make_block_data is an analysis pass which performs forward and backward
/// dataflow over the basic blocks in a CFG to determine the state of locals
/// in the program upon entering and exiting the block.
///
/// Forward dataflow analysis is used to determine which locals must be live
/// (not assigned a value of Uninit). Instructions which assign values to locals
/// as well as those that will throw when accessing uninitialized locals are
/// used to update this state.
///
/// Reverse dataflow analysis is used to determine which locals hold values that
/// may be read from. This is done by marking locals in the may_read sets of
/// observed instructions as anticipated and propagating this information
/// backwards. Writes to locals will clear their anticipated state as they now
/// contain new values.
///
/// In addition to simple control flow, exception edges are also modeled here.
/// We consider any instruction other than those specifically enumerated in
/// instr_may_throw as potentially raising exception. For blocks with fault
/// edges indicating catch blocks:
///   - During forward dataflow analysis state prior to any potentially throwing
///     instruction is propagated into the catch block.
///   - For reverse dataflow state from the catch block is merged with the
///     current state following any instruction which may raise.
fn make_block_data<'b>(nparams: usize, nlocals: usize, blocks: &[Block<'b>]) -> Vec<BlockData> {
    if blocks.is_empty() {
        return Vec::new();
    }

    let mut data = Vec::new();
    data.push(BlockData::new_entry(
        nlocals,
        nparams,
        blocks[0].catch.is_some(),
    ));
    for b in &blocks[1..] {
        data.push(BlockData::new(nlocals, b.catch.is_some()));
    }

    compute_ant(blocks, &mut data);
    compute_avl(blocks, &mut data);
    data
}

fn print_data<'b>(blocks: &[Block<'b>], data: &[BlockData]) {
    if false {
        let mut off: usize = 0;
        for (id, bd) in data.iter().enumerate() {
            let label = match &blocks[id].instrs.first() {
                Some(Instruct::Pseudo(Pseudo::Label(x))) => Some(x),
                _ => None,
            };
            println!(
                "Block #{} (rpo: {:?}) (off: {}) (L: {:?}) (Catch: {:?})",
                id, blocks[id].rpo_id, off, label, blocks[id].catch
            );
            println!("\tpreds: {:?}", &blocks[id].preds);
            println!("\tsuccs: {:?}\n", &blocks[id].succs);
            println!("\tavl_in: {:?}", &bd.avl_in);
            println!("\tavl_out: {:?}\n", &bd.avl_out);
            println!("\tant_in: {:?}", &bd.ant_in);
            println!("\tant_out: {:?}\n", &bd.ant_out);

            for instr in blocks[id].instrs {
                println!("\t| {}", instr.variant_name());
            }
            println!();

            let ict = blocks[id]
                .instrs
                .iter()
                .filter(|i| match &i {
                    Instruct::Opcode(_) => true,
                    _ => false,
                })
                .count();
            off += ict;
        }

        if blocks.len() > 1 {
            let catch_ant = blocks[1].catch.map(|ci| &data[ci].ant_in);
            let ba = data[1].analyze_block(&blocks[1], catch_ant);
            for (i, avl) in ba.avl.iter().enumerate() {
                let ant = &ba.ant[i];
                println!("\t\t1:{} | avl = {:?} ; ant = {:?}", i, avl, ant);
            }
        }
    }
}

fn adj_mkey(mkey: &MemberKey) -> MemberKey {
    match mkey {
        MemberKey::EC(stk, rop) => MemberKey::EC(stk + 1, *rop),
        MemberKey::PC(stk, rop) => MemberKey::PC(stk + 1, *rop),
        _ => *mkey,
    }
}

fn dim_local(instr: &Instruct) -> Option<Local> {
    match &instr {
        Instruct::Opcode(Opcode::Dim(_, MemberKey::EL(loc, _) | MemberKey::PL(loc, _))) => {
            Some(*loc)
        }
        _ => None,
    }
}

fn optimize_locals(
    instr: &Instruct,
    avl: &BitSet,
    ant: &BitSet,
    adj_minstr: &mut bool,
    minstr_locals: &mut Vec<Local>,
) -> Vec<Instruct> {
    if let Some(loc) = dim_local(instr) {
        minstr_locals.push(loc);
    }

    let mut add_unsets = |mut ret: Vec<Instruct>| {
        let la = analyze_locals(instr);
        la.may_read
            .iter()
            .chain(minstr_locals.iter())
            .filter(|l| !ant.contains(l.idx as usize))
            .for_each(|l| {
                ret.push(Instruct::Opcode(Opcode::UnsetL(*l)));
            });
        minstr_locals.clear();
        ret
    };

    match &instr {
        Instruct::Opcode(Opcode::CGetL(loc) | Opcode::CUGetL(loc)) => {
            let idx = loc.idx as usize;
            if avl.contains(idx) && !ant.contains(idx) {
                vec![Instruct::Opcode(Opcode::PushL(*loc))]
            } else {
                add_unsets(vec![instr.clone()])
            }
        }

        Instruct::Opcode(Opcode::BaseL(loc, op, ro)) => {
            let idx = loc.idx as usize;
            match (*op, *ro) {
                (MOpMode::Unset | MOpMode::Define, _) => vec![instr.clone()],
                (_, ReadonlyOp::CheckROCOW | ReadonlyOp::CheckMutROCOW) => vec![instr.clone()],
                (mode, _) => {
                    if avl.contains(idx) && !ant.contains(idx) {
                        *adj_minstr = true;
                        vec![
                            Instruct::Opcode(Opcode::PushL(*loc)),
                            Instruct::Opcode(Opcode::BaseC(0, mode)),
                        ]
                    } else {
                        vec![instr.clone()]
                    }
                }
            }
        }

        Instruct::Opcode(Opcode::Dim(mode, mkey)) if *adj_minstr => {
            vec![Instruct::Opcode(Opcode::Dim(*mode, adj_mkey(mkey)))]
        }

        Instruct::Opcode(Opcode::QueryM(discard, op, mkey)) if *adj_minstr => {
            *adj_minstr = false;
            add_unsets(vec![Instruct::Opcode(Opcode::QueryM(
                discard + 1,
                *op,
                adj_mkey(mkey),
            ))])
        }

        Instruct::Opcode(Opcode::SetL(loc)) if !ant.contains(loc.idx as usize) => {
            vec![]
        }

        Instruct::Opcode(Opcode::PopL(loc)) if !ant.contains(loc.idx as usize) => {
            vec![Instruct::Opcode(Opcode::PopC)]
        }

        Instruct::Opcode(
            Opcode::PushL(_)
            | Opcode::UnsetL(_)
            | Opcode::BaseGC(..)
            | Opcode::BaseGL(..)
            | Opcode::BaseSC(..)
            | Opcode::BaseC(..)
            | Opcode::BaseH
            | Opcode::Dim(..),
        ) => {
            vec![instr.clone()]
        }

        Instruct::Opcode(_) => add_unsets(vec![instr.clone()]),

        _ => {
            vec![instr.clone()]
        }
    }
}

fn mark_volatile_locals(
    emitter: &Emitter<'_, '_>,
    nparams: usize,
    decl_vars: &[StringId],
    nlocals: usize,
) -> BitSet {
    let mut volatile = BitSet::with_capacity(nlocals);
    if !emitter.options().hhbc.optimize_param_lifetimes {
        let params = BitVec::from_elem(nparams, true);
        volatile.union_with(&BitSet::from_bit_vec(params));
    }

    decl_vars
        .iter()
        .enumerate()
        .for_each(|(i, v)| match v.as_str() {
            "$0ReifiedGenerics" | "$0Coeffects" | "$86metadata" | "$86productAttributionData" => {
                volatile.insert(nparams + i);
            }
            _ => (),
        });

    volatile
}

/// optimize_lifetimes pass attempts to reduce the lifetime of locals by either
/// unsetting them (UnsetL) or moving them (PushL). This can be important for
/// copy-on-write types where holding extra references may result in unnecessary
/// copies. The transformations currently supported by this pass are:
///
///   1. Rewriting CGetL instructions to PushL when the local is known to be
///      initialized and the CGetL is the final read of the current value of the
///      local.
///   2. Rewriting BaseL to PushL; BaseC, in addition to the rules from (1) the
///      member operation must also be a QueryM
///   3. Upon seeing any instruction which reads from a local if that
///      instruction is the last such read from a local an UnsetL is inserted
///      following the instruction.
///
/// The (1) and (2) transformations are designed to mirror the behavior of
/// hhbbc, while (3) is designed to be a lighter weight version of hhbbc's
/// behavior (unsetting unused locals possibly containing CoW values) as type
/// information is not inferred.
///
/// These transformations are configured by a pair of settings controlling their
/// aggressiveness:
///
///   1. Hack.Lang.OptimizeLocalLifetimes - enables/disables the optimization
///   2. Hack.Lang.OptimizeParamLifetimes - causes parameter locals to not be
///      optimized ensuring they remain available for debug_backtrace
fn optimize_lifetimes<'b>(
    emitter: &Emitter<'_, '_>,
    blocks: &[Block<'b>],
    data: &[BlockData],
    size_hint: usize,
    nparams: usize,
    decl_vars: &[StringId],
    nlocals: usize,
) -> Vec<Instruct> {
    let volatile = mark_volatile_locals(emitter, nparams, decl_vars, nlocals);
    let mut v = Vec::with_capacity(size_hint);
    for (i, block) in blocks.iter().enumerate() {
        if block.rpo_id.is_none() {
            v.extend(block.instrs.to_owned());
        } else {
            let catch_ant = block.catch.map(|ci| &data[ci].ant_in);
            let analysis = data[i].analyze_block(block, catch_ant);
            let mut adj_minstr = false;
            let mut minstr_locals = Vec::new();
            let instrs: Vec<Instruct> = block
                .instrs
                .iter()
                .enumerate()
                .flat_map(|(i, instr)| {
                    let mut ant = analysis.ant[i].clone();
                    ant.union_with(&volatile);
                    optimize_locals(
                        instr,
                        &analysis.avl[i],
                        &ant,
                        &mut adj_minstr,
                        &mut minstr_locals,
                    )
                })
                .collect();
            v.extend(instrs);
        }
    }
    v
}

pub fn optimize_body(
    emitter: &Emitter<'_, '_>,
    body: InstrSeq,
    nparams: usize,
    decl_vars: &[StringId],
) -> Vec<Instruct> {
    let body_instrs = body.to_vec();
    if !emitter.options().hhbc.optimize_local_lifetimes {
        return body_instrs;
    }

    let nlocals = max_local(&body_instrs) + 1;
    let cfg = make_cfg(&body_instrs);
    let data = make_block_data(nparams, nlocals, &cfg);
    if !emitter.systemlib() {
        print_data(&cfg, &data);
    }

    optimize_lifetimes(
        emitter,
        &cfg,
        &data,
        body_instrs.len(),
        nparams,
        decl_vars,
        nlocals,
    )
}
