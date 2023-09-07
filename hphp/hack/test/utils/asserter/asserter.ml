open Hh_prelude

module type Comparator = sig
  type t

  val to_string : t -> string

  val is_equal : t -> t -> bool
end

module String_comparator = struct
  type t = string

  let to_string x = x

  let is_equal x y = String.equal x y
end

module Int_comparator = struct
  type t = int [@@deriving ord]

  let to_string x = string_of_int x

  let is_equal x y = x = y
end

module Bool_comparator = struct
  type t = bool

  let to_string x = string_of_bool x

  let is_equal x y =
    match (x, y) with
    | (true, true) -> true
    | (false, false) -> true
    | (true, false) -> false
    | (false, true) -> false
end

module Make_deriving_comparator (T : sig
  type t

  val equal : t -> t -> bool

  val show : t -> string
end) =
struct
  type t = T.t

  let to_string = T.show

  let is_equal = T.equal
end

module Hh_json_json_comparator = struct
  type t = Hh_json.json

  let to_string v = Hh_json.json_to_string v

  let is_equal exp actual =
    (* Shortcut by comparing the canonical string representation. *)
    let exp = Hh_json.json_to_string exp in
    let actual = Hh_json.json_to_string actual in
    String.equal exp actual
end

module Process_status_comparator = struct
  type t = Unix.process_status

  let to_string v =
    match v with
    | Unix.WEXITED i -> Printf.sprintf "Unix.WEXITED %d" i
    | Unix.WSIGNALED i -> Printf.sprintf "Unix.WSIGNALED %d" i
    | Unix.WSTOPPED i -> Printf.sprintf "Unix.WSTOPPED %d" i

  let is_equal exp actual =
    match (exp, actual) with
    | (Unix.WEXITED i, Unix.WEXITED j)
    | (Unix.WSIGNALED i, Unix.WSIGNALED j)
    | (Unix.WSTOPPED i, Unix.WSTOPPED j) ->
      Int.equal i j
    | ((Unix.WEXITED _ | Unix.WSIGNALED _ | Unix.WSTOPPED _), _) -> false
end

module Relative_path_comparator = struct
  type t = Relative_path.t

  let to_string = Relative_path.S.to_string

  let is_equal = Relative_path.equal
end

module type Pattern_substitutions = sig
  (** List of key-value pairs. We perform these key to value
   * substitutions in-order.
   *
   * For example, consider the substitions:
   *   [ ("foo", "bar"); ("bar", "world"); ]
   *
   * being appied to the string:
   *   "hello {{foo}}"
   *
   * which gets transformed to:
   *   "hello {bar}"
   *
   * then finally to:
   *   "hello world"
   *
   * Note: in actuality, the keys and values aren't treated as string literals
   * but as a pattern for regex and a template for replacement.
   *)
  val substitutions : (string * string) list
end

(** Comparison between an expected pattern and an actual string. *)
module Pattern_comparator (Substitutions : Pattern_substitutions) = struct
  type t = string

  let apply_substitutions s =
    List.fold
      ~f:(fun acc (k, v) ->
        let re = Str.regexp ("{" ^ k ^ "}") in
        Str.global_replace re v acc)
      ~init:s
      Substitutions.substitutions

  (** Argh, due to the signature of Comparator, the "expected" and
   * "actual" have the same type, even though for this Pattern_comparator
   * we would really like them to be different. We'd like "actual" to
   * be type string, and "epxected" to be type "pattern", and
   * so we can apply the substitutions to only the pattern. But splitting
   * them out into different types for all the modules only because this module
   * needs it isn't really worth it. Oh well. So we treat actual as a pattern
   * as well and apply substitutions - oh well. *)
  let to_string s = apply_substitutions s

  let is_equal expected actual =
    let expected = apply_substitutions expected in
    String.equal expected actual
end

