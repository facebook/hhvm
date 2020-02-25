// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::convert::TryFrom;

use crate::gen::naming_table::TypeOfType;

impl TryFrom<i64> for TypeOfType {
    type Error = String;

    fn try_from(kind: i64) -> Result<Self, String> {
        match kind {
            0 => Ok(TypeOfType::TClass),
            1 => Ok(TypeOfType::TTypedef),
            2 => Ok(TypeOfType::TRecordDef),
            _ => Err(format!("Out of range for TypeOfType: {}", kind)),
        }
    }
}
