// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use intern::string::BytesId;
pub type UnitBytesId = BytesId;
pub type UnitBytesIdMap<V> = intern::string::BytesIdMap<V>;
pub type UnitBytesIdSet = intern::string::BytesIdSet;

/// A string interner for associating IDs with unique string values.  If two
/// identical strings are inserted into the StringInterner they are guaranteed
/// to have the same UnitBytesId.
///
/// Note that there are no guarantees about the numerical values or ordering of
/// the resulting UnitBytesId - in particular use of StringInterner in
/// multi-thread situations will produce non-deterministic ID ordering.
///
/// Currently there is no easy facility to iterate the strings in-order - this
/// prevents accidental ordering misuse.
#[derive(Default, Debug)]
pub struct StringInterner;
