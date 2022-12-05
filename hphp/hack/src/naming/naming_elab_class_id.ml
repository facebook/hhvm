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
  type t = bool

  let empty = false
end

let visitor =
  object (self)
    inherit [_] Naming_visitors.mapreduce as super

    method! on_class_ _ c =
      let env = true in
      super#on_class_ env c

    method! on_class_c_user_attributes _ c_user_attributes =
      (* The attributes applied to a class exist outside the current class so
         references to `self` are invalid *)
      let env = false in
      super#on_class_c_user_attributes env c_user_attributes

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
    method! on_class_id in_class class_id =
      match class_id with
      (* TODO[mjt] if we don't expect these from lowering should we refine the
         NAST repr? *)
      | (_, _, Aast.(CIparent | CIself | CIstatic | CI _)) ->
        failwith "Error in Ast_to_nast module for Class_get"
      | (_, _, Aast.(CIexpr (_, expr_pos, Id (id_pos, cname)))) ->
        if String.equal cname SN.Classes.cParent then
          if not in_class then
            ( ((), expr_pos, Aast.CI (expr_pos, SN.Classes.cUnknown)),
              Err.typing @@ Typing_error.Primary.Parent_outside_class id_pos )
          else
            (((), expr_pos, Aast.CIparent), self#zero)
        else if String.equal cname SN.Classes.cSelf then
          if not in_class then
            ( ((), expr_pos, Aast.CI (expr_pos, SN.Classes.cUnknown)),
              Err.typing @@ Typing_error.Primary.Self_outside_class id_pos )
          else
            (((), expr_pos, Aast.CIself), self#zero)
        else if String.equal cname SN.Classes.cStatic then
          if not in_class then
            ( ((), expr_pos, Aast.CI (expr_pos, SN.Classes.cUnknown)),
              Err.typing @@ Typing_error.Primary.Static_outside_class id_pos )
          else
            (((), expr_pos, Aast.CIstatic), self#zero)
        else
          (((), expr_pos, Aast.CI (expr_pos, cname)), self#zero)
      | (_, _, Aast.(CIexpr (_, expr_pos, Lvar (lid_pos, lid))))
        when String.equal (Local_id.to_string lid) SN.SpecialIdents.this ->
        (* TODO[mjt] why is `$this` valid outside a class? *)
        (Aast.((), expr_pos, CIexpr ((), lid_pos, This)), self#zero)
      | (_, _, (Aast.(CIexpr (_, expr_pos, _)) as class_id_)) ->
        (((), expr_pos, class_id_), self#zero)
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
