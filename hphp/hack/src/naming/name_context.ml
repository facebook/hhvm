(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type t =
  | FunctionNamespace
  | ConstantNamespace
  | ModuleNamespace
  | PackageNamespace
  | TypeNamespace  (** Classes, interfaces, traits, records and type aliases.*)
  (* The following are all subsets of TypeNamespace, used when we can
     give a more specific naming error. E.g. `use Foo;` only allows
     traits. *)
  | TraitContext
  | ClassContext
[@@deriving compare, eq, show]

let to_string = function
  | FunctionNamespace -> "function"
  | ConstantNamespace -> "constant"
  | TypeNamespace -> "type"
  | TraitContext -> "trait"
  | ClassContext -> "class"
  | ModuleNamespace -> "module"
  | PackageNamespace -> "package"
