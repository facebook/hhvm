// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhas_attribute_rust::HhasAttribute;
use hhas_record_def_rust::HhasRecord;
use hhas_symbol_refs_rust::HhasSymbolRefs;
use hhas_typedef_rust::Typedef;

pub struct HhasProgram<'a> {
    pub is_hh: bool,
    /* TODO(hrust): add `pub adata: Hhas_adata.t list` */
    /* TODO(hrust): add `pub fun: Hhas_function.t list` */
    /* TODO(hrust): add `pub classes: Hhas_class.t list` */
    pub record_defs: Vec<HhasRecord<'a>>,
    pub typedefs: Vec<Typedef<'a>>,
    pub file_attributes: Vec<HhasAttribute>,
    /* TODO(hrust): add `pub main: Hhas_body.t` */
    pub symbol_refs: Vec<HhasSymbolRefs>,
}
