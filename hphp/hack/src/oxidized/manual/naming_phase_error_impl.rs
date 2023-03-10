// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::naming_error::NamingError;
use crate::naming_phase_error::ExperimentalFeature;
use crate::naming_phase_error::NamingPhaseError;
use crate::nast_check_error::NastCheckError;

impl From<NamingError> for NamingPhaseError {
    fn from(err: NamingError) -> Self {
        Self::Naming(err)
    }
}

impl From<NastCheckError> for NamingPhaseError {
    fn from(err: NastCheckError) -> Self {
        Self::NastCheck(err)
    }
}

impl From<ExperimentalFeature> for NamingPhaseError {
    fn from(err: ExperimentalFeature) -> Self {
        Self::ExperimentalFeature(err)
    }
}
