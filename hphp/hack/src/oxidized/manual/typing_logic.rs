use crate::typing_defs::*;
use ocamlrep::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

/// The reason why something is expected to have a certain type
/// This has to be defined manually (not in oxidized/gen) because there is a function type in the original
/// definition of Disj
/// @TODO: work out what to do about this!
#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum SubtypeProp {
    Coerce(Ty, Ty),
    IsSubtype(InternalType, InternalType),
    Conj(Vec<SubtypeProp>),
    Disj(Vec<SubtypeProp>),
}

impl OcamlRep for SubtypeProp {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&self, _alloc: &'a A) -> ocamlrep::Value<'a> {
        unimplemented!()
    }

    fn from_ocamlrep(_value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        unimplemented!()
    }
}
