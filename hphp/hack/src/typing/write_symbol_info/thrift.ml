(*
 * Copyright (c) Meta, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Hh_prelude
open Fbthrift
open Option.Monad_infix

type kind =
  | Exception
  | Struct
  | Union
  | Service
  | Enum

type container = {
  name: string;
  kind: kind;
}

type t = {
  thrift_path: string;
  services: (string, string) Hashtbl.t;
  mutable cur_container: container option;
  mutable cur_union_fields: string list;
}

let empty ~thrift_path =
  {
    thrift_path;
    services = Hashtbl.create (module String);
    cur_container = None;
    cur_union_fields = [];
  }

let add { services; _ } ~interface ~service =
  Hashtbl.set services ~key:interface ~data:service

let lookup { services; _ } ~interfaces =
  List.filter_map ~f:(Hashtbl.find services) interfaces |> List.hd

let rex_id = "(\\w(?:\\w|\\.)+)"

let rex_ty = "(?:\\w|<|>|\\.|, )+"

let rex_sp = "(?:\\s|\\*|\\d|\\:|\\-)*"

let rex_container =
  Pcre.regexp
    ~flags:[`MULTILINE]
    (Printf.sprintf
       "Original thrift (struct|service|exception|union|enum):-%s%s"
       rex_sp
       rex_id)

let rex_member =
  Pcre.regexp
    ~flags:[`MULTILINE]
    (Printf.sprintf
       "Original thrift (field|definition|constant):-%s%s%s%s"
       rex_sp
       rex_ty
       rex_sp
       rex_id)

let get_thrift_from_container_aux ~thrift_path ~doc =
  try
    let substrings = Pcre.exec ~rex:rex_container doc in
    let kind = Pcre.get_substring substrings 1 in
    let name = Pcre.get_substring substrings 2 in
    let identifier = Identifier.Key name in
    let file = File.Key (Src.File.Key thrift_path) in
    let qname = QualName.(Key { file; name = identifier }) in
    match kind with
    | "service" ->
      let serv = ServiceName.(Key { name = qname }) in
      let xref_target = XRefTarget.(Service_ serv) in
      Some ({ name; kind = Service }, xref_target)
    | "struct" ->
      let xref_target =
        XRefTarget.(
          Named
            NamedDecl.(
              Key
                { name = NamedType.{ name = qname; kind = NamedKind.Struct_ } }))
      in
      Some ({ name; kind = Struct }, xref_target)
    | "union" ->
      let xref_target =
        XRefTarget.(
          Named
            NamedDecl.(
              Key { name = NamedType.{ name = qname; kind = NamedKind.Union_ } }))
      in
      Some ({ name; kind = Union }, xref_target)
    | "enum" ->
      let xref_target =
        XRefTarget.(
          Named
            NamedDecl.(
              Key { name = NamedType.{ name = qname; kind = NamedKind.Enum_ } }))
      in
      Some ({ name; kind = Enum }, xref_target)
    | "exception" ->
      let xref_target =
        XRefTarget.(Exception_ ExceptionName.(Key { name = qname }))
      in
      Some ({ name; kind = Exception }, xref_target)
    | _ -> failwith "internal error"
  with
  | _ -> None

let get_thrift_from_container t con =
  let (container, fact) =
    match con.c_doc_comment with
    | Some (_, doc) ->
      (match get_thrift_from_container_aux ~thrift_path:t.thrift_path ~doc with
      | Some (container, fact) -> (Some container, Some fact)
      | None -> (None, None))
    | None -> (None, None)
  in
  (match (container, con.c_kind) with
  | (Some { name; kind = Service }, Ast_defs.Cinterface) ->
    add t ~interface:(snd con.c_name) ~service:name
  | _ -> ());
  let implements_to_string = function
    | (_, Aast_defs.Happly ((_, x), _)) -> Some x
    | _ -> None
  in
  let container =
    match (con.c_kind, container) with
    | (Ast_defs.Cclass _, None) ->
      let interfaces =
        List.filter_map ~f:implements_to_string con.c_implements
      in
      (match lookup t ~interfaces with
      | Some service -> Some { name = service; kind = Service }
      | None -> container)
    | _ -> container
  in
  Option.iter ~f:(fun container -> t.cur_container <- Some container) container;
  t.cur_union_fields <- [];
  fact

let make_member_decl member_name container_qname kind =
  let name = container_qname in
  let mname = Identifier.Key member_name in
  match kind with
  | Service ->
    XRefTarget.(
      Function_
        FunctionName.(
          Key { service_ = ServiceName.(Key { name }); name = mname }))
  | Struct ->
    XRefTarget.(
      Field
        FieldDecl.(Key { qname = name; kind = FieldKind.Struct_; name = mname }))
  | Union ->
    XRefTarget.(
      Field
        FieldDecl.(Key { qname = name; kind = FieldKind.Union_; name = mname }))
  | Exception ->
    XRefTarget.(
      Field
        FieldDecl.(
          Key { qname = name; kind = FieldKind.Exception_; name = mname }))
  | Enum ->
    XRefTarget.(
      EnumValue
        EnumValue.(
          Key
            { enum_ = NamedType.{ name; kind = NamedKind.Enum_ }; name = mname }))

let get_thrift_from_comment t ~doc =
  try
    let substrings = Pcre.exec ~rex:rex_member doc in
    let member_name = Pcre.get_substring substrings 2 in
    let file = File.Key (Src.File.Key t.thrift_path) in
    match t.cur_container with
    | Some { name; kind } ->
      (match kind with
      | Union -> t.cur_union_fields <- member_name :: t.cur_union_fields
      | _ -> ());
      let name = Identifier.Key name in
      let container_qname = QualName.(Key { file; name }) in
      Some (make_member_decl member_name container_qname kind)
    | None ->
      let member_name = Identifier.Key member_name in
      let name = QualName.(Key { file; name = member_name }) in
      Some XRefTarget.(Constant Constant.(Key { name }))
  with
  | _ -> None

let get_thrift_from_enum t member_name =
  t.cur_container >>= fun { name; kind } ->
  let name = Identifier.Key name in
  let file = File.Key (Src.File.Key t.thrift_path) in
  let container_qname = QualName.(Key { file; name }) in
  Some (make_member_decl member_name container_qname kind)

let get_thrift_from_union_member t member_name =
  t.cur_container >>= fun { name; kind } ->
  List.find t.cur_union_fields ~f:(fun suffix ->
      String.is_suffix ~suffix member_name)
  >>= fun union_field_name ->
  match kind with
  | Union ->
    let file = File.Key (Src.File.Key t.thrift_path) in
    let name = Identifier.Key name in
    let container_qname = QualName.(Key { file; name }) in
    Some (make_member_decl union_field_name container_qname kind)
  | _ -> None
