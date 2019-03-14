(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Tast

let visitor = object(this)
  inherit [_] Tast.iter as super

  val return_type_ref = ref None

  method with_return_type new_return_type f =
    let old_return_type = !return_type_ref in
    return_type_ref := new_return_type;
    f ();
    return_type_ref := old_return_type

  method! on_fun_ env fun_ =
    this#with_return_type fun_.f_ret (fun () -> super#on_fun_ env fun_)
  method! on_method_ env method_ =
    this#with_return_type method_.m_ret (fun () -> super#on_method_ env method_)
  method! on_Return _ pos1 = function
    | Some _ ->
      begin match !return_type_ref with
      | Some (pos2, Hprim Tvoid) -> Errors.return_in_void pos1 pos2
      | _ -> ()
      end
    | None -> ()
end

module Env = Tast_env

let handler = object
  inherit Tast_visitor.handler_base

  method! at_fun_def = visitor#on_fun_
  method! at_method_ = visitor#on_method_
  method! at_fun_ = visitor#on_fun_
end
