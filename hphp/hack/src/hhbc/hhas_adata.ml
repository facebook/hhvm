(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  adata_id: Hhbc_ast.adata_id;
  adata_value: Typed_value.t;
}

let make adata_id adata_value = { adata_id; adata_value }

let id adata = adata.adata_id

let value adata = adata.adata_value
