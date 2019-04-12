(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel
open Facts

module SU = Hhbc_string_utils
module FSC = Facts_smart_constructors

module FactsParser_ = Full_fidelity_parser
                      .WithSyntax(Full_fidelity_positioned_syntax)
module FactsParser = FactsParser_
                     .WithSmartConstructors(FSC.SC)

let flags_default = 0
let flags_abstract = 1
let flags_final = 2
let flags_multiple_declarations = 4

let set_flag flags flag = flags lor flag

let has_multiple_declarations_flag tf =
  (tf.flags land flags_multiple_declarations) <> 0

let combine_flags tf flags =
  { tf with flags = set_flag tf.flags flags }

let set_multiple_declarations_flag tf =
  combine_flags tf flags_multiple_declarations

let qualified_name_from_parts ns l =
  let open FSC in
  let rec aux l acc =
    match l with
    | [] -> Some (String.concat ~sep:"" @@ List.rev acc)
    | Name n :: xs -> aux xs (n () :: acc)
    (* ignore leading backslash *)
    | Backslash :: xs -> aux xs acc
    | ListItem (Name n, Backslash) :: xs -> aux xs ( (n () ^ "\\") :: acc)
    | _ -> None in
  match aux l [], l with
  (* globally qualified name *)
  | Some n, Backslash :: _ -> Some n
  | Some n, _ -> if ns = "" then Some n else Some (ns ^ "\\" ^ n)
  | _ -> None

let qualified_name ns name =
  let open FSC in
  match name with
  | Name n ->
    (* always simple name *)
    let n = n () in
    if ns = "" then Some n
    else Some (ns ^ "\\" ^ n)
  | XhpName n ->
    (* xhp names are always unqualified *)
    Some (SU.Xhp.mangle_id @@ n ())
  | QualifiedName l -> qualified_name_from_parts ns l
  | _ -> None

let flags_from_modifiers modifiers =
  let open FSC in
  let rec aux l f =
    match l with
    | [] -> f
    | Abstract :: xs ->
      aux xs (set_flag f flags_abstract)
    | Final :: xs ->
      aux xs (set_flag f flags_final)
    | Static :: xs ->
      aux xs (set_flag f @@ set_flag flags_final flags_abstract)
    | _ :: xs ->
      aux xs f in
  match modifiers with
  | List l -> aux l 0
  | _ -> 0

let add_or_update_classish_declaration types name kind flags base_types
    require_extends require_implements =
  match InvSMap.get name types with
  | Some old_tf ->
    let tf =
      if old_tf.kind <> kind then { old_tf with kind = TKMixed }
      else old_tf in
    let tf =
      if not @@ has_multiple_declarations_flag tf
      then set_multiple_declarations_flag tf
      else tf in
    let tf =
      if tf.flags = flags then tf
      else combine_flags tf flags in
    let tf =
      if InvSSet.is_empty base_types
      then tf
      else {
        tf with base_types =
                  InvSSet.union base_types tf.base_types } in
    let tf =
      if InvSSet.is_empty require_extends
      then tf
      else {
        tf with require_extends =
                  InvSSet.union require_extends tf.require_extends } in
    let tf =
      if InvSSet.is_empty require_implements
      then tf
      else {
        tf with require_implements =
                  InvSSet.union require_implements tf.require_implements } in

    if phys_equal tf old_tf then types
    else InvSMap.add name tf types

  | None ->
    let base_types =
      if kind = TKEnum
      then InvSSet.add "HH\\BuiltinEnum" base_types
      else base_types in
    let tf =
      { base_types; flags; kind; require_extends; require_implements } in
    InvSMap.add name tf types

let typenames_from_list ns init l =
  let open FSC in
  let aux s n =
    match qualified_name ns n with
    | Some n -> InvSSet.add n s
    | None -> s in
  match l with
  | List l -> List.fold_left l ~init ~f:aux
  | _ -> init

let define_name n =
  let n = n () in
  (* strip quotes *)
  String.sub n 1 (String.length n - 2)

let defines_from_method_body constants body =
  let open FSC in
  let rec aux acc l =
    match l with
    | List l -> List.fold_left l ~init:acc ~f:aux
    | Define (String name) -> (define_name name) :: acc
    | _ -> acc in
  aux constants body
let type_info_from_class_body facts ns check_require body =
  let open FSC in
  let aux (extends, implements, trait_uses, constants as acc) n =
    match n with
    | RequireExtendsClause name when check_require ->
      begin match qualified_name ns name with
        | Some name -> InvSSet.add name extends, implements, trait_uses, constants
        | None -> acc
      end
    | RequireImplementsClause name when check_require ->
      begin match qualified_name ns name with
        | Some name -> extends, InvSSet.add name implements, trait_uses, constants
        | None -> acc
      end
    | TraitUseClause uses ->
      let trait_uses = typenames_from_list ns trait_uses uses in
      extends, implements, trait_uses, constants
    | MethodDecl body when ns = "" ->
      (* in methods we collect only defines *)
      let constants = defines_from_method_body constants body in
      extends, implements, trait_uses, constants
    | _ -> acc in
  let init = InvSSet.empty, InvSSet.empty, InvSSet.empty, facts.constants in
  let extends, implements, trait_uses, constants =
    match body with
    | List l -> List.fold_left l ~init ~f:aux
    | _ -> init in
  let facts =
    if phys_equal constants facts.constants
    then facts
    else { facts with constants } in
  extends, implements, trait_uses, facts

let facts_from_class_decl facts ns modifiers kind name extends implements body =
  let open FSC in
  match qualified_name ns name with
  | None -> facts
  | Some name ->
    let kind, flags =
      match kind with
      | Class -> TKClass, flags_from_modifiers modifiers
      | Interface -> TKInterface, flags_abstract
      | Trait -> TKTrait, flags_abstract
      | _ -> TKUnknown, flags_default in
    let require_extends, require_implements, trait_uses, facts =
      type_info_from_class_body facts ns
        (kind = TKInterface || kind = TKTrait) body in
    let base_types = typenames_from_list ns trait_uses extends in
    let base_types = typenames_from_list ns base_types implements in
    let types =
      add_or_update_classish_declaration facts.types name kind flags
        base_types require_extends require_implements in
    if phys_equal types facts.types
    then facts
    else { facts with types }

let rec collect (ns, facts as acc) n =
  let open FSC in
  match n with
  | List l ->
    List.fold_left ~init:acc ~f:collect l
  | ClassDecl decl ->
    let facts = facts_from_class_decl facts ns
        decl.modifiers decl.kind decl.name
        decl.extends decl.implements decl.body in
    ns, facts
  | EnumDecl name ->
    begin match qualified_name ns name with
      | Some name ->
        let types =
          add_or_update_classish_declaration facts.types name
            TKEnum flags_final InvSSet.empty InvSSet.empty InvSSet.empty in
        let facts =
          if phys_equal types facts.types
          then facts
          else { facts with types } in
        ns, facts
      | None -> acc
    end
  | FunctionDecl name ->
    begin match qualified_name ns name with
      | Some name -> ns, { facts with functions = name :: facts.functions }
      | None -> acc
    end
  | ConstDecl name ->
    begin match qualified_name ns name with
      | Some name -> ns, { facts with constants = name :: facts.constants }
      | None -> acc
    end
  | TypeAliasDecl name ->
    begin match qualified_name ns name with
      | Some name -> ns, { facts with type_aliases = name :: facts.type_aliases }
      | None -> acc
    end
  | Define (String name) when ns = "" ->
    ns, { facts with constants = (define_name name) :: facts.constants }
  | NamespaceDecl (name, FSC.EmptyBody) ->
    begin match qualified_name "" name with
      | Some name -> name, facts
      | None -> acc
    end
  | NamespaceDecl (name, body) ->
    let name =
      if name = Ignored
      then Some ns
      else qualified_name ns name in
    begin match name with
      | Some name ->
        let _, facts = collect (name, facts) body in
        ns, facts
      | None -> acc
    end
  | _ -> acc

let from_text
    ~(php5_compat_mode:bool)
    ~(hhvm_compat_mode:bool)
    ~(force_hh:bool)
    ~(enable_xhp:bool)
    ~(filename: Relative_path.t)
    ~(text: string): facts option =
  let env = Full_fidelity_parser_env.make ~codegen:true ~php5_compat_mode ~hhvm_compat_mode
      ~force_hh ~enable_xhp () in
  let text = Full_fidelity_source_text.make filename text in
  let (parser, root) =
    let p = FactsParser.make env text in
    FactsParser.parse_script p in
  let has_script_content = FactsParser.sc_state parser in
  (* report errors only if result of parsing is non-empty *)
  if has_script_content && not @@ List.is_empty (FactsParser.errors parser)
  then None
  else begin
    let _, facts = collect ("", empty) root in
    Some facts
  end
let extract_as_json
    ~(php5_compat_mode:bool)
    ~(hhvm_compat_mode:bool)
    ~(force_hh:bool)
    ~(enable_xhp:bool)
    ~(filename: Relative_path.t)
    ~(text: string): Hh_json.json option =
  from_text php5_compat_mode hhvm_compat_mode force_hh enable_xhp filename text
  |> Option.map ~f:(fun facts ->
      let md5 = OpaqueDigest.to_hex @@ OpaqueDigest.string text in
      let sha1 = Sha1.digest text in
      facts_to_json ~md5 ~sha1 facts)
