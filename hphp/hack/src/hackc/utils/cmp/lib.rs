// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

macro_rules! bail {
    ($msg:literal $(,)?) => {
        return Err(CmpError::error(format!($msg)))
    };
    ($err:expr $(,)?) => {
        return Err(CmpError::error(format!($err)))
    };
    ($fmt:expr, $($arg:tt)*) => {
        return Err(CmpError::error(format!($fmt, $($arg)*)))
    };
}

pub mod cmp_ir;
pub mod cmp_unit;
mod context;
mod util;

type Result<T = (), E = CmpError> = std::result::Result<T, E>;

use context::CmpContext;
use util::cmp_eq;
use util::cmp_map_t;
use util::cmp_option;
use util::cmp_set_t;
use util::cmp_slice;
pub use util::CmpError;
use util::MapName;
