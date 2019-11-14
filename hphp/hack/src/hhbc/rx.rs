// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use naming_special_names_rust::user_attributes::*;
use oxidized::aast;
use std::{convert::TryFrom, str::FromStr};

/// The possible Rx levels of a function or method
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum Level {
    NonRx, // TODO this is redundant and ambiguously used in OCaml (== None)
    ConditionalRxLocal,
    ConditionalRxShallow,
    ConditionalRx,
    RxLocal,
    RxShallow,
    Rx,
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
            ConditionalRxLocal => Ok("conditional_rx_local"),
            ConditionalRxShallow => Ok("conditional_rx_shallow"),
            ConditionalRx => Ok("conditional_rx"),
            RxLocal => Ok("rx_local"),
            RxShallow => Ok("rx_shallow"),
            Rx => Ok("rx"),
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
            "conditional_rx_local" => Ok(ConditionalRxLocal),
            "conditional_rx_shallow" => Ok(ConditionalRxShallow),
            "conditional_rx" => Ok(ConditionalRx),
            "rx_local" => Ok(RxLocal),
            "rx_shallow" => Ok(RxShallow),
            "rx" => Ok(Rx),
            _ => Err(RxNone),
        }
    }
}

impl Level {
    pub fn from_ast<Ex, Fb, En, Hi>(
        ast_attrs: &Vec<aast::UserAttribute<Ex, Fb, En, Hi>>,
    ) -> Option<Self> {
        let attrs_contain = |name| ast_attrs.iter().any(|attr| attr.name.1 == name);
        let rx = attrs_contain(REACTIVE);
        let non_rx = attrs_contain(NON_RX);
        let rx_shallow = attrs_contain(SHALLOW_REACTIVE);
        let rx_local = attrs_contain(LOCAL_REACTIVE);
        let rx_conditional = attrs_contain(ONLY_RX_IF_IMPL) || attrs_contain(AT_MOST_RX_AS_ARGS);
        match (rx_conditional, non_rx, rx_local, rx_shallow, rx) {
            (false, false, false, false, false) => None,
            (false, true, false, false, false) => Some(Self::NonRx),
            (true, false, true, false, false) => Some(Self::ConditionalRxLocal),
            (true, false, false, true, false) => Some(Self::ConditionalRxShallow),
            (true, false, false, false, true) => Some(Self::ConditionalRx),
            (false, false, true, false, false) => Some(Self::RxLocal),
            (false, false, false, true, false) => Some(Self::RxShallow),
            (false, false, false, false, true) => Some(Self::Rx),
            _ => panic!("invalid combination of Rx attributes escaped the parser"),
        }
    }
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
        assert_eq!(ConditionalRxLocal.try_into(), Ok("conditional_rx_local"));

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
