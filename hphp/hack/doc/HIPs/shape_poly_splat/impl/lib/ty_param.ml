open StdLabels

(* Rigid type parameters: opaque atoms (a string name) introduced by a [forall]
   quantifier and used at base position ([Base.Rigid]) or, via [Spread], at row
   position. *)
type t = string

let equal = String.equal

let pp ppf t = Fmt.(any "#" ++ string) ppf t

let to_string = Fmt.to_to_string pp

let print = pp Format.std_formatter

module Set = struct
  include Set.Make (String)

  let pp ppf t = Fmt.(hovbox @@ braces @@ list ~sep:comma pp) ppf @@ elements t

  let to_string = Fmt.to_to_string pp

  let print = pp Format.std_formatter
end

module Map = struct
  include Map.Make (String)

  let keys t = List.map ~f:fst @@ bindings t

  let keyset t = Set.of_list (keys t)

  let pp pp_a ppf t =
    Fmt.(vbox @@ list ~sep:cut @@ hovbox @@ pair ~sep:(any " => ") pp pp_a) ppf
    @@ bindings t

  let to_string pp_a = Fmt.to_to_string (pp pp_a)

  let print pp_a = pp pp_a Format.std_formatter
end
