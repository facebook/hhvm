// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::print;
use ffi::Str;
use hash::HashSet;
use hhbc::{
    AdataId, BareThisOp, ClassName, ClassNum, CollectionType, ConstName, ContCheckOp, FCallArgs,
    FatalOp, FunctionName, IncDecOp, InitPropOp, IsLogAsDynamicCallOp, IsTypeOp, IterArgs, IterId,
    Label, Local, LocalRange, MOpMode, MemberKey, MethodName, NumParams, OODeclExistsOp,
    ObjMethodOp, Opcode, ParamName, PropName, QueryMOp, ReadonlyOp, SetOpOp, SetRangeOp, SilenceOp,
    SpecialClsRef, StackIndex, SwitchKind, TypeStructResolveOp,
};
use hhbc_string_utils::float;
use print_opcode::{PrintOpcode, PrintOpcodeTypes};
use std::io::{Error, ErrorKind, Result, Write};

#[derive(PrintOpcode)]
#[print_opcode(override = "SSwitch")]
pub(crate) struct PrintOpcode<'a, 'b> {
    pub(crate) opcode: &'b Opcode<'a>,
    pub(crate) dv_labels: &'b HashSet<Label>,
    pub(crate) local_names: &'b [Str<'a>],
}

impl<'a, 'b> PrintOpcode<'a, 'b> {
    pub(crate) fn new(
        opcode: &'b Opcode<'a>,
        dv_labels: &'b HashSet<Label>,
        local_names: &'b [Str<'a>],
    ) -> Self {
        Self {
            opcode,
            dv_labels,
            local_names,
        }
    }

    fn get_opcode(&self) -> &'b Opcode<'a> {
        self.opcode
    }

    fn print_branch_labels(&self, w: &mut dyn Write, labels: &[Label]) -> Result<()> {
        w.write_all(b"<")?;
        for (i, label) in labels.iter().enumerate() {
            if i != 0 {
                w.write_all(b" ")?;
            }
            self.print_label(w, label)?;
        }
        w.write_all(b">")
    }

    fn print_fcall_args(&self, w: &mut dyn Write, args: &FCallArgs<'_>) -> Result<()> {
        print::print_fcall_args(w, args, self.dv_labels)
    }

    fn print_label(&self, w: &mut dyn Write, label: &Label) -> Result<()> {
        print::print_label(w, label, self.dv_labels)
    }

    fn print_label2(&self, w: &mut dyn Write, [label1, label2]: &[Label; 2]) -> Result<()> {
        self.print_label(w, label1)?;
        w.write_all(b" ")?;
        self.print_label(w, label2)
    }

    fn print_local(&self, w: &mut dyn Write, local: &Local) -> Result<()> {
        print_local(w, local, self.local_names)
    }

    fn print_iter_args(&self, w: &mut dyn Write, iter_args: &IterArgs) -> Result<()> {
        print_iter_args(w, iter_args, self.local_names)
    }

    fn print_member_key(&self, w: &mut dyn Write, member_key: &MemberKey<'_>) -> Result<()> {
        print_member_key(w, member_key, self.local_names)
    }

    fn print_s_switch(
        &self,
        w: &mut dyn Write,
        cases: &[Str<'_>],
        targets: &[Label],
        _dummy_imm: &u8,
    ) -> Result<()> {
        if cases.len() != targets.len() {
            return Err(Error::new(
                ErrorKind::Other,
                "sswitch cases and targets must match length",
            ));
        }

        let mut iter = cases.iter().zip(targets).rev();
        let (_, last_target) = if let Some(last) = iter.next() {
            last
        } else {
            return Err(Error::new(
                ErrorKind::Other,
                "sswitch should have at least one case",
            ));
        };
        let iter = iter.rev();

        w.write_all(b"SSwitch <")?;
        for (case, target) in iter {
            print_quoted_str(w, case)?;
            w.write_all(b":")?;
            self.print_label(w, target)?;
            w.write_all(b" ")?;
        }

        w.write_all(b"-:")?;
        self.print_label(w, last_target)?;
        w.write_all(b">")
    }
}

macro_rules! print_with_display {
    ($func_name: ident, $ty: ty) => {
        fn $func_name(w: &mut dyn Write, arg: &$ty) -> Result<()> {
            write!(w, "{}", arg)
        }
    };
}

print_with_display!(print_class_num, ClassNum);
print_with_display!(print_num_params, NumParams);
print_with_display!(print_stack_index, StackIndex);

macro_rules! print_with_debug {
    ($vis: vis $func_name: ident, $ty: ty) => {
        $vis fn $func_name(w: &mut dyn Write, arg: &$ty) -> Result<()> {
            write!(w, "{:?}", arg)
        }
    };
}

