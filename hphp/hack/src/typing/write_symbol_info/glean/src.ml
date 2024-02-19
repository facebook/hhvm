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
    JSON_Object ([
      ("file", Some (File.to_json file));
      ("reason", Some (IndexFailureReason.to_json reason));
      ("details", Some (JSON_String details));
    ] |> List.filter_map Util.rem_opt)

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
    JSON_Object ([
      ("fileLines", Some (Range.to_json file_lines));
      ("contains", Some (Range.to_json contains));
    ] |> List.filter_map Util.rem_opt)

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
    JSON_Object ([
      ("byteSpan", Some (ByteSpan.to_json byte_span));
      ("contains", Some (ByteSpan.to_json contains));
    ] |> List.filter_map Util.rem_opt)

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
    JSON_Object ([
      ("file", Some (File.to_json file));
      ("language", Some (Language.to_json language));
    ] |> List.filter_map Util.rem_opt)

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
    JSON_Object ([
      ("file", Some (File.to_json file));
      ("lengths", Some (JSON_Array (List.map (fun x -> JSON_Number (string_of_int x)) lengths)));
      ("endsInNewline", Some (JSON_Bool ends_in_newline));
      ("hasUnicodeOrTabs", Some (JSON_Bool has_unicode_or_tabs));
    ] |> List.filter_map Util.rem_opt)

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
    JSON_Object ([
      ("offset", Some (JSON_Number (string_of_int offset)));
      ("length", Some (JSON_Number (string_of_int length)));
    ] |> List.filter_map Util.rem_opt)

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
    JSON_Object ([
      ("begin", Some (JSON_Number (string_of_int begin_)));
      ("end", Some (JSON_Number (string_of_int end_)));
    ] |> List.filter_map Util.rem_opt)

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
    JSON_Object ([
      ("file", Some (File.to_json file));
      ("line", Some (JSON_Number (string_of_int line)));
      ("column", Some (JSON_Number (string_of_int column)));
    ] |> List.filter_map Util.rem_opt)

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
    JSON_Object ([
      ("start", Some (JSON_Number (string_of_int start)));
      ("length", Some (JSON_Number (string_of_int length)));
    ] |> List.filter_map Util.rem_opt)

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
    JSON_Object ([
      ("file", Some (File.to_json file));
      ("lineBegin", Some (JSON_Number (string_of_int line_begin)));
      ("columnBegin", Some (JSON_Number (string_of_int column_begin)));
      ("lineEnd", Some (JSON_Number (string_of_int line_end)));
      ("columnEnd", Some (JSON_Number (string_of_int column_end)));
    ] |> List.filter_map Util.rem_opt)

end

and PackedByteSpans: sig
  type t = PackedByteSpansGroup.t list
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = PackedByteSpansGroup.t list
  [@@deriving ord]

  let to_json l = JSON_Array (List.map (fun x -> PackedByteSpansGroup.to_json x) l)
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
    JSON_Object ([
      ("length", Some (JSON_Number (string_of_int length)));
      ("offsets", Some (JSON_Array (List.map (fun x -> JSON_Number (string_of_int x)) offsets)));
    ] |> List.filter_map Util.rem_opt)

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
    JSON_Object ([
      ("file", Some (File.to_json file));
      ("span", Some (ByteSpan.to_json span));
    ] |> List.filter_map Util.rem_opt)

end


