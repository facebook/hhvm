#![deny(warnings, rust_2018_idioms)]
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

pub mod bytestring;
pub mod de;
mod errors;
mod header;
pub mod ser;
pub mod value;

pub use crate::de::from_reader;
pub use crate::de::from_slice;
