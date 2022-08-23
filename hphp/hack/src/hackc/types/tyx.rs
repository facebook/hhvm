use oxidized::typing_defs::FunTypeFlags;

//// Using a stripped-down type system to get some baselines for inference.
/// This will likely be replaced with rupro types
#[derive(Clone, Debug, Eq, PartialEq)]
pub(crate) enum Tyx {
    Fun(Box<FunType>),
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
