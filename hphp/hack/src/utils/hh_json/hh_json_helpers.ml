(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 * *)

module Option = Base.Option
module Int = Base.Int
module List = Base.List

(************************************************************************)
(* Helpers for parsing & printing                                       *)
(************************************************************************)

module Jget = struct
  exception Parse of string

  (* Helpers for the various "option" monads in use for Json, to succinctly
     capture the spirit of JSON (tolerance for missing values) and the spirit
     of LSP (loads of nested optional members with obvious defaults)
     and the usefuless of error-checking (in case a required field is absent)...
     - We use "json option" throughout. Things which you might expect to return
       a json are instead lifted to return a json option, so you can use all the
       accessors on them more easily. When you attempt to get string "o.m", either
       it's present both because "o" is Some, and because "m" is a string member
       Or it's absent because any of those three things is false...
     - The "_opt" accessors uniformally return Some (present) or None (absent),
       regardless of which of the two things caused absence.
     - The "_d" accessors uniformally return a value (present) or default.
     - The "_exn" accessors uniformally return a value (present) or throw.

     The effect of this is you lose precise information about what exactly
     caused an absence (which is usually only of marginal benefit), and in
     return you gain a consistent way to handle both optionals and requireds.

     Note one exception to the above: if you wish to get an int/float, and it's
     present as a number but not parseable as an int/float, then all
     accessors will throw.
  *)

  let member key = function
    | `Assoc obj -> List.Assoc.find ~equal:String.equal obj key
    | _ -> None

  (* Accessors which return None on absence *)
  let string_opt json key =
    match json with
    | None -> None
    | Some json ->
      (match member key json with
      | Some (`String s) -> Some s
      | _ -> None)

  let bool_opt json key =
    match json with
    | None -> None
    | Some json ->
      (match member key json with
      | Some (`Bool b) -> Some b
      | _ -> None)

  let obj_opt json key =
    match json with
    | None -> None
    | Some json ->
      (match member key json with
      | Some (`Assoc _ as obj) -> Some obj
      | _ -> None)

  let val_opt json key =
    match json with
    | None -> None
    | Some json -> member key json

  let int_opt json key =
    match json with
    | None -> None
    | Some json ->
      (match member key json with
      | Some (`Int i) -> Some i
      | Some (`Intlit s) ->
        (try Some (Int.of_string s) with
        | Failure _ -> raise (Parse ("not an int: " ^ s)))
      | _ -> None)

  let float_opt json key =
    match json with
    | None -> None
    | Some json ->
      (match member key json with
      | Some (`Float f) -> Some f
      | Some (`Int i) -> Some (Float.of_int i)
      | Some (`Intlit s) ->
        (try Some (Float.of_string s) with
        | Failure _ -> raise (Parse ("not a float: " ^ s)))
      | _ -> None)

  let array_opt json key =
    match json with
    | None -> None
    | Some json ->
      (match member key json with
      | Some (`List l) -> Some (List.map ~f:(fun a -> Some a) l)
      | _ -> None)

  let get_exn opt_getter json key =
    match opt_getter json key with
    | None -> raise (Parse key)
    | Some v -> v

  (* Accessors which return a supplied default on absence *)
  let string_d json key ~default = Option.value (string_opt json key) ~default

  let bool_d json key ~default = Option.value (bool_opt json key) ~default

  let int_d json key ~default = Option.value (int_opt json key) ~default

  let float_d json key ~default = Option.value (float_opt json key) ~default

  let array_d json key ~default = Option.value (array_opt json key) ~default

  (* Accessors which throw "Error.Parse key" on absence *)
  let bool_exn = get_exn bool_opt

  let string_exn = get_exn string_opt

  let val_exn = get_exn val_opt

  let int_exn = get_exn int_opt

  let float_exn = get_exn float_opt

  let array_exn = get_exn array_opt

  (** obj_exn lifts the result into the "json option" monad *)
  let obj_exn json key = Some (get_exn obj_opt json key)

  let string_array_exn json key =
    array_exn json key
    |> List.map ~f:(fun opt -> Option.value_exn opt)
    |> List.map ~f:(function
           | `String s -> s
           | _ -> raise (Parse "expected string in array"))
end

module Jprint = struct
  (* object_opt is like `Assoc constructor except it takes
     key * (value option): if a value is None, then it omits this member. *)
  let object_opt (keyvalues : (string * Yojson.Safe.t option) list) :
      Yojson.Safe.t =
    let rec filter keyvalues =
      match keyvalues with
      | [] -> []
      | (_key, None) :: rest -> filter rest
      | (key, Some value) :: rest -> (key, value) :: filter rest
    in
    `Assoc (filter keyvalues)

  (* Convenience function to convert string list to `List *)
  let string_array (l : string list) : Yojson.Safe.t =
    `List (List.map ~f:(fun s -> `String s) l)
