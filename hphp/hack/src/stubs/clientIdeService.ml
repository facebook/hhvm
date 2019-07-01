type t = unit
let make () = ()
let initialize_from_saved_state () ~root:_ =
  Lwt.return_error "Serverless IDE not available in open-source build"
let serve () = Lwt.return_unit
let destroy () = Lwt.return_unit
let notify_file_changed () _ = ()
let hover () ~file_input:_ ~line:_ ~char:_ =
  Lwt.return_error "Serverless IDE not available in open-source build"
let completion () _ =
  Lwt.return_error "Serverless IDE not available in open-source build"
let go_to_definition () ~file_input:_ ~line:_ ~char:_ =
  Lwt.return_error "Serverless IDE not available in open-source build"
let resolve () _ =
  Lwt.return_error "Serverless IDE not available in open-source build"
