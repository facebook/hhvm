(*
 * Tries to get the logged in username via various ways, consecutively
 * as each one fails.
 *)
val get_logged_in_username : unit -> string option
