(* Assert that the early counter hands off to the concurrent counter without overlap *)

let rec loop prev = function
  | 0 -> ()
  | n ->
    let i = Ident.tmp () in
    assert (prev < i);
    loop i (n - 1)

let () =
  let early = Ident.tmp () in
  (* Initializing shared mem will switch over to the concurrent counter, even
   * if we never connect any workers. *)
  let handle =
    SharedMem.init
      ~num_workers:0
      {
        SharedMem.global_size = 0;
        heap_size = 0;
        dep_table_pow = 0;
        hash_table_pow = 0;
        shm_dirs = [];
        shm_min_avail = 0;
        log_level = 0;
        sample_rate = 0.0;
        compression = 0;
      }
  in
  ignore (handle : SharedMem.handle);

  loop early 10_000;

  ()
