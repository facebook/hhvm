// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use naming_special_names_rust::{self as sn, user_attributes::*};
use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};
use oxidized::{aast as a, ast_defs};
use std::{convert::TryFrom, str::FromStr};

pub type Conditional = bool;

/// The possible Rx levels of a function or method
#[derive(Debug, Clone, Copy, PartialEq, FromOcamlRep, ToOcamlRep)]
pub enum Level {
    NonRx,
    RxLocal,
    RxShallow,
    Rx,
    Pure,
}

#[derive(Debug, PartialEq)]
pub struct RxNone; // this is used as alternative (None was)

/// Implicitly provides via blanket impl: TryInto<&str> for Rx
impl TryFrom<Level> for &str {
    type Error = RxNone;
    fn try_from(level: Level) -> Result<Self, Self::Error> {
        use Level::*;
        match level {
            NonRx => Err(RxNone),
            RxLocal => Ok("rx_local"),
            RxShallow => Ok("rx_shallow"),
            Rx => Ok("rx"),
            Pure => Ok("pure"),
        }
    }
}

/// Implicitly provides via blanket impl: TryInto<Level> for &str
impl TryFrom<&str> for Level {
    type Error = RxNone; //&'static str;
    fn try_from(s: &str) -> Result<Self, Self::Error> {
        <Self as FromStr>::from_str(&s)
    }
}

impl FromStr for Level {
    type Err = RxNone;
    fn from_str(s: &str) -> Result<Self, Self::Err> {
        use Level::*;
        match s {
            "rx_local" => Ok(RxLocal),
            "rx_shallow" => Ok(RxShallow),
            "rx" => Ok(Rx),
            "pure" => Ok(Pure),
            _ => Err(RxNone),
        }
    }
}

impl Level {
    pub fn from_ast<Ex, Fb, En, Hi>(
        ast_attrs: impl AsRef<[a::UserAttribute<Ex, Fb, En, Hi>]>,
    ) -> Option<Self> {
        let ast_attrs = ast_attrs.as_ref();
        let attrs_contain = |name| ast_attrs.iter().any(|attr| attr.name.1 == name);
        let pure = attrs_contain(PURE);
        let rx = attrs_contain(REACTIVE);
        let non_rx = attrs_contain(NON_RX);
        let rx_shallow = attrs_contain(SHALLOW_REACTIVE);
        let rx_local = attrs_contain(LOCAL_REACTIVE);
        match (rx_local, rx_shallow, rx, pure) {
            (true, false, false, false) => Some(Self::RxLocal),
            (false, true, false, false) => Some(Self::RxShallow),
            (false, false, true, false) => Some(Self::Rx),
            (false, false, false, true) => Some(Self::Pure),
            (false, false, false, false) => {
                if non_rx {
                    Some(Self::NonRx)
                } else {
                    None
                }
            }
            _ => panic!("invalid combination of Rx attributes escaped the parser"),
        }
    }

    pub fn is_non_rx(&self) -> bool {
        if let Self::NonRx = self {
            return true;
        }
        false
    }
}

pub fn halves_of_is_enabled_body<Ex, Fb, En, Hi>(
    body: &a::FuncBody<Ex, Fb, En, Hi>,
) -> Option<(&a::Block<Ex, Fb, En, Hi>, &a::Block<Ex, Fb, En, Hi>)> {
    use a::*;
    if let [Stmt(_, Stmt_::If(if_))] = body.ast.as_slice() {
        if let (Expr(_, Expr_::Id(sid)), enabled, disabled) = &**if_ {
            let ast_defs::Id(_, name) = &**sid;
            return if name != sn::rx::IS_ENABLED {
                None
            } else {
                match disabled.as_slice() {
                    [] | [Stmt(_, Stmt_::Noop)] => None,
                    _ => Some((enabled, disabled)),
                }
            };
        }
    }
    None
}

// TODO(hrust) port remaining

#[cfg(test)]
mod tests {
    use super::*;

    use std::convert::TryInto;

    #[test]
    fn test_level_to_str_implicit_impl() {
        use Level::*;
        assert_eq!(Rx.try_into(), Ok("rx"));
        assert_eq!(RxLocal.try_into(), Ok("rx_local"));

        let s: Result<&str, RxNone> = NonRx.try_into();
        assert_eq!(s, Err(RxNone));
    }

    #[test]
    fn test_str_to_level_implicit_impl() {
        use Level::*;
        assert_eq!("rx".try_into(), Ok(Rx));
        assert_eq!("rx_shallow".try_into(), Ok(RxShallow));

        let level: Result<Level, RxNone> = "".try_into();
        assert_eq!(level, Err(RxNone));
    }
}
