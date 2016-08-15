type t = unit
type client = unit

let provider_from_file_descriptor _ = ()
let provider_for_test _ = ()
let client_from_channel_pair _ = ()

let sleep_and_check _ _ = false, false

let not_implemented () = failwith "not implemented"

let accept_client _ = not_implemented ()

let say_hello _ = not_implemented ()

let read_connection_type _ = not_implemented ()

let send_response_to_client _ _ = not_implemented ()

let read_client_msg _ = not_implemented ()

let get_channels _ =  not_implemented ()

let is_persistent _ _ = not_implemented ()

let shutdown_client _ = not_implemented ()
