(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Simplihack_intf

module Make (LangService : LanguageServiceProvider) = struct
  type t = LangService.t

  module Class = struct
    (* Helper functions to reduce code duplication *)

    (* Create a symbol occurrence for a method or constructor *)
    let create_symbol_occurrence class_name method_name =
      let dummy_pos = Pos.none in
      SymbolOccurrence.
        {
          name = method_name;
          type_ =
            SymbolOccurrence.Method
              (SymbolOccurrence.ClassName class_name, method_name);
          is_declaration = None;
          pos = dummy_pos;
        }

    (* Format a method or constructor description directly from class_elt *)
    let format_member_description env class_name ce method_name kind =
      let open Typing_defs in
      (* Get visibility as string *)
      let visibility =
        match ce.ce_visibility with
        | Vpublic -> "public"
        | Vprivate _ -> "private"
        | Vprotected _ -> "protected"
        | Vinternal _ -> "internal"
        | Vprotected_internal _ -> "protected internal"
      in

      (* Force evaluation of the lazy type *)
      let ce_type = Lazy.force ce.ce_type in

      (* Create a symbol occurrence for the method/constructor *)
      let occurrence = create_symbol_occurrence class_name method_name in

      (* Get a string representation of the type *)
      let type_str =
        Tast_env.print_decl_ty_with_identity env ce_type occurrence None
      in

      (* Format the description *)
      let description =
        Format.asprintf
          "@[<v>%s %s defined in %s: %s@]"
          visibility
          kind
          (Utils.strip_ns ce.ce_origin)
          type_str
      in

      (* Add deprecation warning if applicable *)
      match ce.ce_deprecated with
      | Some msg ->
        Format.asprintf
          "%s@,@,WARNING: This method is deprecated: %s"
          description
          msg
      | None -> description

    let constructor ctx name =
      let cls = LangService.Class.find ~ctx name in
      let env = LangService.env ctx in
      let constructor = LangService.Class.constructor cls in
      match constructor with
      | None ->
        Format.asprintf
          "@[<v>This class does not have an explicit constructor.@,It uses the default constructor.@,Default constructors are public and do not have any parameters.@]"
      | Some ce ->
        (* Get class name as string *)
        let class_name = snd name in

        (* Format the constructor description *)
        format_member_description env class_name ce "__construct" "constructor"

    (* Helper function to format class members (methods, fields, etc.) *)
    let format_class_members ctx name get_members member_type =
      let cls = LangService.Class.find ~ctx name in
      let env = LangService.env ctx in
      let class_name = snd name in

      (* Get all members from the class using the provided function *)
      let members = get_members cls in

      if List.is_empty members then
        Format.asprintf
          "The class %s does not have any %ss."
          (Utils.strip_ns class_name)
          member_type
      else
        (* Format each member with its signature and visibility *)
        let member_entries =
          List.map members ~f:(fun (member_name, ce) ->
              (* Format the member description *)
              format_member_description
                env
                class_name
                ce
                member_name
                (member_type ^ " " ^ member_name))
        in

        (* Convert member entries list to a formatted string using fold *)
        let formatted_members =
          List.fold member_entries ~init:"" ~f:(fun acc entry ->
              if String.is_empty acc then
                entry
              else
                Format.asprintf "%s@,%s" acc entry)
        in
        Format.asprintf "@[<v>%s@]" formatted_members

    let methods ctx name =
      format_class_members ctx name LangService.Class.methods "instance method"

    let static_methods ctx name =
      format_class_members
        ctx
        name
        LangService.Class.static_methods
        "static method"

    let fields ctx name =
      format_class_members ctx name LangService.Class.fields "instance field"

    let static_fields ctx name =
      format_class_members
        ctx
        name
        LangService.Class.static_fields
        "static field"
  end
end
