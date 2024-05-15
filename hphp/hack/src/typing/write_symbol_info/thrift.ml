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

type kind =
  | Exception
  | Struct
  | Service

type container = {
  thrift_path: string;
  name: string;
  kind: kind;
}

type t = {
  services: (string, string * string) Hashtbl.t;
  mutable cur_container: container option;
}

let empty () =
  { services = Hashtbl.create (module String); cur_container = None }

let add { services; _ } ~interface ~thrift_path ~service =
  Hashtbl.set services ~key:interface ~data:(thrift_path, service)

let lookup { services; _ } ~interfaces =
  List.filter_map ~f:(Hashtbl.find services) interfaces |> List.hd

let rex_container =
  Pcre.regexp
    ~flags:[`MULTILINE]
    "Original thrift (struct|service|exception):-(?:\\s|\\*)*([\\w]+)"

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
      Some ({ thrift_path; name; kind = Service }, xref_target)
    | "struct" ->
      let xref_target =
        XRefTarget.(
          Named
            NamedDecl.(
              Key
                { name = NamedType.{ name = qname; kind = NamedKind.Struct_ } }))
      in
      Some ({ thrift_path; name; kind = Struct }, xref_target)
    | "exception" ->
      let xref_target =
        XRefTarget.(Exception_ ExceptionName.(Key { name = qname }))
      in
      Some ({ thrift_path; name; kind = Exception }, xref_target)
    | _ -> failwith "internal error"
  with
  | _ ->
    Hh_logger.log "Couldn't parse thrift comment %s" doc;
    None

let get_thrift_from_container t ~thrift_path con =
  let (container, fact) =
    match con.c_doc_comment with
    | Some (_, doc) ->
      (match get_thrift_from_container_aux ~thrift_path ~doc with
      | Some (container, fact) -> (Some container, Some fact)
      | None -> (None, None))
    | None -> (None, None)
  in
  let parent_kind = Predicate.get_parent_kind con.c_kind in
  (match (container, parent_kind) with
  | (Some { thrift_path; name; kind = Service }, Predicate.InterfaceContainer)
    ->
    add t ~interface:(snd con.c_name) ~service:name ~thrift_path
  | _ -> ());
  let implements_to_string = function
    | (_, Aast_defs.Happly ((_, x), _)) -> Some x
    | _ -> None
  in
  let interfaces = List.filter_map ~f:implements_to_string con.c_implements in
  let container =
    match (parent_kind, container) with
    | (Predicate.ClassContainer, None) ->
      (match lookup t ~interfaces with
      | Some (thrift_path, service) ->
        Some { thrift_path; name = service; kind = Service }
      | None -> container)
    | _ -> container
  in
  Option.iter ~f:(fun container -> t.cur_container <- Some container) container;
  fact

let rex_member =
  Pcre.regexp
    ~flags:[`MULTILINE]
    "(?:Original thrift definition:-(?:\\s|\\*)*(?:\\w)+(?:\\s|\\*)*(\\w+))|(?:Original thrift field:-(?:\\s|\\*|.|\\d|\\:)*(?:\\w+) (\\w+))"

let make_member_decl member_name container_qname kind =
  let name = container_qname in
  match kind with
  | Service ->
    XRefTarget.(
      Function_
        FunctionName.(
          Key
            {
              service_ = ServiceName.(Key { name });
              name = Identifier.Key member_name;
            }))
  | Struct ->
    XRefTarget.(
      Named
        NamedDecl.(Key { name = NamedType.{ name; kind = NamedKind.Struct_ } }))
  | Exception -> XRefTarget.(Exception_ ExceptionName.(Key { name }))

let get_thrift_from_member t ~doc =
  try
    let substrings = Pcre.exec ~rex:rex_member doc in
    let member_name =
      try Pcre.get_substring substrings 1 with
      | _ -> Pcre.get_substring substrings 2
    in
    match t.cur_container with
    | None ->
      Hh_logger.log "Container not set when indexing this method";
      None
    | Some { thrift_path; name; kind } ->
      let name = Identifier.Key name in
      let file = File.Key (Src.File.Key thrift_path) in
      let container_qname = QualName.(Key { file; name }) in
      Some (make_member_decl member_name container_qname kind)
  with
  | _ ->
    Hh_logger.log "Couldn't parse thrift comment %s" doc;
    None
