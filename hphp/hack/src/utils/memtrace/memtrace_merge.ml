(*
 * (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

open Hh_prelude

let usage () =
  Printf.printf
    ("Usage: memtrace_merge *.ctf > merged.ctf -- this will merge memtrace logs.\n"
    ^^ "Note: a bug means that timestamps in the merged log are incorrect.")

let merge_memtraces srcs dst =
  let open Memtrace.Trace in
  let fd = Unix.openfile dst [Unix.O_RDWR; Unix.O_CREAT; Unix.O_TRUNC] 0o666 in
  (* Memtrace requires us to claim a unique pid for each trace. We have to lie; we'll pick this one. *)
  let pid = Int64.of_int (Unix.getpid ()) in
  (* We'll pick out the single earliest memtrace, and all other timestamps will be relative to this. *)
  let infos =
    List.map srcs ~f:(fun filename ->
        let reader = Reader.open_ ~filename in
        let info = Reader.info reader in
        Reader.close reader;
        (filename, info))
    |> List.sort ~compare:(fun (_, info1) (_, info2) ->
           Float.compare
             (Timestamp.to_float info1.Info.start_time)
             (Timestamp.to_float info2.Info.start_time))
  in
  begin
    match infos with
    | [] -> ()
    | (_, info) :: _ ->
      let info = { info with Info.pid } in
      let writer = Writer.create fd ~getpid:(fun () -> info.Info.pid) info in
      List.iter infos ~f:(fun (src, _src_info) ->
          let obj = Obj_id.Tbl.create 100 in
          let reader = Reader.open_ ~filename:src in
          Reader.iter ~parse_backtraces:true reader (fun _time_delta ev ->
              (* TODO: I can't figure out time. See https://github.com/janestreet/memtrace/issues/14 *)
              let time = info.Info.start_time in
              match ev with
              | Event.Alloc
                  {
                    obj_id = relative_id;
                    length;
                    nsamples;
                    source;
                    backtrace_buffer;
                    backtrace_length;
                    common_prefix = _;
                  } ->
                (* I don't know why this should be reversed, but I see that the memtrace source code does it *)
                let btrev =
                  Array.init backtrace_length ~f:(fun i ->
                      backtrace_buffer.(backtrace_length - 1 - i))
                in
                let decode_callstack_entry =
                  Reader.lookup_location_code reader
                in
                let absolute_id =
                  Writer.put_alloc
                    writer
                    time
                    ~length
                    ~nsamples
                    ~source
                    ~callstack:btrev
                    ~decode_callstack_entry
                in
                Obj_id.Tbl.add obj relative_id absolute_id
              | Event.Promote relative_id ->
                let absolute_id = Obj_id.Tbl.find obj relative_id in
                Writer.put_promote writer time absolute_id
              | Event.Collect relative_id ->
                let absolute_id = Obj_id.Tbl.find obj relative_id in
                Writer.put_collect writer time absolute_id);
          Reader.close reader);
      Writer.flush writer;
      ()
  end;
  Unix.close fd;
  ()

let () =
  if
    Array.length Sys.argv = 1
    || (Array.length Sys.argv = 2 && String.equal Sys.argv.(1) "--help")
  then
    usage ()
  else
    let srcs = List.drop (Array.to_list Sys.argv) 1 in
    (* The ctf-writer requires a seekable output. We'll write to a tempfile, then cat it, then delete it. *)
    let dst = Stdlib.Filename.temp_file "memtrace.merged." ".ctf" in
    Utils.try_finally
      ~f:(fun () ->
        merge_memtraces srcs dst;
        Printf.printf "%s" (Sys_utils.cat dst))
      ~finally:(fun () -> Sys_utils.unlink_no_fail dst)
