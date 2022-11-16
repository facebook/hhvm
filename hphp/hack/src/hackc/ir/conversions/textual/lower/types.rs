use ir::BaseType;
use ir::ClassId;
use ir::EnforceableType;
use ir::StringInterner;
use ir::TypeConstraintFlags;
use ir::TypeInfo;
use log::trace;

pub(crate) fn lower_ty_in_place(ty: &mut TypeInfo, strings: &StringInterner) {
    let tmp = std::mem::take(ty);
    *ty = lower_ty(tmp, strings);
}

pub(crate) fn lower_ty(mut ty: TypeInfo, strings: &StringInterner) -> TypeInfo {
    ty.enforced = lower_enforced_ty(ty.enforced, strings);
    ty
}

pub(crate) fn lower_enforced_ty(ty: EnforceableType, strings: &StringInterner) -> EnforceableType {
    match ty {
        EnforceableType {
            ty: cls @ BaseType::Class(_),
            modifiers: TypeConstraintFlags::NoFlags,
        } => EnforceableType {
            ty: BaseType::RawPtr(Box::new(cls)),
            modifiers: TypeConstraintFlags::NoFlags,
        },
        EnforceableType {
            ty: BaseType::Float,
            modifiers: TypeConstraintFlags::NoFlags | TypeConstraintFlags::ExtendedHint,
        } => ty_hack_float(strings),
        EnforceableType {
            ty: BaseType::Int,
            modifiers: TypeConstraintFlags::NoFlags | TypeConstraintFlags::ExtendedHint,
        } => ty_hack_int(strings),
        EnforceableType {
            ty: BaseType::Mixed,
            modifiers: TypeConstraintFlags::NoFlags,
        } => ty_hack_mixed(strings),
        EnforceableType {
            ty: BaseType::None,
            modifiers: TypeConstraintFlags::NoFlags,
        } => ty_hack_mixed(strings),
        EnforceableType {
            ty: BaseType::String,
            modifiers: TypeConstraintFlags::NoFlags | TypeConstraintFlags::ExtendedHint,
        } => ty_hack_string(strings),
        EnforceableType {
            ty: BaseType::Void,
            modifiers: TypeConstraintFlags::NoFlags,
        } => ty_hack_void_ptr(),
        ref e => {
            trace!("PARAM: {}", ir::print::FmtEnforceableType(e, strings));
            textual_todo! { ty }
        }
    }
}

fn ty_class_ptr(name: &[u8], strings: &StringInterner) -> EnforceableType {
    let cid = ClassId::from_bytes(name, strings);
    EnforceableType {
        ty: BaseType::RawPtr(Box::new(BaseType::Class(cid))),
        modifiers: TypeConstraintFlags::NoFlags,
    }
}

fn ty_hack_float(strings: &StringInterner) -> EnforceableType {
    ty_class_ptr(b"HackFloat", strings)
}

fn ty_hack_int(strings: &StringInterner) -> EnforceableType {
    ty_class_ptr(b"HackInt", strings)
}

pub(crate) fn ty_hack_mixed(strings: &StringInterner) -> EnforceableType {
    ty_class_ptr(b"HackMixed", strings)
}

fn ty_hack_string(strings: &StringInterner) -> EnforceableType {
    ty_class_ptr(b"HackString", strings)
}

fn ty_hack_void_ptr() -> EnforceableType {
    EnforceableType {
        ty: BaseType::RawPtr(Box::new(BaseType::Void)),
        modifiers: TypeConstraintFlags::NoFlags,
    }
}
