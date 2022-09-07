use oxidized::typing_defs::FunTypeFlags;
use oxidized_by_ref::typing_defs;

//// Using a stripped-down type system to get some baselines for inference.
/// This will likely be replaced with rupro types
#[derive(Clone, Debug, Eq, PartialEq)]
pub(crate) enum Tyx {
    Fun(Box<FunType>),
    // Class{class_name: String},
    Object {
        class_name: String,
    },
    GiveUp, // We're not sure what the type is
    Todo,   // should be gone later
    #[allow(dead_code)]
    Bottom, // like empty union in hh
    Primitive,
    Mixed, // top
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub(crate) struct FunType {
    pub flags: FunTypeFlags,
    pub ret: Tyx,
}

pub(crate) fn ty_to_fun_type_opt(ty_: &typing_defs::Ty_<'_>) -> Option<FunType> {
    match ty_ {
        typing_defs::Ty_::Tfun(ft) => Some(FunType {
            flags: ft.flags,
            ret: Tyx::Todo,
        }),
        _ => None,
    }
}
