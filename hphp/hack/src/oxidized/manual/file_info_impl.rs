// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::convert::{From, TryFrom};

use crate::gen::file_info::Mode;
use crate::gen::file_info::NameType;
use crate::gen::naming_types::KindOfType;

impl Mode {
    pub fn from_string(s: &str) -> Option<Self> {
        match s {
            "strict" | "" => Some(Mode::Mstrict),
            "partial" => Some(Mode::Mpartial),
            _ => None,
        }
    }
}

impl TryFrom<u32> for NameType {
    type Error = String;

    fn try_from(kind: u32) -> Result<Self, String> {
        match kind {
            0 => Ok(NameType::Fun),
            1 => Ok(NameType::Class),
            2 => Ok(NameType::RecordDef),
            3 => Ok(NameType::Typedef),
            4 => Ok(NameType::Const),
            _ => Err(format!("Out of range for NameType: {}", kind)),
        }
    }
}

impl From<KindOfType> for NameType {
    fn from(kind: KindOfType) -> Self {
        match kind {
            KindOfType::TClass => NameType::Class,
            KindOfType::TTypedef => NameType::Typedef,
            KindOfType::TRecordDef => NameType::RecordDef,
        }
    }
}
