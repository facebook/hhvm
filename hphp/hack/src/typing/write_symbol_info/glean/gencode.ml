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


module rec GenCodeSignature: sig
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

and GenCodeBySource: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    source: Src.File.t;
    gencode: Src.File.t;
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
    source: Src.File.t;
    gencode: Src.File.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {source; gencode} = 
    JSON_Object ([
      ("source", Some (Src.File.to_json source));
      ("gencode", Some (Src.File.to_json gencode));
    ] |> List.filter_map Util.rem_opt)

end

and GenCodeCommand: sig
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

and GenCodeClass: sig
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

and GenCode: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    file: Src.File.t;
    variant: GenCodeVariant.t;
    source: Src.File.t option;
    command: GenCodeCommand.t option;
    class_: GenCodeClass.t option;
    signature: GenCodeSignature.t option;
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
    file: Src.File.t;
    variant: GenCodeVariant.t;
    source: Src.File.t option;
    command: GenCodeCommand.t option;
    class_: GenCodeClass.t option;
    signature: GenCodeSignature.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {file; variant; source; command; class_; signature} = 
    JSON_Object ([
      ("file", Some (Src.File.to_json file));
      ("variant", Some (GenCodeVariant.to_json variant));
      ("source", Option.map (fun x -> Src.File.to_json x) source);
      ("command", Option.map (fun x -> GenCodeCommand.to_json x) command);
      ("class_", Option.map (fun x -> GenCodeClass.to_json x) class_);
      ("signature", Option.map (fun x -> GenCodeSignature.to_json x) signature);
    ] |> List.filter_map Util.rem_opt)

end


and GenCodeVariant: sig
  type t = 
    | Full
    | Partial

  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
    | Full
    | Partial

  [@@deriving ord]

  let to_json  = function
     | Full -> JSON_Number (string_of_int 0)
     | Partial -> JSON_Number (string_of_int 1)

end


