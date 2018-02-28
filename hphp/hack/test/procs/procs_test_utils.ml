let entry = WorkerController.register_entry_point ~restore:(fun _ -> ())

let try_finalize f x finally y =
  let res = try f x with exn -> finally y; raise exn in
  finally y;
  res

let make_workers n =
  let handle = SharedMem.init GlobalConfig.default_sharedmem_config in
  let workers = MultiWorker.make handle entry n GlobalConfig.gc_control handle in
  SharedMem.connect handle ~is_master:true;
  workers

let cleanup () =
  WorkerController.killall ()
