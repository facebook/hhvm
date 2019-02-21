(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)


(*
  Add env here so we're not dependent on removing
  tenv from nastCheck just yet.
  - We can add fields as we need them to the
  Nast_env we've created here.
*)

type env = {
  def_type: string;
}

let fun_env _ =
  { def_type = "fun" }

let def_env x =
  match x with
  | Nast.Fun x -> fun_env x
  | Nast.Class _ -> { def_type = "class" }
  | Nast.Typedef _ -> { def_type = "typedef" }
  | Nast.Constant _
  | Nast.Stmt _
  | Nast.Namespace _
  | Nast.NamespaceUse _
  | Nast.SetNamespaceEnv _ -> { def_type = "" }

open Core_kernel

class virtual iter = object (self)
  inherit [_] Nast.iter as super

  (* Entry point *)
  method go program = self#on_list (fun () -> self#go_def) () program

  method go_def x = self#on_def (def_env x) x

  method! on_fun_ _env x = super#on_fun_ (fun_env x) x
end


class type handler = object

  method at_fun_ : env -> Nast.fun_ -> unit
  method at_class_ : env -> Nast.class_ -> unit
end

class virtual handler_base : handler = object

  method at_fun_ _ _ = ()
  method at_class_ _ _ = ()
end

let iter_with (handlers : handler list) : iter = object

  inherit iter as super

  method! on_fun_ env x =
    List.iter handlers (fun v -> v#at_fun_ env x);
    super#on_fun_ env x;

  method! on_class_ env x =
    List.iter handlers (fun v -> v#at_class_ env x);
    super#on_class_ env x;

end
