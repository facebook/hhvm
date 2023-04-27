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

let on_class_ c ~ctx = (Env.set_in_class ctx ~in_class:true, Ok c)

(* The attributes applied to a class exist outside the current class so
   references to `self` are invalid *)
let on_class_c_user_attributes c_user_attributes ~ctx =
  (Env.set_in_class ctx ~in_class:false, Ok c_user_attributes)

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
let on_class_id on_error class_id ~ctx =
  let in_class = Env.in_class ctx in
  let (class_id, err_opt) =
    match class_id with
    (* TODO[mjt] if we don't expect these from lowering should we refine the
       NAST repr? *)
    | (_, _, Aast.(CIparent | CIself | CIstatic | CI _)) -> (class_id, None)
    | (annot, _, Aast.(CIexpr (_, expr_pos, Id (id_pos, cname)))) ->
      if String.equal cname SN.Classes.cParent then
        if not in_class then
          ( (annot, expr_pos, Aast.CI (expr_pos, SN.Classes.cUnknown)),
            Some (Err.naming @@ Naming_error.Parent_outside_class id_pos) )
        else
          ((annot, expr_pos, Aast.CIparent), None)
      else if String.equal cname SN.Classes.cSelf then
        if not in_class then
          ( (annot, expr_pos, Aast.CI (expr_pos, SN.Classes.cUnknown)),
            Some (Err.naming @@ Naming_error.Self_outside_class id_pos) )
        else
          ((annot, expr_pos, Aast.CIself), None)
      else if String.equal cname SN.Classes.cStatic then
        if not in_class then
          ( (annot, expr_pos, Aast.CI (expr_pos, SN.Classes.cUnknown)),
            Some (Err.naming @@ Naming_error.Static_outside_class id_pos) )
        else
          ((annot, expr_pos, Aast.CIstatic), None)
      else
        ((annot, expr_pos, Aast.CI (expr_pos, cname)), None)
    | (annot, _, Aast.(CIexpr (ci_annot, expr_pos, Lvar (lid_pos, lid))))
      when String.equal (Local_id.to_string lid) SN.SpecialIdents.this ->
      (* TODO[mjt] why is `$this` valid outside a class? *)
      (Aast.(annot, expr_pos, CIexpr (ci_annot, lid_pos, This)), None)
    | (annot, _, (Aast.(CIexpr (_, expr_pos, _)) as class_id_)) ->
      ((annot, expr_pos, class_id_), None)
  in
  Option.iter ~f:on_error err_opt;
  (ctx, Ok class_id)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_class_ = Some on_class_;
        on_fld_class__c_user_attributes = Some on_class_c_user_attributes;
        on_ty_class_id = Some (fun elem ~ctx -> on_class_id on_error elem ~ctx);
      }
