type command_token =
  [ `Text of string
  | `Loc_file
  | `Loc_first_line
  | `Loc_last_line ]

type command_template = command_token list

val subst : command_template -> string -> int -> int -> string

val parse : string -> command_template
