(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type vars = {
  bound: Pos.t Local_id.Map.t;
  free: Pos.t Local_id.Map.t;
}

let empty = { bound = Local_id.Map.empty; free = Local_id.Map.empty }

let add_local_def vars (pos, lid) : vars =
  { vars with bound = Local_id.Map.add lid pos vars.bound }

let add_local_defs vars lvals : vars =
  List.fold lvals ~init:vars ~f:(fun acc ((), lval) -> add_local_def acc lval)

let add_param vars param : vars =
  let lid = Local_id.make_unscoped param.Aast.param_name in
  add_local_def vars (param.Aast.param_pos, lid)

let add_params vars f : vars = List.fold f.Aast.f_params ~init:vars ~f:add_param

let lvalues (e : Nast.expr) : Nast.capture_lid list =
  let rec aux acc (_, _, e) =
    match e with
    | Aast.List lv -> List.fold_left ~init:acc ~f:aux lv
    | Aast.Lvar (pos, lid) -> ((), (pos, lid)) :: acc
    | _ -> acc
  in
  aux [] e

let add_local_defs_from_lvalue vars e : vars =
  List.fold (lvalues e) ~init:vars ~f:(fun acc ((), lval) ->
      add_local_def acc lval)

let add_local_ref vars (pos, lid) : vars =
  if Local_id.Map.mem lid vars.bound then
    vars
  else
    { vars with free = Local_id.Map.add lid pos vars.free }

let vars = ref empty

(* Walk this AAST, track free variables, and add them to capture lists in
   lambdas.

   A free variable is any local that isn't bound as a parameter or directly
   defined.

   ($a) ==> {
     $b = $a;
     $c;
   }

   In this example, only $c is free. *)
let visitor =
  object
    inherit [_] Aast.endo as super

    method on_'ex () () = ()

    method on_'en () () = ()

    method! on_Lvar () e lv =
      vars := add_local_ref !vars lv;
      super#on_Lvar () e lv

    method! on_Binop () e (Aast.{ bop; lhs; _ } as binop) =
      (match bop with
      | Ast_defs.Eq None ->
        (* Introducing a new local variable.

           $x = ... *)
        vars := add_local_defs_from_lvalue !vars lhs
      | _ -> ());
      super#on_Binop () e binop

    method! on_as_expr () ae =
      (* [as] inside a foreach loop introduces a new local variable.

         foreach(... as $x) { ... } *)
      (match ae with
      | Aast.As_v e
      | Aast.Await_as_v (_, e) ->
        vars := add_local_defs_from_lvalue !vars e
      | Aast.As_kv (k, v)
      | Aast.Await_as_kv (_, k, v) ->
        vars := add_local_defs_from_lvalue !vars k;
        vars := add_local_defs_from_lvalue !vars v);

      super#on_as_expr () ae

    method! on_Awaitall () s el block =
      (* [concurrent] blocks are desugared to a list of expressions,
         which can introduce new locals.

         concurrent {
           $x = await foo();
           await bar();
         } *)
      List.iter el ~f:(fun (lv, _) -> vars := add_local_def !vars lv);
      super#on_Awaitall () s el block

    method! on_catch () (c_name, lv, block) =
      (* [catch] introduces a new local variable.

         try { ... } catch (Foo $x) { ... } *)
      vars := add_local_def !vars lv;
      super#on_catch () (c_name, lv, block)

    method! on_Efun () e efun =
      let outer_vars = !vars in
      let idl = efun.Aast.ef_use in

      (* We want to know about free variables inside the lambda, but
         we don't want its bound variables. *)
      vars := add_params empty efun.Aast.ef_fun;
      vars := add_local_defs !vars idl;
      let efun =
        match super#on_Efun () e efun with
        | Aast.Efun efun -> efun
        | _ -> assert false
      in
      vars :=
        { outer_vars with free = Local_id.Map.union outer_vars.free !vars.free };

      (* Efun syntax requires that the user specifies the captures.

         function() use($captured1, $captured2) { ... }

         We just check that they haven't tried to explicitly capture
         $this. *)
      let idl =
        List.filter idl ~f:(fun (_, (p, lid)) ->
            if
              String.equal
                (Local_id.to_string lid)
                Naming_special_names.SpecialIdents.this
            then (
              Errors.add_error
                Naming_error.(to_user_error @@ This_as_lexical_variable p);
              false
            ) else
              true)
      in
      Aast.Efun { efun with Aast.ef_use = idl }

    method! on_Lfun () e f _ =
      let outer_vars = !vars in

      (* We want to know about free variables inside the lambda, but
         we don't want its bound variables. *)
      vars := add_params empty f;
      let f =
        match super#on_Lfun () e f [] with
        | Aast.Lfun (f, _) -> f
        | _ -> assert false
      in
      let idl =
        Local_id.Map.fold
          (fun lid pos acc -> ((), (pos, lid)) :: acc)
          !vars.free
          []
      in
      vars :=
        { outer_vars with free = Local_id.Map.union outer_vars.free !vars.free };

      Aast.Lfun (f, idl)
  end

(* Populate the capture list for all lambdas occurring in this
   top-level definition.

   ($x) ==> $x + $y; // $y is captured here
*)

let elab f elem =
  vars := empty;
  let elem = f () elem in
  vars := empty;
  elem

let elab_fun_def elem = elab visitor#on_fun_def elem

let elab_typedef elem = elab visitor#on_typedef elem

let elab_module_def elem = elab visitor#on_module_def elem

let elab_gconst elem = elab visitor#on_gconst elem

let elab_class elem = elab visitor#on_class_ elem

let elab_program elem = elab visitor#on_program elem

let populate_fun_def (fd : Nast.fun_def) : Nast.fun_def = elab_fun_def fd

let populate_class_ (c : Nast.class_) : Nast.class_ = elab_class c
