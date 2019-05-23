type t = unit
let uninitialized = ()
let make_from_saved_state ~root:_ = Lwt.return_unit
let serve () = Lwt.return_unit
let destroy () = Lwt.return_unit
let hover () _ =
  Lwt.return_error "Serverless IDE not available in open-source build"
