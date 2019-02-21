(* open Core_kernel *)
let visitor = Nast_visitor.iter_with []

let program = visitor#go
let def = visitor#go_def
