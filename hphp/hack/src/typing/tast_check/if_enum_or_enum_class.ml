(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type kind =
  | Enum
  | EnumClass
  | EnumClassLabel

let apply
    env ~(default : 'a) ~(f : kind -> Tast_env.env -> string -> 'a) name args =
  let check_ec kind = function
    | [enum; _interface] -> begin
      match Typing_defs.get_node enum with
      | Typing_defs.Tclass ((_, cid), _, _) when Tast_env.is_enum_class env cid
        ->
        f kind env cid
      | _ -> default
    end
    | _ -> default
  in
  if Tast_env.is_enum env name then
    f Enum env name
  else if String.equal name Naming_special_names.Classes.cMemberOf then
    check_ec EnumClass args
  else if String.equal name Naming_special_names.Classes.cEnumClassLabel then
    check_ec EnumClassLabel args
  else
    default
