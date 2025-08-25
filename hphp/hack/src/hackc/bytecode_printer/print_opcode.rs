// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::io::Error;
use std::io::ErrorKind;
use std::io::Result;
use std::io::Write;

use hash::HashSet;
use hhbc::AdataState;
use hhbc::AsTypeStructExceptionKind;
use hhbc::BareThisOp;
use hhbc::BytesId;
use hhbc::ClassGetCMode;
use hhbc::ClassName;
use hhbc::CollectionType;
use hhbc::ConstName;
use hhbc::ContCheckOp;
use hhbc::FCallArgs;
use hhbc::FatalOp;
use hhbc::FloatBits;
use hhbc::FunctionName;
use hhbc::IncDecOp;
use hhbc::InitPropOp;
use hhbc::IsLogAsDynamicCallOp;
use hhbc::IsTypeOp;
use hhbc::IterArgs;
use hhbc::IterId;
use hhbc::Label;
use hhbc::Local;
use hhbc::LocalRange;
use hhbc::MOpMode;
use hhbc::MemberKey;
use hhbc::MethodName;
use hhbc::NumParams;
use hhbc::OODeclExistsOp;
use hhbc::ObjMethodOp;
use hhbc::Opcode;
use hhbc::PropName;
use hhbc::QueryMOp;
use hhbc::ReadonlyOp;
use hhbc::SetOpOp;
use hhbc::SetRangeOp;
use hhbc::SpecialClsRef;
use hhbc::StackIndex;
use hhbc::StringId;
use hhbc::SwitchKind;
use hhbc::TypeStructEnforceKind;
use hhbc::TypeStructResolveOp;
use hhbc::TypedValue;
use hhbc_string_utils::float;
use print_opcode::PrintOpcode;
use print_opcode::PrintOpcodeTypes;

use crate::print;
use crate::write::angle;

#[derive(PrintOpcode)]
#[print_opcode(override = "SSwitch")]
pub struct PrintOpcode<'b> {
    pub(crate) opcode: &'b Opcode,
    pub(crate) dv_labels: &'b HashSet<Label>,
    pub(crate) local_names: &'b [StringId],
}

impl<'b> PrintOpcode<'b> {
    pub fn new(
        opcode: &'b Opcode,
        dv_labels: &'b HashSet<Label>,
        local_names: &'b [StringId],
    ) -> Self {
        Self {
            opcode,
            dv_labels,
            local_names,
        }
    }

    fn get_opcode(&self) -> &'b Opcode {
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

    fn print_fcall_args(&self, w: &mut dyn Write, args: &FCallArgs) -> Result<()> {
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
        print_iter_args(w, iter_args)
    }

    fn print_member_key(&self, w: &mut dyn Write, member_key: &MemberKey) -> Result<()> {
        print_member_key(w, member_key, self.local_names)
    }

    fn print_s_switch(
        &self,
        w: &mut dyn Write,
        cases: &[BytesId],
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
            print_quoted_bytes(w, case.as_bytes())?;
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
print_with_debug!(print_class_get_c_mode, ClassGetCMode);
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
print_with_debug!(print_special_cls_ref, SpecialClsRef);
print_with_debug!(print_switch_kind, SwitchKind);
print_with_debug!(print_type_struct_resolve_op, TypeStructResolveOp);
print_with_debug!(print_type_struct_enforce_kind, TypeStructEnforceKind);
print_with_debug!(
    print_as_type_struct_exception_kind,
    AsTypeStructExceptionKind
);

fn print_adata_id(w: &mut dyn Write, v: &TypedValue, adata: &AdataState) -> Result<()> {
    let id = adata.index_of(v).unwrap();
    write!(w, "@{}", id)
}

fn print_class_name(w: &mut dyn Write, id: &ClassName) -> Result<()> {
    print_quoted_str(w, id.as_str())
}

fn print_const_name(w: &mut dyn Write, id: &ConstName) -> Result<()> {
    print_quoted_str(w, id.as_str())
}

fn print_float(w: &mut dyn Write, d: FloatBits) -> Result<()> {
    write!(w, "{}", float::to_string(d.to_f64()))
}

fn print_function_name(w: &mut dyn Write, id: &FunctionName) -> Result<()> {
    print_quoted_str(w, id.as_str())
}

fn print_iter_args(w: &mut dyn Write, iter_args: &IterArgs) -> Result<()> {
    angle(w, |w| {
        let flags = hhvm_hhbc_defs_ffi::ffi::iter_args_flags_to_string_ffi(iter_args.flags);
        write!(w, "{}", flags)
    })?;
    w.write_all(b" ")?;
    print_iterator_id(w, &iter_args.iter_id)
}

fn print_iterator_id(w: &mut dyn Write, i: &IterId) -> Result<()> {
    write!(w, "{}", i)
}

fn print_local(w: &mut dyn Write, local: &Local, local_names: &[StringId]) -> Result<()> {
    match local_names.get(local.index()) {
        Some(name) => write!(w, "{}", name.as_str()),
        None => write!(w, "_{}", local),
    }
}

fn print_local_range(w: &mut dyn Write, locrange: &LocalRange) -> Result<()> {
    write!(w, "L:{}+{}", locrange.start, locrange.len)
}

fn print_member_key(w: &mut dyn Write, mk: &MemberKey, local_names: &[StringId]) -> Result<()> {
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
            print_quoted_bytes(w, s.as_bytes())?;
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

fn print_method_name(w: &mut dyn Write, id: &MethodName) -> Result<()> {
    print_quoted_bytes(w, id.as_bytes())
}

pub(crate) fn print_prop_name(w: &mut dyn Write, id: &PropName) -> Result<()> {
    print_quoted_str(w, id.as_str())
}

fn print_quoted_bytes_id(w: &mut dyn Write, s: &BytesId) -> Result<()> {
    print_quoted_bytes(w, s.as_bytes())
}

fn print_quoted_str(w: &mut dyn Write, s: &str) -> Result<()> {
    w.write_all(b"\"")?;
    w.write_all(escaper::escape(s).as_bytes())?;
    w.write_all(b"\"")
}

fn print_quoted_bytes(w: &mut dyn Write, s: &[u8]) -> Result<()> {
    use bstr::ByteSlice as BS;
    w.write_all(b"\"")?;
    w.write_all(&escaper::escape_bstr(s.as_bstr()))?;
    w.write_all(b"\"")
}

fn print_shape_fields(w: &mut dyn Write, keys: &[BytesId]) -> Result<()> {
    w.write_all(b"<")?;
    for (i, key) in keys.iter().enumerate() {
        if i != 0 {
            w.write_all(b" ")?;
        }
        print_quoted_bytes(w, key.as_bytes())?;
    }
    w.write_all(b">")
}

fn print_string_id(w: &mut dyn Write, s: &StringId) -> Result<()> {
    w.write_all(s.as_str().as_bytes())
}

impl<'b> PrintOpcodeTypes for PrintOpcode<'b> {
    type Write = dyn Write + 'b;
    type Error = Error;
}
