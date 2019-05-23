type t = unit
let uninitialized = ()
let make_from_saved_state ~root:_ = Lwt.return_unit
let serve () = Lwt.return_unit
