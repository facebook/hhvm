(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_json

module type Result_printer = sig
  type t

  val go : (t, string) result -> bool -> unit
end

module type Result_converter = sig
  type t

  val to_string : t -> string

  val to_json : t -> Hh_json.json
end

module Unit_converter = struct
  type t = unit

  let to_string : t -> string = (fun () -> "")

  let to_json : t -> Hh_json.json = (fun () -> JSON_String "ok")
end

module Int_converter = struct
  type t = int

  let to_string : t -> string = string_of_int

  let to_json : t -> Hh_json.json = (fun i -> JSON_Number (string_of_int i))
end

module Make (Converter : Result_converter) :
  Result_printer with type t = Converter.t = struct
  type t = Converter.t

  let to_json result =
    let result =
      match result with
      | Ok result -> ("result", Converter.to_json result)
      | Error s -> ("error_message", JSON_String s)
    in
    JSON_Object [result]

  let print_json res = print_endline (Hh_json.json_to_string (to_json res))

  let print_readable = function
    | Ok result -> Printf.printf "%s" (Converter.to_string result)
    | Error s ->
      let msg = Printf.sprintf "Error: %s" s in
      print_endline msg;
      exit 1

  let go res output_json =
    if output_json then
      print_json res
    else
      print_readable res
end

module Unit_printer = Make (Unit_converter)
module Int_printer = Make (Int_converter)
