type t = unit
let make () = ()
let initialize_from_saved_state ()
    ~root:_ ~naming_table_saved_state_path:_ ~wait_for_initialization:_ =
  Lwt.return_error "Serverless IDE not available in open-source build"
let serve () = Lwt.return_unit
let destroy () = Lwt.return_unit
let notify_file_changed () _ = ()
let rpc () _ =
  Lwt.return_error "Serverless IDE not available in open-source build"
let get_notifications () =
  failwith "Serverless IDE not available in open-source build"
