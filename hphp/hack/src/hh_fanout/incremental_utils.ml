(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let init_state_dir (state_dir : Path.t) ~(populate_dir : Path.t -> unit) : unit
    =
  Disk.mkdir_p (state_dir |> Path.dirname |> Path.to_string);
  if not (Path.file_exists state_dir) then
    Tempfile.with_tempdir (fun temp_dir ->
        populate_dir temp_dir;
        try
          Disk.rename (Path.to_string temp_dir) (Path.to_string state_dir)
        with
        | Disk.Rename_target_already_exists _
        | Disk.Rename_target_dir_not_empty _ ->
          (* Assume that the directory was initialized by another process
          before us, so we don't need to do anything further. *)
          ())

let make_client_id (client_config : Incremental.client_config) :
    Incremental.client_id =
  let client_id =
    Printf.sprintf
      "client,%s,%d"
      client_config.Incremental.client_id
      (Hashtbl.hash client_config)
  in
  Incremental.Client_id client_id

(** Construct the cursor ID exposed to the user.

For debugging purposes, the `from` and `client_config` fields are also
included in the cursor, even though we could store them in the state and
recover them from the ID.

For convenience during debugging, we try to ensure that cursors are
lexicographically-orderable by the time ordering. For that reason, it's
important that the first field in the cursor ID is the
monotonically-increasing ID.

The choice of `,` as a delimiter is important. Watchman uses `:`, which is
inappropriate for this goal, because the ASCII value of `,` is less than that
of all the numerals, while `:` is greater than that of all the numerals.

Using this delimiter ensures that a string like `cursor,1,foo` is less than a
string like `cursor,10,foo` by the ASCII lexicographical ordering, which is
not true for `cursor:1:foo` vs. `cursor:10:foo`.

Some reasoning about delimiter choice:

  * `-` is likely to appear in `from` strings.
  * `+` would contrast strangely with `-` in `from` strings.
  * `#` is interpreted as a comment in the shell.
  * `$` and `!` may accidentally interpolate values in the shell.
  * `&` launched background processes in Bash.
  * `(`, `)`, `'`, and `"` are usually paired, and have special meaning in
  the shell. Also, in this OCaml comment I have to write this " to close the
  previous double-quote, or this comment is a syntax-error.
  * '/' suggests a hierarchical relationship or an actual file.
  * `%` and `*` look a little strange in my opinion.
  * `.` and  `,` are fine.
*)
let make_cursor_id (id : int) (client_config : Incremental.client_config) :
    Incremental.cursor_id =
  let cursor_id =
    Printf.sprintf
      "cursor,%d,%s,%d"
      id
      client_config.Incremental.client_id
      (Hashtbl.hash client_config)
  in
  Incremental.Cursor_id cursor_id
