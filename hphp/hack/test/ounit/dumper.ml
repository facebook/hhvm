(* Dump an OCaml value into a printable string.
 * By Richard W.M. Jones (rich@annexia.org).
 * dumper.ml 1.2 2005/02/06 12:38:21 rich Exp
 *)

open Obj

let rec dump r =
  if is_int r then
    string_of_int (magic r : int)
  else
    (* Block. *)
    let rec get_fields acc = function
      | 0 -> acc
      | n ->
        let n = n - 1 in
        get_fields (field r n :: acc) n
    in
    let rec is_list r =
      if is_int r then
        if (magic r : int) = 0 then
          true
        (* [] *)
        else
          false
      else
        let s = size r and t = tag r in
        if t = 0 && s = 2 then
          is_list (field r 1)
        (* h :: t *)
        else
          false
    in
    let rec get_list r =
      if is_int r then
        []
      else
        let h = field r 0 and t = get_list (field r 1) in
        h :: t
    in
    let opaque name =
      (* XXX In future, print the address of value 'r'.  Not possible in
       * pure OCaml at the moment.
       *)
      "<" ^ name ^ ">"
    in
    let s = size r and t = tag r in
    (* From the tag, determine the type of block. *)
    if is_list r then
      (* List. *)
      let fields = get_list r in
      "[" ^ String.concat "; " (List.map dump fields) ^ "]"
    else if t = 0 then
      (* Tuple, array, record. *)
      let fields = get_fields [] s in
      "(" ^ String.concat ", " (List.map dump fields) ^ ")"
    (* Note that [lazy_tag .. forward_tag] are < no_scan_tag.  Not
     * clear if very large constructed values could have the same
     * tag. XXX *)
    else if t = lazy_tag then
      opaque "lazy"
    else if t = closure_tag then
      opaque "closure"
    else if t = object_tag then
      (* Object. *)
      let fields = get_fields [] s in
      let (_clasz, id, slots) =
        match fields with
        | h :: h' :: t -> (h, h', t)
        | _ -> assert false
      in
      (* No information on decoding the class (first field).  So just print
       * out the ID and the slots.
       *)
      "Object #"
      ^ dump id
      ^ " ("
      ^ String.concat ", " (List.map dump slots)
      ^ ")"
    else if t = infix_tag then
      opaque "infix"
    else if t = forward_tag then
      opaque "forward"
    else if t < no_scan_tag then
      (* Constructed value. *)
      let fields = get_fields [] s in
      "Tag"
      ^ string_of_int t
      ^ " ("
      ^ String.concat ", " (List.map dump fields)
      ^ ")"
    else if t = string_tag then
      "\"" ^ String.escaped (magic r : string) ^ "\""
    else if t = double_tag then
      string_of_float (magic r : float)
    else if t = abstract_tag then
      opaque "abstract"
    else if t = custom_tag then
      opaque "custom"
    else
      failwith ("dump: impossible tag (" ^ string_of_int t ^ ")")

let dump v = dump (repr v)