print_with_debug!(print_bare_this_op, BareThisOp);
print_with_debug!(print_collection_type, CollectionType);
print_with_debug!(print_cont_check_op, ContCheckOp);
print_with_debug!(print_fatal_op, FatalOp);
print_with_debug!(print_inc_dec_op, IncDecOp);
print_with_debug!(print_init_prop_op, InitPropOp);
print_with_debug!(print_is_log_as_dynamic_call_op, IsLogAsDynamicCallOp);
print_with_debug!(print_is_type_op, IsTypeOp);
print_with_debug!(print_m_op_mode, MOpMode);
print_with_debug!(print_obj_method_op, ObjMethodOp);
print_with_debug!(print_oo_decl_exists_op, OODeclExistsOp);
print_with_debug!(print_query_m_op, QueryMOp);
print_with_debug!(print_readonly_op, ReadonlyOp);
print_with_debug!(print_set_op_op, SetOpOp);
print_with_debug!(print_set_range_op, SetRangeOp);
print_with_debug!(print_silence_op, SilenceOp);
print_with_debug!(print_special_cls_ref, SpecialClsRef);
print_with_debug!(print_switch_kind, SwitchKind);
print_with_debug!(print_type_struct_resolve_op, TypeStructResolveOp);

fn print_adata_id(w: &mut dyn Write, id: &AdataId<'_>) -> Result<()> {
    w.write_all(b"@")?;
    print_str(w, id)
}

fn print_class_name(w: &mut dyn Write, id: &ClassName<'_>) -> Result<()> {
    print_quoted_str(w, &id.as_ffi_str())
}

fn print_const_name(w: &mut dyn Write, id: &ConstName<'_>) -> Result<()> {
    print_quoted_str(w, &id.as_ffi_str())
}

fn print_double(w: &mut dyn Write, d: f64) -> Result<()> {
    write!(w, "{}", float::to_string(d))
}

fn print_function_name(w: &mut dyn Write, id: &FunctionName<'_>) -> Result<()> {
    print_quoted_str(w, &id.as_ffi_str())
}

fn print_iter_args(w: &mut dyn Write, iter_args: &IterArgs, local_names: &[Str<'_>]) -> Result<()> {
    print_iterator_id(w, &iter_args.iter_id)?;
    if iter_args.key_id.is_valid() {
        w.write_all(b" K:")?;
        print_local(w, &iter_args.key_id, local_names)?;
    } else {
        w.write_all(b" NK")?;
    }
    w.write_all(b" V:")?;
    print_local(w, &iter_args.val_id, local_names)
}

fn print_iterator_id(w: &mut dyn Write, i: &IterId) -> Result<()> {
    write!(w, "{}", i)
}

fn print_local(w: &mut dyn Write, local: &Local, local_names: &[Str<'_>]) -> Result<()> {
    match local_names.get(local.idx as usize) {
        Some(name) => write!(w, "{}", name.unsafe_as_str()),
        None => write!(w, "_{}", local.idx),
    }
}

fn print_local_range(w: &mut dyn Write, locrange: &LocalRange) -> Result<()> {
    write!(w, "L:{}+{}", locrange.start, locrange.len)
}

fn print_member_key(w: &mut dyn Write, mk: &MemberKey<'_>, local_names: &[Str<'_>]) -> Result<()> {
    use MemberKey as M;
    match mk {
        M::EC(si, op) => {
            w.write_all(b"EC:")?;
            print_stack_index(w, si)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::EL(local, op) => {
            w.write_all(b"EL:")?;
            print_local(w, local, local_names)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::ET(s, op) => {
            w.write_all(b"ET:")?;
            print_quoted_str(w, s)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::EI(i, op) => {
            write!(w, "EI:{} ", i)?;
            print_readonly_op(w, op)
        }
        M::PC(si, op) => {
            w.write_all(b"PC:")?;
            print_stack_index(w, si)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::PL(local, op) => {
            w.write_all(b"PL:")?;
            print_local(w, local, local_names)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::PT(id, op) => {
            w.write_all(b"PT:")?;
            print_prop_name(w, id)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::QT(id, op) => {
            w.write_all(b"QT:")?;
            print_prop_name(w, id)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::W => w.write_all(b"W"),
    }
}

fn print_method_name(w: &mut dyn Write, id: &MethodName<'_>) -> Result<()> {
    print_quoted_str(w, &id.as_ffi_str())
}

fn print_param_name(w: &mut dyn Write, param_name: &ParamName<'_>) -> Result<()> {
    match param_name {
        ParamName::ParamUnnamed(i) => w.write_all(i.to_string().as_bytes()),
        ParamName::ParamNamed(s) => w.write_all(s),
    }
}

pub(crate) fn print_prop_name(w: &mut dyn Write, id: &PropName<'_>) -> Result<()> {
    print_quoted_str(w, &id.as_ffi_str())
}

fn print_quoted_str(w: &mut dyn Write, s: &ffi::Str<'_>) -> Result<()> {
    w.write_all(b"\"")?;
    w.write_all(&escaper::escape_bstr(s.as_bstr()))?;
    w.write_all(b"\"")
}

fn print_shape_fields(w: &mut dyn Write, keys: &[ffi::Str<'_>]) -> Result<()> {
    w.write_all(b"<")?;
    for (i, key) in keys.iter().enumerate() {
        if i != 0 {
            w.write_all(b" ")?;
        }
        print_quoted_str(w, key)?;
    }
    w.write_all(b">")
}

fn print_str(w: &mut dyn Write, s: &ffi::Str<'_>) -> Result<()> {
    use bstr::ByteSlice;
    w.write_all(s.as_bytes())
}

impl<'a, 'b> PrintOpcodeTypes for PrintOpcode<'a, 'b> {
    type Write = dyn Write + 'b;
    type Error = Error;
}
