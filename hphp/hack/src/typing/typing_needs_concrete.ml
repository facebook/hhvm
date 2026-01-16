(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let check_class_get
    (env : Typing_env_types.env)
    (class_get_pos : Pos.t)
    (def_pos : Pos_or_decl.t)
    (cid : string)
    (mid : string)
    (ce : Typing_defs.class_elt)
    (e : ('ex, 'en) Aast_defs.class_id_)
    (is_method : bool) : unit =
  if TypecheckerOptions.needs_concrete env.genv.tcopt then
    let callee_is_needs_concrete_method : bool =
      is_method && Typing_defs.get_ce_readonly_prop_or_needs_concrete ce
    in
    let check_needs_concrete_call (via : [ `Static | `Self | `Parent ]) : unit =
      (* `self` and `parent` forward the referent of `static` so are just as dangerous *)
      if
        callee_is_needs_concrete_method
        && not (Typing_env.static_points_to_concrete_class env)
      then
        Typing_warning_utils.add
          env
          ( class_get_pos,
            Typing_warning.Call_needs_concrete,
            {
              Typing_warning.Call_needs_concrete.call_pos = class_get_pos;
              class_name = cid;
              meth_name = mid;
              decl_pos = def_pos;
              via = (via :> [ `Id | `Static | `Self | `Parent ]);
            } )
    in
    begin
      match e with
      | CI _ when callee_is_needs_concrete_method ->
        Typing_env.get_class env cid
        |> Decl_entry.to_option
        |> Option.iter ~f:(fun (class_ : Decl_provider.class_decl) ->
               let is_concrete : bool =
                 let is_non_abstract : bool =
                   not (Folded_class.abstract class_)
                 in
                 let is_final_non_consistent_construct =
                   lazy
                     (match snd @@ Typing_env.get_construct env class_ with
                     | Typing_defs.FinalClass -> true
                     | Typing_defs.Inconsistent
                     | Typing_defs.ConsistentConstruct ->
                       false)
                 in
                 is_non_abstract
                 || Folded_class.final class_
                    && Lazy.force is_final_non_consistent_construct
               in
               if not is_concrete then
                 Typing_warning_utils.add
                   env
                   ( class_get_pos,
                     Typing_warning.Call_needs_concrete,
                     {
                       Typing_warning.Call_needs_concrete.call_pos =
                         class_get_pos;
                       class_name = cid;
                       meth_name = mid;
                       decl_pos = def_pos;
                       via = `Id;
                     } ))
      | CIself -> check_needs_concrete_call `Self
      | CIparent -> check_needs_concrete_call `Parent
      | CIstatic ->
        let () = check_needs_concrete_call `Static in
        if
          Typing_defs.get_ce_abstract ce
          && not (Typing_env.static_points_to_concrete_class env)
        then
          (* We check for abstract access via `static`
           * as part of the "needs concrete" feature, because
           * checking for calls to `abstract` functions for
           * `self`/`parent`/classname, etc. is already covered by other type
           * errors such as Primary.Self_abstract_call, Primary.Parent_abstract_call, etc.
           *)
          Typing_warning_utils.add
            env
            ( class_get_pos,
              Typing_warning.Abstract_access_via_static,
              {
                Typing_warning.Abstract_access_via_static.access_pos =
                  class_get_pos;
                class_name = cid;
                member_name = mid;
                decl_pos = def_pos;
                containing_method_pos = Some env.genv.function_pos;
              } )
      | CI _ -> ()
      | CIexpr _ -> ()
    end

let check_instantiation
    (env : Typing_env_types.env)
    (instantiation_pos : Pos.t)
    (cid : ('ex, 'en) Aast_defs.class_id_) : unit =
  if TypecheckerOptions.needs_concrete env.genv.tcopt then
    match cid with
    | CIstatic when not (Typing_env.static_points_to_concrete_class env) ->
      Typing_env.get_self_class env
      |> Decl_entry.to_option
      |> Option.iter ~f:(fun (class_ : Decl_provider.class_decl) ->
             let would_be_redundant =
               (* Elsewhere we already generate a 4002 (uninstantiable class)
                * error for `new static(....)` in `abstract final` classes
                * https://www.internalfb.com/code/fbsource/[0032e6cacac2bab09425384b967f12d8d4743daf]/fbcode/hphp/hack/src/typing/typing.ml?lines=4226%2C11972
                *)
               Folded_class.abstract class_ && Folded_class.final class_
             in
             if not would_be_redundant then
               Typing_warning_utils.add
                 env
                 ( instantiation_pos,
                   Typing_warning.Uninstantiable_class_via_static,
                   {
                     Typing_warning.Uninstantiable_class_via_static.usage_pos =
                       instantiation_pos;
                     class_name = Folded_class.name class_;
                     decl_pos = Folded_class.pos class_;
                   } ))
    | CIstatic
    | CIself
    | CIparent
    | CI _
    | CIexpr _ ->
      ()
