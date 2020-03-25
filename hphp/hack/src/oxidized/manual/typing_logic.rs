use crate::typing_defs::*;
use ocamlrep_derive::OcamlRep;
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
    Serialize,
    OcamlRep
)]
pub enum SubtypeProp {
    Coerce(Ty, Ty),
    IsSubtype(InternalType, InternalType),
    Conj(Vec<SubtypeProp>),
    Disj(Vec<SubtypeProp>),
}

impl Default for SubtypeProp {
    fn default() -> Self {
        SubtypeProp::Disj(vec![])
    }
}
