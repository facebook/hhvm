(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

class generator_visitor = object
  inherit [bool * bool] Ast_visitor.ast_visitor as _super

  method! on_yield (_, is_pair_generator) field =
    (* No need to call recursively on the field values as there cannot be
     * nested yields *)
    match field with
    | Ast.AFvalue _ -> (true, is_pair_generator)
    | Ast.AFkvalue (_, _) -> (true, true)

  method! on_yield_break (_, is_pair_generator) =
    (true, is_pair_generator)

  method! on_yield_from (_, is_pair_generator) _ =
    (true, is_pair_generator)

  method! on_class_ acc _ = acc
  method! on_fun_ acc _ = acc

end

(* Returns a tuple of is_generator and is_pair_generator *)
let is_function_generator b =
  let visitor = new generator_visitor in
  visitor#on_program (false, false) b
