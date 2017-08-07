(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open SymbolOccurrence
open Typing_defs

type single_result = (string SymbolOccurrence.t) * (string SymbolDefinition.t option)
type result = single_result list

let result_to_ide_message x =
  let open Ide_message in
  Identify_symbol_response (
    List.map x begin fun (occurrence, definition) ->
      {occurrence; definition}
    end
  )

let is_target target_line target_char pos =
  let l, start, end_ = Pos.info_pos pos in
  (* We need to filter out by filename too, due to lazy decl firing hooks in
   * unrelated files *)
  (Pos.filename pos = ServerIdeUtils.path) &&
  l = target_line && start <= target_char && target_char - 1 <= end_

let append_result result_ref result =
  if not (List.mem !result_ref result) then result_ref := result :: !result_ref

let process_class_id result_ref is_target_fun cid _ =
  if is_target_fun (fst cid)
  then begin
    let name = snd cid in
    append_result result_ref @@
      {
        name;
        type_ = Class;
        pos   = fst cid
      }
  end

let clean_member_name name = String_utils.lstrip name "$"

let process_member result_ref is_target_fun c_name id ~is_method ~is_const =
  if is_target_fun (fst id)
  then begin
    let member_name = (snd id) in
    let type_ =
      if is_const then ClassConst (c_name, member_name)
      else if is_method then Method (c_name, member_name)
      else Property (c_name, member_name)
    in
    append_result result_ref @@
           { name  = (c_name ^ "::" ^ (clean_member_name member_name));
             type_;
             pos   = fst id
           }
  end

let process_method_id result_ref is_target_fun class_ ~targs:_ id _ _
    ~is_method ~is_const =
  let class_name = class_.Typing_defs.tc_name in
  process_member result_ref is_target_fun class_name id is_method is_const

let process_constructor result_ref is_target_fun class_ ~targs _ p =
  process_method_id
    result_ref is_target_fun
      class_ ~targs (p, Naming_special_names.Members.__construct)
        () () ~is_method:true ~is_const:false

let process_fun_id result_ref is_target_fun id =
  if is_target_fun (fst id)
  then begin
    let name = snd id in
    append_result result_ref @@
           { name;
             type_ = Function;
             pos   = fst id
           }
  end

let process_global_const result_ref is_target_fun id =
  if is_target_fun (fst id)
  then begin
    let name = snd id in
    append_result result_ref @@
           { name;
             type_ = GConst;
             pos   = fst id
           }
  end

let process_lvar_id result_ref is_target_fun _ id _ =
  if is_target_fun (fst id)
  then begin
    append_result result_ref @@
                       { name  = snd id;
                         type_ = LocalVar;
                         pos   = fst id
                       }
  end

let process_typeconst result_ref is_target_fun class_name tconst_name pos =
  if (is_target_fun pos) then begin
    append_result result_ref @@
           { name = class_name ^ "::" ^ tconst_name;
             type_ = Typeconst (class_name, tconst_name);
             pos;
           }
  end

let process_taccess result_ref is_target_fun class_ typeconst pos =
    let class_name = class_.tc_name in
    let tconst_name = (snd typeconst.ttc_name) in
    process_typeconst result_ref is_target_fun class_name tconst_name pos

let process_named_class result_ref is_target_fun class_ =
  process_class_id result_ref is_target_fun class_.Nast.c_name ();
  let c_name = snd class_.Nast.c_name in
  let all_methods = class_.Nast.c_methods @ class_.Nast.c_static_methods in
  List.iter all_methods begin fun method_ ->
    process_member result_ref is_target_fun
      c_name method_.Nast.m_name ~is_method:true ~is_const:false
  end;
  let all_props = class_.Nast.c_vars @ class_.Nast.c_static_vars in
  List.iter all_props begin fun prop ->
    process_member result_ref is_target_fun
      c_name prop.Nast.cv_id ~is_method:false ~is_const:false
  end;
  List.iter class_.Nast.c_consts begin fun (_, const_id, _) ->
    process_member result_ref is_target_fun
      c_name const_id ~is_method:false ~is_const:true
  end;
  List.iter class_.Nast.c_typeconsts begin fun typeconst ->
    process_typeconst result_ref is_target_fun c_name
      (snd typeconst.Nast.c_tconst_name) (fst typeconst.Nast.c_tconst_name)
  end;
  (* We don't check anything about xhp attributes, so the hooks won't fire when
     typechecking the class. Need to look at them individually. *)
  List.iter class_.Nast.c_xhp_attr_uses begin function
    | _, Nast.Happly (cid, _) ->
      process_class_id result_ref is_target_fun cid ()
    | _ -> ()
  end;
  match class_.Nast.c_constructor with
    | Some method_ ->
      let id =
        fst method_.Nast.m_name, Naming_special_names.Members.__construct
      in
      process_member result_ref is_target_fun
        c_name id ~is_method:true ~is_const:false
    | None -> ()

let process_named_fun result_ref is_target_fun fun_ =
  process_fun_id result_ref is_target_fun fun_.Nast.f_name

let really_attach_hooks result_ref is_target_fun =
  let process_method_id = process_method_id result_ref is_target_fun in
  Typing_hooks.attach_cmethod_hook process_method_id;
  Typing_hooks.attach_smethod_hook process_method_id;
  Typing_hooks.attach_constructor_hook
    (process_constructor result_ref is_target_fun);
  Typing_hooks.attach_fun_id_hook (process_fun_id result_ref is_target_fun);
  Typing_hooks.attach_global_const_hook
    (process_global_const result_ref is_target_fun);
  Typing_hooks.attach_taccess_hook (process_taccess result_ref is_target_fun);
  Decl_hooks.attach_class_id_hook (process_class_id result_ref is_target_fun);
  Naming_hooks.attach_lvar_hook (process_lvar_id result_ref is_target_fun);
  Naming_hooks.attach_class_named_hook
    (process_named_class result_ref is_target_fun);
  Naming_hooks.attach_fun_named_hook
    (process_named_fun result_ref is_target_fun)

let attach_hooks result_ref line char =
  let is_target_fun = is_target line char in
  really_attach_hooks result_ref is_target_fun

let detach_hooks () =
  Naming_hooks.remove_all_hooks ();
  Decl_hooks.remove_all_hooks ();
  Typing_hooks.remove_all_hooks ();
  ()
