(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module T = Aast

let generator_visitor () =
  (* is_generator, is_pair_generator *)
  let state = ref (false, false) in
  object
    inherit [_] Aast.iter

    method state () = !state

    method! on_Yield () afield =
      match afield with
      | T.AFvalue _ ->
        let (_, is_pair_generator) = !state in
        state := (true, is_pair_generator)
      | T.AFkvalue (_, _) -> state := (true, true)

    method! on_Yield_break _ =
      let (_, is_pair_generator) = !state in
      state := (true, is_pair_generator)

    method! on_Yield_from _ _ =
      let (_, is_pair_generator) = !state in
      state := (true, is_pair_generator)

    method! on_class_ _ _ = ()

    method! on_fun_ _ _ = ()
  end

(* Returns a tuple of is_generator and is_pair_generator *)
let is_function_generator b =
  let visitor = generator_visitor () in
  let _ = visitor#on_program () b in
  visitor#state ()
