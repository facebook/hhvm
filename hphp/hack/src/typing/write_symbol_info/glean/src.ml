(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 *)

(* @generated
   regenerate: buck2 run fbcode//glean/schema/gen:gen-schema  -- --ocaml fbcode/hphp/hack/src/typing/write_symbol_info/schema --dir DEST_DIR *)

open Hh_json
open Core [@@warning "-33"]


module rec FileDigest: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= File.t
  [@@deriving ord]
  and value= string
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json
  val to_json_value: value -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= File.t
  [@@deriving ord]
  and value= string
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key x = File.to_json x
  and to_json_value str = JSON_String str
end

and IndexFailure: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    file: File.t;
    reason: IndexFailureReason.t;
    details: string;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    file: File.t;
    reason: IndexFailureReason.t;
    details: string;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {file; reason; details} = 
    let fields = [
      ("file", File.to_json file);
      ("reason", IndexFailureReason.to_json reason);
      ("details", JSON_String details);
    ] in
    JSON_Object fields

end

and RangeContains: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    file_lines: Range.t;
    contains: Range.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    file_lines: Range.t;
    contains: Range.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {file_lines; contains} = 
    let fields = [
      ("fileLines", Range.to_json file_lines);
      ("contains", Range.to_json contains);
    ] in
    JSON_Object fields

end

and ByteSpanContains: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    byte_span: ByteSpan.t;
    contains: ByteSpan.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    byte_span: ByteSpan.t;
    contains: ByteSpan.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {byte_span; contains} = 
    let fields = [
      ("byteSpan", ByteSpan.to_json byte_span);
      ("contains", ByteSpan.to_json contains);
    ] in
    JSON_Object fields

end

and File: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= string
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= string
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key str = JSON_String str
end

and FileLanguage: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    file: File.t;
    language: Language.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    file: File.t;
    language: Language.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {file; language} = 
    let fields = [
      ("file", File.to_json file);
      ("language", Language.to_json language);
    ] in
    JSON_Object fields

end

and FileLines: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    file: File.t;
    lengths: int list;
    ends_in_newline: bool;
    has_unicode_or_tabs: bool;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    file: File.t;
    lengths: int list;
    ends_in_newline: bool;
    has_unicode_or_tabs: bool;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {file; lengths; ends_in_newline; has_unicode_or_tabs} = 
    let fields = [
      ("file", File.to_json file);
      ("lengths", JSON_Array (List.map ~f:(fun x -> JSON_Number (string_of_int x)) lengths));
      ("endsInNewline", JSON_Bool ends_in_newline);
      ("hasUnicodeOrTabs", JSON_Bool has_unicode_or_tabs);
    ] in
    JSON_Object fields

end


and RelByteSpan: sig
  type t = {
    offset: int;
    length: int;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    offset: int;
    length: int;
  }
  [@@deriving ord]

  let to_json {offset; length} = 
    let fields = [
      ("offset", JSON_Number (string_of_int offset));
      ("length", JSON_Number (string_of_int length));
    ] in
    JSON_Object fields

end

and ByteRange: sig
  type t = {
    begin_: int;
    end_: int;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    begin_: int;
    end_: int;
  }
  [@@deriving ord]

  let to_json {begin_; end_} = 
    let fields = [
      ("begin", JSON_Number (string_of_int begin_));
      ("end", JSON_Number (string_of_int end_));
    ] in
    JSON_Object fields

end

and IndexFailureReason: sig
  type t = 
    | CompileError
    | BuildSystemError
    | Unclassified
    | DiscoveryError

  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
    | CompileError
    | BuildSystemError
    | Unclassified
    | DiscoveryError

  [@@deriving ord]

  let to_json  = function
     | CompileError -> JSON_Number (string_of_int 0)
     | BuildSystemError -> JSON_Number (string_of_int 1)
     | Unclassified -> JSON_Number (string_of_int 2)
     | DiscoveryError -> JSON_Number (string_of_int 3)

end

and Loc: sig
  type t = {
    file: File.t;
    line: int;
    column: int;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    file: File.t;
    line: int;
    column: int;
  }
  [@@deriving ord]

  let to_json {file; line; column} = 
    let fields = [
      ("file", File.to_json file);
      ("line", JSON_Number (string_of_int line));
      ("column", JSON_Number (string_of_int column));
    ] in
    JSON_Object fields

end

and ByteSpan: sig
  type t = {
    start: int;
    length: int;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    start: int;
    length: int;
  }
  [@@deriving ord]

  let to_json {start; length} = 
    let fields = [
      ("start", JSON_Number (string_of_int start));
      ("length", JSON_Number (string_of_int length));
    ] in
    JSON_Object fields

end

and Range: sig
  type t = {
    file: File.t;
    line_begin: int;
    column_begin: int;
    line_end: int;
    column_end: int;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    file: File.t;
    line_begin: int;
    column_begin: int;
    line_end: int;
    column_end: int;
  }
  [@@deriving ord]

  let to_json {file; line_begin; column_begin; line_end; column_end} = 
    let fields = [
      ("file", File.to_json file);
      ("lineBegin", JSON_Number (string_of_int line_begin));
      ("columnBegin", JSON_Number (string_of_int column_begin));
      ("lineEnd", JSON_Number (string_of_int line_end));
      ("columnEnd", JSON_Number (string_of_int column_end));
    ] in
    JSON_Object fields

end

and PackedByteSpans: sig
  type t = PackedByteSpansGroup.t list
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = PackedByteSpansGroup.t list
  [@@deriving ord]

  let to_json l = JSON_Array (List.map ~f:(fun x -> PackedByteSpansGroup.to_json x) l)
end

and PackedByteSpansGroup: sig
  type t = {
    length: int;
    offsets: int list;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    length: int;
    offsets: int list;
  }
  [@@deriving ord]

  let to_json {length; offsets} = 
    let fields = [
      ("length", JSON_Number (string_of_int length));
      ("offsets", JSON_Array (List.map ~f:(fun x -> JSON_Number (string_of_int x)) offsets));
    ] in
    JSON_Object fields

end

and Language: sig
  type t = 
    | Buck
    | C
    | Cpp
    | Hack
    | Haskell
    | ObjC
    | ObjCpp
    | Python
    | Thrift
    | Java
    | GraphQL

  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
    | Buck
    | C
    | Cpp
    | Hack
    | Haskell
    | ObjC
    | ObjCpp
    | Python
    | Thrift
    | Java
    | GraphQL

  [@@deriving ord]

  let to_json  = function
     | Buck -> JSON_Number (string_of_int 0)
     | C -> JSON_Number (string_of_int 1)
     | Cpp -> JSON_Number (string_of_int 2)
     | Hack -> JSON_Number (string_of_int 3)
     | Haskell -> JSON_Number (string_of_int 4)
     | ObjC -> JSON_Number (string_of_int 5)
     | ObjCpp -> JSON_Number (string_of_int 6)
     | Python -> JSON_Number (string_of_int 7)
     | Thrift -> JSON_Number (string_of_int 8)
     | Java -> JSON_Number (string_of_int 9)
     | GraphQL -> JSON_Number (string_of_int 10)

end

and FileLocation: sig
  type t = {
    file: File.t;
    span: ByteSpan.t;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    file: File.t;
    span: ByteSpan.t;
  }
  [@@deriving ord]

  let to_json {file; span} = 
    let fields = [
      ("file", File.to_json file);
      ("span", ByteSpan.to_json span);
    ] in
    JSON_Object fields

end


