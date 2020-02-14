// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Interestingly, HHAS does not represent the declared types of constants,
// unlike formal parameters and return types. We might consider fixing this.
// Also interestingly, abstract constants are not emitted at all.

use hhbc_id_rust as hhbc_id;
use instruction_sequence_rust::InstrSeq;
use runtime::TypedValue;

#[derive(Debug)]
pub struct HhasConstant<'id> {
    pub name: hhbc_id::r#const::Type<'id>,
    pub value: Option<TypedValue>,
    pub initializer_instrs: Option<InstrSeq>,
}
