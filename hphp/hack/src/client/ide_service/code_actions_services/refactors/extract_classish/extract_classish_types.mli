(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type candidate = {
  class_: Tast.class_;
  selected_methods: Tast.method_ list;
      (** The user triggers the refactor by selecting
  a range within a class body. We ignore any
  properties, constants, attributes, etc. in this range.
  *)
}
