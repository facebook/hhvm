open ServerCommandTypes
open ServerEnv

type t = Unix.file_descr
type client = Timeout.in_channel * out_channel

let provider_from_file_descriptor x = x
let provider_for_test () = failwith "for use in tests only"

let client_from_channel_pair x = x

let sleep_and_check in_fd per_in_fd =
  let l = match per_in_fd with
  | Some fd -> [in_fd ; fd]
  | None -> [in_fd] in
  let ready_fd_l, _, _ = Unix.select l [] [] (0.5) in
  match ready_fd_l with
  | [_; _] -> true, true
  | [fd] -> if (fd <> in_fd) then false, true else true, false
  | _ -> false, false

(** Retrieve channels to client from monitor process. *)
let accept_client parent_in_fd =
  let socket = Libancillary.ancil_recv_fd parent_in_fd in
  (Timeout.in_channel_of_descr socket), (Unix.out_channel_of_descr socket)

let say_hello oc =
  let fd = Unix.descr_of_out_channel oc in
  Marshal_tools.to_fd_with_preamble fd "Hello"

let read_connection_type ic =
  Timeout.with_timeout
    ~timeout:1
    ~on_timeout: (fun _ -> raise Read_command_timeout)
    ~do_: (fun timeout -> Timeout.input_value ~timeout ic)

let read_connection_type (ic, oc) =
  say_hello oc;
  read_connection_type ic

let send_response_to_client (_, oc) response =
  let fd = Unix.descr_of_out_channel oc in
  Marshal_tools.to_fd_with_preamble fd response

let read_client_msg (ic, _) =
  Timeout.with_timeout
    ~timeout:1
    ~on_timeout: (fun _ -> raise Read_command_timeout)
    ~do_: (fun timeout -> Timeout.input_value ~timeout ic)

let get_channels x = x

let is_persistent (_, oc) env =
  let fd = Unix.descr_of_out_channel oc in
  match env.persistent_client_fd with
   | Some p_fd -> fd = p_fd
   | None -> false

let shutdown_client x =
  ServerUtils.shutdown_client x
