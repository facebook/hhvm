let lock root = ServerFiles.path_of_root root "watchman_event_watcher_lock"
let log_link root = ServerFiles.path_of_root root "watchman_event_watcher_log"
let socket_file root = ServerFiles.path_of_root root
  "watchman_event_watcher_sock"

let unknown_str = "unknown"
let mid_update_str = "mid_update"
let settled_str = "settled"
