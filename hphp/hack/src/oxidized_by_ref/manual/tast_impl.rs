// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[allow(unused_imports)]
use crate::*;

use i_map::IMap;
use s_map::SMap;
use tast::SavedEnv;
use type_parameter_env::TypeParameterEnv;
use typing_inference_env::TypingInferenceEnv;
use typing_logic::SubtypeProp;
use typing_tyvar_occurrences::TypingTyvarOccurrences;

const INFERENCE_ENV: TypingInferenceEnv<'_> = TypingInferenceEnv {
    tvenv: IMap::empty(),
    tyvars_stack: &[],
    subtype_prop: SubtypeProp::default_ref(),
    tyvar_occurrences: &TYVAR_OCCURRENCES,
    allow_solve_globals: false,
};

const TYVAR_OCCURRENCES: TypingTyvarOccurrences<'_> = TypingTyvarOccurrences {
    tyvar_occurrences: IMap::empty(),
    tyvars_in_tyvar: IMap::empty(),
};

const TPENV: TypeParameterEnv<'_> = TypeParameterEnv {
    tparams: SMap::empty(),
    consistent: false,
};

impl Default for SavedEnv<'_> {
    fn default() -> Self {
        Self {
            tcopt: Default::default(),
            inference_env: &INFERENCE_ENV,
            tpenv: &TPENV,
            condition_types: Default::default(),
            pessimize: Default::default(),
            fun_tast_info: Default::default(),
        }
    }
}
