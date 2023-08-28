let lock root = ServerFiles.path_of_root root ".watchman_event_watcher_lock"

let log_link root = ServerFiles.path_of_root root ".watchman_event_watcher_log"

let socket_file root =
  ServerFiles.path_of_root root ".watchman_event_watcher_sock"

module Responses = struct
  let unknown_str = "unknown"

  let mid_update_str = "mid_update"

  let settled_str = "settled"

  exception Invalid_response

  type t =
    | Unknown
    | Mid_update
    | Settled

  let to_string = function
    | Unknown -> unknown_str
    | Mid_update -> mid_update_str
    | Settled -> settled_str

  let of_string s =
    match s with
    | str when String.equal str unknown_str -> Unknown
    | str when String.equal str mid_update_str -> Mid_update
    | str when String.equal str settled_str -> Settled
    | _ -> raise Invalid_response
end
