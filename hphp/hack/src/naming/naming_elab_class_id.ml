(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error
module SN = Naming_special_names

module Env = struct
  let set_in_class t ~in_class =
    Naming_phase_env.{ t with elab_class_id = Elab_class_id.{ in_class } }

  let in_class
      Naming_phase_env.{ elab_class_id = Elab_class_id.{ in_class }; _ } =
    in_class
end

let on_class_ (env, c, err) =
  Naming_phase_pass.Cont.next (Env.set_in_class env ~in_class:true, c, err)

(* The attributes applied to a class exist outside the current class so
   references to `self` are invalid *)
let on_class_c_user_attributes (env, c_user_attributes, err_acc) =
  Naming_phase_pass.Cont.next
    (Env.set_in_class env ~in_class:false, c_user_attributes, err_acc)

(* The lowerer will give us CIexpr (Id  _ | Lvar _ ); here we:
      - convert CIexpr(_,_,Id _) to CIparent, CIself, CIstatic and CI.
      - convert CIexpr(_,_,Lvar $this) to CIexpr(_,_,This)

      If there is a CIexpr with anything other than an Lvar or This after this
      elaboration step, it is an error and will be raised in subsequent
      validation passes

      TODO[mjt] We're overriding `on_class` rather than `on_class_` since
      the legacy code mangles positions by using the inner `class_id_` position
      in the output `class_id` tuple. This looks to be erroneous.
*)
let on_class_id (env, class_id, err_acc) =
  let in_class = Env.in_class env in
  let (class_id, err_acc) =
    match class_id with
    (* TODO[mjt] if we don't expect these from lowering should we refine the
       NAST repr? *)
    | (_, _, Aast.(CIparent | CIself | CIstatic | CI _)) -> (class_id, err_acc)
    | (_, _, Aast.(CIexpr (_, expr_pos, Id (id_pos, cname)))) ->
      if String.equal cname SN.Classes.cParent then
        if not in_class then
          ( ((), expr_pos, Aast.CI (expr_pos, SN.Classes.cUnknown)),
            (Err.typing @@ Typing_error.Primary.Parent_outside_class id_pos)
            :: err_acc )
        else
          (((), expr_pos, Aast.CIparent), err_acc)
      else if String.equal cname SN.Classes.cSelf then
        if not in_class then
          ( ((), expr_pos, Aast.CI (expr_pos, SN.Classes.cUnknown)),
            (Err.typing @@ Typing_error.Primary.Self_outside_class id_pos)
            :: err_acc )
        else
          (((), expr_pos, Aast.CIself), err_acc)
      else if String.equal cname SN.Classes.cStatic then
        if not in_class then
          ( ((), expr_pos, Aast.CI (expr_pos, SN.Classes.cUnknown)),
            (Err.typing @@ Typing_error.Primary.Static_outside_class id_pos)
            :: err_acc )
        else
          (((), expr_pos, Aast.CIstatic), err_acc)
      else
        (((), expr_pos, Aast.CI (expr_pos, cname)), err_acc)
    | (_, _, Aast.(CIexpr (_, expr_pos, Lvar (lid_pos, lid))))
      when String.equal (Local_id.to_string lid) SN.SpecialIdents.this ->
      (* TODO[mjt] why is `$this` valid outside a class? *)
      (Aast.((), expr_pos, CIexpr ((), lid_pos, This)), err_acc)
    | (_, _, (Aast.(CIexpr (_, expr_pos, _)) as class_id_)) ->
      (((), expr_pos, class_id_), err_acc)
  in
  Naming_phase_pass.Cont.next (env, class_id, err_acc)

let pass =
  Naming_phase_pass.(
    top_down
      {
        identity with
        on_class_ = Some on_class_;
        on_class_c_user_attributes = Some on_class_c_user_attributes;
        on_class_id = Some on_class_id;
      })
