// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod formatters;
pub mod print;
pub mod util;

use std::fmt;

pub use formatters::FmtBid;
pub use formatters::FmtEnforceableType;
pub use formatters::FmtInstr;
pub use formatters::FmtLid;
pub use formatters::FmtLoc;
pub use formatters::FmtLocId;
pub use formatters::FmtRawBid;
pub use formatters::FmtRawVid;
pub use formatters::FmtVid;
use ir_core::BlockId;
use ir_core::Func;
use ir_core::InstrId;
use ir_core::InstrIdSet;
pub use print::print_unit;
pub use util::FmtEscapedString;
pub use util::FmtOption;
pub use util::FmtOptionOr;
pub use util::FmtSep;

// This isn't used by the print crate but is useful for code that wants to print
// a Func for debugging purposes.
pub struct DisplayFunc<'b> {
    pub func: &'b Func,
    /* verbose */ pub verbose: bool,
    pub f_pre_block: Option<&'b dyn Fn(&mut dyn fmt::Write, BlockId) -> fmt::Result>,
    pub f_pre_instr: Option<&'b dyn Fn(&mut dyn fmt::Write, InstrId) -> fmt::Result>,
}

impl<'b> DisplayFunc<'b> {
    pub fn new(func: &'b Func, verbose: bool) -> Self {
        DisplayFunc {
            func,
            verbose,
            f_pre_block: None,
            f_pre_instr: None,
        }
    }
}

impl fmt::Display for DisplayFunc<'_> {
    fn fmt(&self, w: &mut fmt::Formatter<'_>) -> fmt::Result {
        let DisplayFunc {
            func,
            verbose,
            f_pre_block,
            f_pre_instr,
        } = *self;
        writeln!(w, "func {} {{", formatters::FmtFuncParams(func))?;
        print::print_func_body(w, func, verbose, f_pre_block, f_pre_instr)?;
        writeln!(w, "}}")?;

        if verbose {
            let mut unused: InstrIdSet = (0..func.instrs.len()).map(InstrId::from_usize).collect();
            for iid in func.body_iids() {
                unused.remove(&iid);
            }
            for iid in func
                .block_ids()
                .flat_map(|bid| func.block(bid).params.iter().copied())
            {
                unused.remove(&iid);
            }
            if !unused.is_empty() {
                let mut unused: Vec<InstrId> = unused.into_iter().collect();
                unused.sort();

                writeln!(w, "unowned:")?;
                for iid in unused {
                    writeln!(w, "  {iid}: {:?}", func.instrs[iid])?;
                }
            }
        }
        writeln!(w)
    }
}
