(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Aast

let on_user_attributes on_error us ~ctx =
  (* Complain on duplicates, e.g. <<Foo, Bar, Foo>>. *)
  let seen = Stdlib.Hashtbl.create 0 in
  let dedup (attrs, err_acc) (Aast.{ ua_name = (pos, attr_name); _ } as attr) =
    match Stdlib.Hashtbl.find_opt seen attr_name with
    | Some prev_pos ->
      let err =
        Naming_phase_error.naming
        @@ Naming_error.Duplicate_user_attribute { pos; prev_pos; attr_name }
      in
      (attrs, err :: err_acc)
    | _ ->
      Stdlib.Hashtbl.add seen attr_name pos;
      (attr :: attrs, err_acc)
  in
  let (us, errs) =
    Tuple2.map_fst ~f:List.rev @@ List.fold_left us ~init:([], []) ~f:dedup
  in
  List.iter ~f:on_error errs;

  (* Fully qualify arguments to __Memoize with enum class syntax.
     E.g. <<__Memoize(#KeyedByIC)>> becomes <<__Memoize(\HH\MemoizeOption#KeyedByIC)>> *)
  let us =
    List.map
      ~f:(fun ({ ua_name = (_, attr_name); ua_params } as attr) ->
        if
          String.equal attr_name Naming_special_names.UserAttributes.uaMemoize
          || String.equal
               attr_name
               Naming_special_names.UserAttributes.uaMemoizeLSB
        then
          let ua_params =
            List.map ua_params ~f:(fun e ->
                let (ex, pos, e_) = e in
                let e_ =
                  match e_ with
                  | EnumClassLabel (None, label) ->
                    EnumClassLabel
                      ( Some (Pos.none, Naming_special_names.HH.memoizeOption),
                        label )
                  | EnumClassLabel (Some cid, label) ->
                    let (class_pos, class_name) = cid in
                    if
                      not
                        (String.equal
                           class_name
                           Naming_special_names.HH.memoizeOption)
                    then
                      on_error
                        (Naming_phase_error.naming
                        @@ Naming_error.Invalid_memoize_label
                             { pos = class_pos; attr_name });
                    EnumClassLabel (Some cid, label)
                  | _ -> e_
                in
                (ex, pos, e_))
          in
          { attr with ua_params }
        else
          attr)
      us
  in

  (ctx, Ok us)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_user_attributes =
          Some (fun elem ~ctx -> on_user_attributes on_error elem ~ctx);
      }