(** Useful for abstracting on option types. *)
module Make_option_comparator (Comp : Comparator) :
  Comparator with type t = Comp.t option = struct
  type t = Comp.t option

  let to_string v =
    match v with
    | None -> "None"
    | Some v -> Comp.to_string v

  let is_equal = Option.equal Comp.is_equal
end

module Make_asserter (Comp : Comparator) : sig
  val assert_equals : Comp.t -> Comp.t -> string -> unit

  val assert_list_equals : Comp.t list -> Comp.t list -> string -> unit

  val assert_option_equals : Comp.t option -> Comp.t option -> string -> unit
end = struct
  let assert_equals exp actual failure_msg =
    if Comp.is_equal exp actual then
      ()
    else
      let () =
        Printf.eprintf
          "Error: assertion failure. Expected: '%s'; But Found: '%s'\n"
          (Comp.to_string exp)
          (Comp.to_string actual)
      in
      let () = Printf.eprintf "Assertion msg: %s\n" failure_msg in
      assert false

  let assert_list_equals exp actual failure_msg =
    if Int.equal (List.length exp) (List.length actual) then
      List.iter2_exn
        ~f:(fun exp actual -> assert_equals exp actual failure_msg)
        exp
        actual
    else
      let () =
        Printf.eprintf
          "assert_list_equals failed. Counts not equal (%d expected, %d actual)\n"
          (List.length exp)
          (List.length actual)
      in
      let exp_strs = List.map ~f:Comp.to_string exp in
      let actual_strs = List.map ~f:Comp.to_string actual in
      let () =
        Printf.eprintf
          "Error: Assertion failure. Expected:\n'%s'\n\n But Found:\n'%s'\n"
          (String.concat ~sep:"\n" exp_strs)
          (String.concat ~sep:"\n" actual_strs)
      in
      let () = Printf.eprintf "Assertion msg: %s" failure_msg in
      assert false

  let assert_option_equals exp actual failure_msg =
    match (exp, actual) with
    | (None, None) -> ()
    | (None, Some v) ->
      Printf.eprintf
        "assert_option_equals failed. Expected None but got Some(%s)\n"
        (Comp.to_string v);
      Printf.eprintf "Assertion msg: %s" failure_msg;
      assert false
    | (Some v, None) ->
      Printf.eprintf
        "assert_option_equals failed. Expected Some(%s) but got None\n"
        (Comp.to_string v);
      Printf.eprintf "Assertion msg: %s" failure_msg;
      assert false
    | (Some exp, Some actual) -> assert_equals exp actual failure_msg
end

module Make_asserter_ord (Ord : sig
  include Comparator

  val compare : t -> t -> int
end) =
struct
  include Make_asserter (Ord)

  let assert_leq ~expected ~actual failure_msg =
    if Ord.compare actual expected <= 0 then
      ()
    else
      let () =
        Printf.eprintf
          "Error: assertion failure. Expected result '%s' to be lesser than or equal to '%s'\n"
          (Ord.to_string actual)
          (Ord.to_string expected)
      in
      let () = Printf.eprintf "Assertion msg: %s\n" failure_msg in
      assert false
end

module Hh_json_json_option_comparator =
  Make_option_comparator (Hh_json_json_comparator)
module Int_option_comparator = Make_option_comparator (Int_comparator)
module String_asserter = Make_asserter (String_comparator)
module Bool_asserter = Make_asserter (Bool_comparator)
module Hh_json_json_asserter = Make_asserter (Hh_json_json_comparator)
module Int_asserter = Make_asserter_ord (Int_comparator)
module Process_status_asserter = Make_asserter (Process_status_comparator)
module Relative_path_asserter = Make_asserter (Relative_path_comparator)
module Type_name_asserter =
  Make_asserter (Make_deriving_comparator (Symbol_name.Type))
module Fun_name_asserter =
  Make_asserter (Make_deriving_comparator (Symbol_name.Fun))
module Const_name_asserter =
  Make_asserter (Make_deriving_comparator (Symbol_name.Const))
