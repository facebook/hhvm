(* open Core_kernel *)
[@@@warning "-33"]
open Core_kernel
[@@@warning "+33"]


let visitor = Nast_visitor.iter_with [
  Const_prohibited_check.handler;
  Mutability_check.handler;
  Inout_check.handler;
  Naming_coroutine_check.handler;
  Interface_check.handler;
]

let program = visitor#go
let def = visitor#go_def
