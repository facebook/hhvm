open StdLabels

type t = string

let pp ppf t = Fmt.(quote ~mark:"'" string) ppf t

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
    Fmt.(hovbox @@ list ~sep:comma @@ pair ~sep:(any ": ") pp pp_a) ppf
    @@ bindings t

  let to_string pp_a = Fmt.to_to_string (pp pp_a)

  let print pp_a = pp pp_a Format.std_formatter
end
