(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Converts a type hint into a type  *)
(*****************************************************************************)
open Core_kernel
open Nast
open Typing_defs

module Env = Typing_env
module Cls = Typing_classes_heap

(*****************************************************************************)
(* The signature of the visitor. *)
(*****************************************************************************)

class type ['a] hint_visitor_type = object
  method on_hint   : 'a -> Nast.hint -> 'a
  method on_hint_  : 'a -> Nast.hint_ -> 'a

  method on_any    : 'a -> 'a
  method on_dynamic  : 'a -> 'a
  method on_mixed  : 'a -> 'a
  method on_nonnull : 'a -> 'a
  method on_this   : 'a -> 'a
  method on_tuple  : 'a -> Nast.hint list -> 'a
  method on_abstr  : 'a -> string -> 'a
  method on_array  : 'a -> Nast.hint option -> Nast.hint option -> 'a
  method on_prim   : 'a -> Nast.tprim -> 'a
  method on_option : 'a -> Nast.hint -> 'a
  method on_param_kind : 'a -> Ast.param_kind -> 'a
  method on_fun    : 'a ->
                     Nast.func_reactive ->
                     bool ->
                     Nast.hint list ->
                     Ast.param_kind option list ->
                     Nast.param_mutability option list ->
                     Nast.variadic_hint ->
                     Nast.hint ->
                     bool ->
                     'a
  method on_apply  : 'a -> Nast.sid -> Nast.hint list -> 'a
  method on_shape  : 'a -> nast_shape_info -> 'a
  method on_access : 'a -> Nast.hint -> Nast.sid list -> 'a
  method on_soft   : 'a -> Nast.hint -> 'a
  method on_reified : 'a -> Nast.hint -> 'a
end

(*****************************************************************************)
(* The generic visitor ('a is the type of the accumulator). *)
(*****************************************************************************)

class virtual ['a] hint_visitor: ['a] hint_visitor_type = object(this)

  method on_hint acc (_, h) = this#on_hint_ acc h

  method on_hint_ acc h = match h with
    | Hany                  -> this#on_any    acc
    | Hdynamic              -> this#on_dynamic acc
    | Hmixed                -> this#on_mixed  acc
    | Hnonnull              -> this#on_nonnull acc
    | Hthis                 -> this#on_this   acc
    | Htuple hl             -> this#on_tuple  acc (hl:Nast.hint list)
    | Habstr x              -> this#on_abstr  acc x
    | Harray (hopt1, hopt2) -> this#on_array  acc hopt1 hopt2
    | Hdarray (h1, h2)      -> this#on_array  acc (Some h1) (Some h2)
    | Hvarray_or_darray h
    | Hvarray h             -> this#on_array  acc (Some h) None
    | Hprim p               -> this#on_prim   acc p
    | Hoption h             -> this#on_option acc h
    | Hfun (is_r, is_c, hl, kl, mut, b, h, mut_ret)  -> this#on_fun acc is_r is_c hl kl mut b h mut_ret
    | Happly (i, hl)        -> this#on_apply  acc i hl
    | Hshape hm             -> this#on_shape  acc hm
    | Haccess (h, il)       -> this#on_access acc h il
    | Hsoft h               -> this#on_soft   acc h
    | Hreified h            -> this#on_reified acc h

  method on_any acc = acc
  method on_mixed acc = acc
  method on_nonnull acc = acc
  method on_dynamic acc = acc
  method on_this acc = acc
  method on_tuple acc hl =
    List.fold_left ~f:this#on_hint ~init:acc (hl:Nast.hint list)

  method on_abstr acc _ = acc

  method on_array acc hopt1 hopt2 =
    let acc = match hopt1 with
      | None -> acc
      | Some h -> this#on_hint acc h
    in
    let acc = match hopt2 with
      | None -> acc
      | Some h -> this#on_hint acc h
    in
    acc

  method on_prim acc _ = acc

  method on_option acc h = this#on_hint acc h

  method on_param_kind acc _ = acc

  method on_fun acc _ _ hl kl _ _ h _ =
    let acc = List.fold_left ~f:this#on_hint ~init:acc hl in
    let acc = List.fold_left ~f:(fun acc k ->
      match k with
      | Some kind -> this#on_param_kind acc kind
      | None -> acc
    ) ~init:acc kl in
    let acc = this#on_hint acc h in
    acc

  method on_apply acc _ (hl:Nast.hint list) =
    let acc = List.fold_left ~f:this#on_hint ~init:acc hl in
    acc

  method on_shape acc { nsi_allows_unknown_fields=_; nsi_field_map } =
    ShapeMap.fold begin fun _ shape_field_info acc ->
      let acc = this#on_hint acc shape_field_info.sfi_hint in
      acc
    end nsi_field_map acc

  method on_access acc h _ =
    this#on_hint acc h

  method on_soft acc h =
    this#on_hint acc h

  method on_reified acc h =
    this#on_hint acc h

end

(* For checking whether hint refers to a construct that cannot ever have
 * instances. To avoid race conditions, only look up class info after the decl
 * phase is complete. *)
module CheckInstantiability = struct

  let validate_classname = function
    | _,
      (
        Happly _
        | Hthis
        | Hany
        | Hmixed
        | Hnonnull
        | Habstr _
        | Haccess _
        | Hdynamic
        | Hsoft _
        | Hreified _
      ) -> ()
    | p,
      (
        Htuple _
        | Harray _
        | Hdarray _
        | Hvarray _
        | Hvarray_or_darray _
        | Hprim _
        | Hoption _
        | Hfun _
        | Hshape _
      ) ->
        Errors.invalid_classname p

  let visitor env = object(this)
    inherit [unit] hint_visitor as super

    (* A type constant may appear inside an abstract final class, i.e.
     *
     *   abstract final class Foo { const type T = int; }
     *
     * Even though the type `Foo` is not instantiable, the type `Foo::T` is.
     * Thus for type checking the Taccess type, we skip the instantiability
     * for the root of the Taccess.
     *)
    method! on_access () h ids = match h with
      | _, Happly (_, hl) ->
        List.iter hl (this#on_hint ())
      | _ -> super#on_access () h ids

    method! on_apply () (usage_pos, n) hl =
      let () = (match Env.get_class env n with
        | Some cls
          when
            let kind = Cls.kind cls in
            let tc_name = Cls.name cls in
            (kind = Ast.Ctrait || kind = Ast.Cabstract && Cls.final cls)
            && tc_name <> SN.Collections.cDict
            && tc_name <> SN.Collections.cKeyset
            && tc_name <> SN.Collections.cVec ->
          let tc_pos = Cls.pos cls in
          let tc_name = Cls.name cls in
          Errors.uninstantiable_class usage_pos tc_pos tc_name []
        | _ -> ()) in
      if n = SN.Classes.cClassname
      then Option.iter (List.hd hl) validate_classname
      else super#on_apply () (usage_pos, n) hl

    method! on_abstr () _ =
      (* there should be no need to descend into abstract params, as
       * the necessary param checks happen on the declaration of the
       * constraint *)
      ()

  end

  let check env h = (visitor env)#on_hint () h

end

let check_instantiable (env:Env.env) (h:Nast.hint) =
  CheckInstantiability.check env h

let check_param_instantiable (env:Env.env) (param:Nast.fun_param)=
  match param.param_hint with
    | None -> ()
    | Some h -> check_instantiable env h

let check_params_instantiable (env:Env.env) (params:Nast.fun_param list)=
  List.iter params ~f:(check_param_instantiable env)

let check_tparams_instantiable (env:Env.env) (tparams:Nast.tparam list) =
  List.iter tparams ~f:begin fun t ->
    List.iter t.tp_constraints ~f:begin fun (_ck, h) -> check_instantiable env h end
  end

let instantiable_hint env h =
  check_instantiable env h;
  Decl_hint.hint env.Env.decl_env h