end

(* Some ad-hoc JSON processing helpers. *)

module AdhocJsonHelpers = struct
  exception Not_found

  let get_object_exn = function
    | `Assoc obj -> obj
    | _ -> raise Not_found

  let try_get_val key json =
    let obj = get_object_exn json in
    Base.List.Assoc.find ~equal:String.equal obj key

  let get_string_exn = function
    | `String s -> s
    | _ -> raise Not_found

  let get_number_exn = function
    | `Int i -> string_of_int i
    | `Float f -> string_of_float f
    | `Intlit s -> s
    | _ -> raise Not_found

  let get_bool_exn = function
    | `Bool b -> b
    | _ -> raise Not_found

  let get_array_exn = function
    | `List l -> l
    | _ -> raise Not_found

  let get_string_val key ?default json =
    let v = try_get_val key json in
    match (v, default) with
    | (Some v, _) -> get_string_exn v
    | (None, Some def) -> def
    | (None, None) -> raise Not_found

  let get_number_val key ?default json =
    let v = try_get_val key json in
    match (v, default) with
    | (Some v, _) -> get_number_exn v
    | (None, Some def) -> def
    | (None, None) -> raise Not_found

  let get_bool_val key ?default json =
    let v = try_get_val key json in
    match (v, default) with
    | (Some v, _) -> get_bool_exn v
    | (None, Some def) -> def
    | (None, None) -> raise Not_found

  let get_array_val key ?default json =
    let v = try_get_val key json in
    match (v, default) with
    | (Some v, _) -> get_array_exn v
    | (None, Some def) -> def
    | (None, None) -> raise Not_found

  let strlist args = `List (List.map ~f:(fun arg -> `String arg) args)

  (* Prepend a string to a JSON array of strings. pred stands for predicate,
   * because that's how they are typically represented in watchman. See e.g.
   * https://facebook.github.io/watchman/docs/expr/allof.html *)
  let pred name args = `List (`String name :: args)
end

(************************************************************************)
(* Yojson.Safe.t accessor functions                                     *)
(************************************************************************)

let get_object_exn = function
  | `Assoc o -> o
  | _ -> assert false

let get_array_exn = function
  | `List a -> a
  | _ -> assert false

let get_string_exn = function
  | `String s -> s
  | _ -> assert false

let get_number_exn = function
  | `Int i -> string_of_int i
  | `Float f -> string_of_float f
  | `Intlit s -> s
  | _ -> assert false

let get_number_int_exn = function
  | `Int i -> i
  | `Intlit s -> int_of_string s
  | _ -> assert false

let get_bool_exn = function
  | `Bool b -> b
  | _ -> assert false

(************************************************************************)
(* Monadic API for traversing a JSON object                             *)
(************************************************************************)

type json_type =
  | Object_t
  | Array_t
  | String_t
  | Number_t
  | Integer_t
  | Bool_t

let json_type_to_string = function
  | Object_t -> "Object"
  | Array_t -> "Array"
  | String_t -> "String"
  | Number_t -> "Number"
  | Integer_t -> "Integer"
  | Bool_t -> "Bool"

module type Access = sig
  type keytrace = string list

  type access_failure =
    | Not_an_object of keytrace
    | Missing_key_error of string * keytrace
    | Wrong_type_error of keytrace * json_type

  type 'a m = ('a * keytrace, access_failure) result

  val keytrace_to_string : keytrace -> string

  val access_failure_to_string : access_failure -> string

  val return : 'a -> 'a m

  val ( >>= ) : 'a m -> ('a * keytrace -> 'b m) -> 'b m

  val counit_with : (access_failure -> 'a) -> 'a m -> 'a

  val to_option : 'a m -> 'a option

  val get_obj : string -> Yojson.Safe.t * keytrace -> Yojson.Safe.t m

  val get_bool : string -> Yojson.Safe.t * keytrace -> bool m

  val get_string : string -> Yojson.Safe.t * keytrace -> string m

  val get_number : string -> Yojson.Safe.t * keytrace -> string m

  val get_number_int : string -> Yojson.Safe.t * keytrace -> int m

  val get_array : string -> Yojson.Safe.t * keytrace -> Yojson.Safe.t list m

  val get_val : string -> Yojson.Safe.t * keytrace -> Yojson.Safe.t m
end

module Access = struct
  type keytrace = string list

  type access_failure =
    | Not_an_object of keytrace
    | Missing_key_error of string * keytrace
    | Wrong_type_error of keytrace * json_type

  type 'a m = ('a * keytrace, access_failure) result

  let keytrace_to_string x =
    if x = [] then
      ""
    else
      let res = List.rev x |> String.concat "." in
      " (at field `" ^ res ^ "`)"

  let access_failure_to_string = function
    | Not_an_object x ->
      Printf.sprintf "Value is not an object %s" (keytrace_to_string x)
    | Missing_key_error (x, y) ->
      Printf.sprintf "Missing key: %s%s" x (keytrace_to_string y)
    | Wrong_type_error (x, y) ->
      Printf.sprintf
        "Value expected to be %s%s"
        (json_type_to_string y)
        (keytrace_to_string x)

  let return v = Ok (v, [])

  let ( >>= ) m f =
    match m with
    | Error _ as x -> x
    | Ok x -> f x

  let counit_with f m =
    match m with
    | Ok (v, _) -> v
    | Error e -> f e

  let to_option = function
    | Ok (v, _) -> Some v
    | Error _ -> None

  let catch_type_error exp f (v, keytrace) =
    try Ok (f v, keytrace) with
    | Failure msg when String.equal "int_of_string" msg ->
      Error (Wrong_type_error (keytrace, exp))
    | Assert_failure _ -> Error (Wrong_type_error (keytrace, exp))

  let get_val k (v, keytrace) =
    try
      let obj = get_object_exn v in
      let candidate =
        List.fold_left obj ~init:None ~f:(fun opt (key, json) ->
            if opt <> None then
              opt
            else if key = k then
              Some json
            else
              None)
      in
      match candidate with
      | None -> Error (Missing_key_error (k, keytrace))
      | Some obj -> Ok (obj, k :: keytrace)
    with
    | Assert_failure _ -> Error (Not_an_object keytrace)

  let make_object_json v = `Assoc (get_object_exn v)

  let get_obj k (v, keytrace) =
    get_val k (v, keytrace) >>= catch_type_error Object_t make_object_json

  let get_bool k (v, keytrace) =
    get_val k (v, keytrace) >>= catch_type_error Bool_t get_bool_exn

  let get_string k (v, keytrace) =
    get_val k (v, keytrace) >>= catch_type_error String_t get_string_exn

  let get_number k (v, keytrace) =
    get_val k (v, keytrace) >>= catch_type_error Number_t get_number_exn

  let get_number_int k (v, keytrace) =
    get_val k (v, keytrace) >>= catch_type_error Integer_t get_number_int_exn

  let get_array k (v, keytrace) =
    get_val k (v, keytrace) >>= catch_type_error Array_t get_array_exn
end

(************************************************************************)
(* JSON truncation                                                      *)
(************************************************************************)

let ( >=@ ) : int -> int option -> bool =
 fun lhs rhs ->
  match rhs with
  | None -> false
  | Some rhs -> lhs >= rhs

let ( <=@ ) : int -> int option -> bool =
 fun lhs rhs ->
  match rhs with
  | None -> false
  | Some rhs -> lhs <= rhs

let json_truncate
    ?(max_string_length : int option)
    ?(max_object_child_count : int option)
    ?(max_array_elt_count : int option)
    ?(max_depth : int option)
    ?(max_total_count : int option)
    ?(has_changed : bool ref option)
    (json : Yojson.Safe.t) : Yojson.Safe.t =
  let total_count = ref 0 in
  let mark_changed () =
    match has_changed with
    | None -> ()
    | Some r -> r := true
  in
  let truncate_children children max_child_count ~f =
    let rec truncate_children child_count children =
      match children with
      | [] -> []
      | _ when !total_count >=@ max_total_count ->
        mark_changed ();
        []
      | _ when child_count >=@ max_child_count ->
        mark_changed ();
        []
      | c :: rest ->
        incr total_count;
        let c' = f c in
        c' :: truncate_children (child_count + 1) rest
    in
    truncate_children 0 children
  in
  let rec truncate ~(depth : int) (json : Yojson.Safe.t) : Yojson.Safe.t =
    match json with
    | `Assoc []
    | `List []
    | `Int _
    | `Intlit _
    | `Float _
    | `Bool _
    | `Null ->
      json
    | `Assoc props ->
      let f (k, v) = (k, truncate ~depth:(depth + 1) v) in
      if depth >=@ max_depth then (
        mark_changed ();
        `Assoc []
      ) else
        `Assoc (truncate_children props max_object_child_count ~f)
    | `List values ->
      let f v = truncate ~depth:(depth + 1) v in
      if depth >=@ max_depth then (
        mark_changed ();
        `List []
      ) else
        `List (truncate_children values max_array_elt_count ~f)
    | `String s -> begin
      match max_string_length with
      | None -> json
      | Some max_string_length ->
        if String.length s <= max_string_length then
          `String s
        else (
          mark_changed ();
          `String (String.sub s 0 max_string_length ^ "...")
        )
    end
    | `Tuple _
    | `Variant _ ->
      json
  in
  truncate ~depth:0 json

let json_truncate_string
    ?(max_string_length : int option)
    ?(max_child_count : int option)
    ?(max_depth : int option)
    ?(max_total_count : int option)
    ?(allowed_total_length : int option)
    ?(if_reformat_multiline = true)
    (s : string) : string =
  if String.length s <=@ allowed_total_length then
    s
  else
    let has_changed = ref false in
    let json = Yojson.Safe.from_string s in
    let truncated_json =
      json_truncate
        ?max_string_length
        ?max_object_child_count:max_child_count
        ?max_array_elt_count:max_child_count
        ?max_depth
        ?max_total_count
        ~has_changed
        json
    in
    if not !has_changed then
      s
    else if if_reformat_multiline then
      Yojson.Safe.pretty_to_string truncated_json
    else
      Yojson.Safe.to_string truncated_json

(************************************************************************)
(* Field access helpers                                                 *)
(************************************************************************)

let get_field accessor on_failure (json : Yojson.Safe.t) =
  Access.(
    let on_failure af = on_failure (access_failure_to_string af) in
    counit_with on_failure (return json >>= accessor))

let get_field_opt accessor (json : Yojson.Safe.t) =
  Access.(to_option (return json >>= accessor))
