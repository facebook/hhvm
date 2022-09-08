use oxidized_by_ref::typing_defs;
use oxidized_by_ref::typing_defs_core::Exact;

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
    Readonly(Box<Tyx>),
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub(crate) struct FunType {
    pub ret: Tyx,
}

/// Lossy conversion, falls back to Ty::Mixed
/// no `From` implementation because this is a reference->owned conversion rather than owned->owned
pub(crate) fn convert(ty_: &typing_defs::Ty_<'_>) -> Tyx {
    use typing_defs::Ty_;
    // TODO: can get more expressivity if we support Tapply, Tnewtype, soundly-approximated refinements
    match ty_ {
        Ty_::Tthis => Tyx::Mixed,
        Ty_::Tapply(_) => Tyx::Mixed,
        Ty_::Trefinement(_) => Tyx::Mixed,
        Ty_::Tmixed => Tyx::Mixed,
        Ty_::Tlike(_) => Tyx::Mixed,
        Ty_::Terr => Tyx::GiveUp,
        Ty_::Tdynamic => Tyx::GiveUp,
        Ty_::Toption(_) => Tyx::Mixed,
        Ty_::Tprim(_) => Tyx::Primitive,
        Ty_::Tfun(_) => {
            let converted_ft = ty_to_fun_type_opt(ty_).unwrap();
            Tyx::Fun(Box::new(converted_ft))
        }
        Ty_::Ttuple(_) => Tyx::Mixed,
        Ty_::Tshape(_) => Tyx::Mixed,
        Ty_::Tvar(_) => Tyx::Mixed,
        Ty_::Tgeneric(_) => Tyx::Mixed,
        Ty_::Tunion(ty_s) => {
            if ty_s.is_empty() {
                Tyx::Bottom
            } else {
                Tyx::Mixed
            }
        }
        Ty_::Tintersection(_) => Tyx::Mixed,
        Ty_::TunappliedAlias(_) => Tyx::Mixed,
        Ty_::Tnewtype(_) => Tyx::Mixed,
        Ty_::Tclass(((_, class_name), Exact::Exact, _ty_args)) => Tyx::Object {
            class_name: class_name.to_string(),
        },
        Ty_::Tclass((_, Exact::Nonexact(_refinements), _ty_args)) => Tyx::Mixed,
        Ty_::Tneg(_) => Tyx::Mixed,
        _ => Tyx::Mixed,
    }
}

pub(crate) fn ty_to_fun_type_opt(ty_: &typing_defs::Ty_<'_>) -> Option<FunType> {
    match ty_ {
        typing_defs::Ty_::Tfun(ft) => {
            let ret = convert(&ft.ret.type_.1);
            let ret = if ft
                .flags
                .contains(typing_defs::FunTypeFlags::RETURNS_READONLY)
            {
                Tyx::Readonly(Box::new(ret))
            } else {
                ret
            };
            Some(FunType { ret })
        }
        _ => None,
    }
}
