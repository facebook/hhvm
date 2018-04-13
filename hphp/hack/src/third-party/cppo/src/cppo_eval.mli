(** The type signatures in this module are not yet for public consumption.

    Please don't rely on them in any way.*)

module S : Set.S with type elt = string
module M : Map.S with type key = string

val builtin_env
  : [> `Defun of
         Cppo_types.loc * string * string list *
         [> `Capitalize of Cppo_types.node
         | `Concat of (Cppo_types.node * Cppo_types.node)
         | `Stringify of Cppo_types.node ] list * 'a
    | `Special ] M.t as 'a

val include_inputs
  : extensions:(string, Cppo_command.command_template) Hashtbl.t
  -> preserve_quotations:bool
  -> incdirs:string list
  -> show_exact_locations:bool
  -> show_no_locations:bool
  -> Buffer.t
  -> (([< `Def of Cppo_types.loc * string * Cppo_types.node list * 'a
       | `Defun of Cppo_types.loc * string * string list * Cppo_types.node list * 'a
       | `Special
            > `Def `Defun ]
       as 'b)
        M.t as 'a)
  -> (string * string * (unit -> Lexing.lexbuf) * (unit -> unit)) list -> 'a
