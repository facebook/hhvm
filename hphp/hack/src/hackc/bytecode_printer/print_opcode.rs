// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::print;
use ffi::{Maybe, Str};
use hash::HashSet;
use hhbc_ast::{
    AdataId, BareThisOp, ClassId, ClassNum, CollectionType, ConstId, ContCheckOp, FCallArgs,
    FatalOp, FunctionId, IncDecOp, InitPropOp, IsLogAsDynamicCallOp, IsTypeOp, IterArgs,
    LocalRange, MOpMode, MemberKey, MethodId, NumParams, OODeclExistsOp, ObjMethodOp, Opcode,
    ParamId, PropId, QueryMOp, ReadonlyOp, SetOpOp, SetRangeOp, SilenceOp, SpecialClsRef,
    StackIndex, SwitchKind, TypeStructResolveOp,
};
use hhbc_string_utils::float;
use iterator::IterId;
use label::Label;
use local::Local;
use print_opcode::{PrintOpcode, PrintOpcodeTypes};
use std::io::{Error, ErrorKind, Result, Write};

#[derive(PrintOpcode)]
#[print_opcode(override = "SSwitch")]
pub(crate) struct PrintOpcode<'a, 'b, 'c> {
    pub(crate) opcode: &'b Opcode<'a>,
    pub(crate) dv_labels: &'b HashSet<Label>,
    pub(crate) phantom: std::marker::PhantomData<&'c i8>,
}

impl<'a, 'b, 'c> PrintOpcode<'a, 'b, 'c> {
    pub(crate) fn new(opcode: &'b Opcode<'a>, dv_labels: &'b HashSet<Label>) -> Self {
        Self {
            opcode,
            dv_labels,
            phantom: Default::default(),
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

    fn print_s_switch(
        &self,
        w: &mut dyn Write,
        cases: &[Str<'_>],
        targets: &[Label],
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

fn print_class_id(w: &mut dyn Write, id: &ClassId<'_>) -> Result<()> {
    print_quoted_str(w, &id.as_ffi_str())
}

fn print_const_id(w: &mut dyn Write, id: &ConstId<'_>) -> Result<()> {
    print_quoted_str(w, &id.as_ffi_str())
}

fn print_double(w: &mut dyn Write, d: f64) -> Result<()> {
    write!(w, "{}", float::to_string(d))
}

fn print_function_id(w: &mut dyn Write, id: &FunctionId<'_>) -> Result<()> {
    print_quoted_str(w, &id.as_ffi_str())
}

fn print_iter_args(w: &mut dyn Write, iter_args: &IterArgs<'_>) -> Result<()> {
    print_iterator_id(w, &iter_args.iter_id)?;
    w.write_all(b" ")?;
    match &iter_args.key_id {
        Maybe::Nothing => w.write_all(b"NK")?,
        Maybe::Just(k) => {
            w.write_all(b"K:")?;
            print_local(w, k)?;
        }
    };
    w.write_all(b" ")?;
    w.write_all(b"V:")?;
    print_local(w, &iter_args.val_id)
}

fn print_iterator_id(w: &mut dyn Write, i: &IterId) -> Result<()> {
    write!(w, "{}", i)
}

fn print_local(w: &mut dyn Write, local: &Local<'_>) -> Result<()> {
    match local {
        Local::Unnamed(id) => write!(w, "_{}", id),
        Local::Named(id) => w.write_all(id),
    }
}

fn print_local_range(w: &mut dyn Write, locrange: &LocalRange) -> Result<()> {
    write!(w, "L:{}+{}", locrange.start, locrange.len)
}

fn print_member_key(w: &mut dyn Write, mk: &MemberKey<'_>) -> Result<()> {
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
            print_local(w, local)?;
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
            print_local(w, local)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::PT(id, op) => {
            w.write_all(b"PT:")?;
            print_prop_id(w, id)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::QT(id, op) => {
            w.write_all(b"QT:")?;
            print_prop_id(w, id)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::W => w.write_all(b"W"),
    }
}

fn print_method_id(w: &mut dyn Write, id: &MethodId<'_>) -> Result<()> {
    print_quoted_str(w, &id.as_ffi_str())
}

fn print_param_id(w: &mut dyn Write, param_id: &ParamId<'_>) -> Result<()> {
    match param_id {
        ParamId::ParamUnnamed(i) => w.write_all(i.to_string().as_bytes()),
        ParamId::ParamNamed(s) => w.write_all(s),
    }
}

pub(crate) fn print_prop_id(w: &mut dyn Write, id: &PropId<'_>) -> Result<()> {
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

impl<'a, 'b, 'c> PrintOpcodeTypes for PrintOpcode<'a, 'b, 'c> {
    type Write = dyn Write + 'c;
    type Error = Error;
}
