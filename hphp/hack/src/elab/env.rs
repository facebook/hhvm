// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::RefCell;

use bitflags::bitflags;
use oxidized::naming_phase_error::NamingPhaseError;
use oxidized::typechecker_options::TypecheckerOptions;

#[derive(Debug, Clone, Default)]
pub struct ProgramSpecificOptions {
    pub is_hhi: bool,
    pub allow_module_declarations: bool,
}

bitflags! {
    #[derive(PartialEq, Eq, PartialOrd, Ord, Hash, Debug, Clone, Copy)]
    struct Flags: u16 {
        const SOFT_AS_LIKE = 1 << 0;
        const HKT_ENABLED = 1 << 1;
        const IS_HHI = 1 << 2;
        const IS_SYSTEMLIB = 1 << 3;
        const LIKE_TYPE_HINTS_ENABLED = 1 << 4;
        const CONST_ATTRIBUTE = 1 << 5;
        const CONST_STATIC_PROPS = 1 << 6;
        const ALLOW_MODULE_DECLARATIONS = 1 << 7;
        const ERROR_PHP_LAMBDAS = 1 << 9;
        const INFER_FLOWS = 1 << 10;
        const EVERYTHING_SDT = 1 << 11;
        const SUPPORTDYNAMIC_TYPE_HINT_ENABLED = 1 << 12;
        const NO_AUTO_DYNAMIC_ENABLED = 1 << 13;
    }
}

impl Flags {
    pub fn new(tco: &TypecheckerOptions, pso: &ProgramSpecificOptions) -> Self {
        let mut flags: Self = Flags::empty();

        flags.set(
            Self::SOFT_AS_LIKE,
            tco.po_interpret_soft_types_as_like_types,
        );

        flags.set(Self::HKT_ENABLED, tco.tco_higher_kinded_types);
        flags.set(Self::IS_SYSTEMLIB, tco.tco_is_systemlib);
        flags.set(Self::LIKE_TYPE_HINTS_ENABLED, tco.tco_like_type_hints);
        flags.set(
            Self::NO_AUTO_DYNAMIC_ENABLED,
            tco.tco_enable_no_auto_dynamic,
        );
        flags.set(
            Self::SUPPORTDYNAMIC_TYPE_HINT_ENABLED,
            tco.tco_experimental_features
                .contains("supportdynamic_type_hint"),
        );
        flags.set(Self::EVERYTHING_SDT, tco.tco_everything_sdt);
        flags.set(Self::CONST_ATTRIBUTE, tco.tco_const_attribute);
        flags.set(Self::CONST_STATIC_PROPS, tco.tco_const_static_props);
        flags.set(Self::ERROR_PHP_LAMBDAS, tco.tco_error_php_lambdas);

        flags.set(Self::IS_HHI, pso.is_hhi);
        flags.set(
            Self::ALLOW_MODULE_DECLARATIONS,
            pso.allow_module_declarations,
        );

        flags.set(
            Self::INFER_FLOWS,
            tco.tco_experimental_features
                .contains(EXPERIMENTAL_INFER_FLOWS),
        );

        flags
    }
}

#[derive(Debug)]
pub struct Env {
    flags: Flags,
    errors: RefCell<Vec<NamingPhaseError>>,
    pub consistent_ctor_level: isize,
}

impl Default for Env {
    fn default() -> Self {
        Self::new(
            &TypecheckerOptions::default(),
            &ProgramSpecificOptions::default(),
        )
    }
}

impl Env {
    pub fn new(tco: &TypecheckerOptions, pso: &ProgramSpecificOptions) -> Self {
        Self {
            flags: Flags::new(tco, pso),
            errors: RefCell::new(vec![]),
            consistent_ctor_level: tco.tco_explicit_consistent_constructors,
        }
    }

    pub fn emit_error(&self, err: impl Into<NamingPhaseError>) {
        self.errors.borrow_mut().push(err.into())
    }

    pub fn assert_no_errors(&self) {
        assert!(self.errors.borrow().is_empty());
    }

    pub fn into_errors(self) -> Vec<NamingPhaseError> {
        self.errors.into_inner()
    }

    pub fn soft_as_like(&self) -> bool {
        self.flags.contains(Flags::SOFT_AS_LIKE)
    }

    pub fn error_php_lambdas(&self) -> bool {
        self.flags.contains(Flags::ERROR_PHP_LAMBDAS)
    }

    pub fn allow_module_declarations(&self) -> bool {
        self.flags.contains(Flags::ALLOW_MODULE_DECLARATIONS)
    }

    pub fn hkt_enabled(&self) -> bool {
        self.flags.contains(Flags::HKT_ENABLED)
    }

    pub fn is_systemlib(&self) -> bool {
        self.flags.contains(Flags::IS_SYSTEMLIB)
    }

    pub fn like_type_hints_enabled(&self) -> bool {
        self.flags.contains(Flags::LIKE_TYPE_HINTS_ENABLED)
    }

    pub fn supportdynamic_type_hint_enabled(&self) -> bool {
        self.flags.contains(Flags::SUPPORTDYNAMIC_TYPE_HINT_ENABLED)
    }

    pub fn no_auto_dynamic_enabled(&self) -> bool {
        self.flags.contains(Flags::NO_AUTO_DYNAMIC_ENABLED)
    }

    pub fn everything_sdt(&self) -> bool {
        self.flags.contains(Flags::EVERYTHING_SDT)
    }

    pub fn is_hhi(&self) -> bool {
        self.flags.contains(Flags::IS_HHI)
    }

    pub fn const_attribute(&self) -> bool {
        self.flags.contains(Flags::CONST_ATTRIBUTE)
    }

    pub fn const_static_props(&self) -> bool {
        self.flags.contains(Flags::CONST_STATIC_PROPS)
    }

    pub fn infer_flows(&self) -> bool {
        self.flags.contains(Flags::INFER_FLOWS)
    }
}

const EXPERIMENTAL_INFER_FLOWS: &str = "ifc_infer_flows";
