(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

module Hack_to_notebook_chunks : sig
  val go : string -> (Notebook_chunk.t list, Notebook_convert_error.t) result
end = struct
  type state =
    | In_chunk of {
        metadata: string;
        chunk_acc_rev: string list;
      }
    | Outside_chunk
    | Found_error of Notebook_convert_error.t

  type t = {
    acc: Notebook_chunk.t list;
    state: state;
  }

  let end_chunk_exn (t : t) metadata chunk_acc_rev : t =
    let contents = chunk_acc_rev |> List.rev |> String.concat ~sep:"\n" in
    match Notebook_chunk.of_hack ~comment:metadata contents with
    | Ok chunk -> { state = Outside_chunk; acc = chunk :: t.acc }
    | Error e -> { state = Found_error e; acc = [] }

  let start_chunk (t : t) (line : string) : t =
    { t with state = In_chunk { metadata = line; chunk_acc_rev = [] } }

  let on_line (t : t) (line : string) : t =
    match t.state with
    | In_chunk { metadata; chunk_acc_rev } ->
      if Notebook_chunk.is_chunk_start_comment line then
        (* The best we can do is guess that ending the chunk was intended *)
        let () =
          Out_channel.output_string
            stderr
            ("Warning: found bad metadata when converting Hack to a notebook: "
            ^ "got start-of-cell metadata before the previous cell ended.\n")
        in
        let t = end_chunk_exn t metadata chunk_acc_rev in
        start_chunk t line
      else if Notebook_chunk.is_chunk_end_comment line then
        end_chunk_exn t metadata chunk_acc_rev
      else
        {
          t with
          state = In_chunk { metadata; chunk_acc_rev = line :: chunk_acc_rev };
        }
    | Outside_chunk ->
      if Notebook_chunk.is_chunk_start_comment line then
        start_chunk t line
      else
        t
    | Found_error e -> { t with state = Found_error e }

  let go (hack : string) :
      (Notebook_chunk.t list, Notebook_convert_error.t) result =
    match
      hack
      |> String.split_lines
      |> List.fold ~init:{ acc = []; state = Outside_chunk } ~f:on_line
    with
    | { state = Found_error e; _ } -> Error e
    | { state = Outside_chunk; acc } -> Ok acc
    | { state = In_chunk _; acc } ->
      let () =
        Out_channel.output_string
          stderr
          ("Warning: found bad metadata when converting Hack to a notebook: "
          ^ "reached the end of the file without finding an end-of-cell marker.\n"
          )
      in
      Ok acc
end

let format_chunk (chunk : Notebook_chunk.t) : Notebook_chunk.t =
  let contents =
    (* Hackfmt refuses to format code unless it starts with "<?hh" *)
    let prefix = "<?hh\n" in
    let unformatted =
      let raw_contents = chunk.Notebook_chunk.contents in
      if String.is_prefix ~prefix raw_contents then
        raw_contents
      else
        prefix ^ raw_contents
    in
    unformatted
    |> Notebook_convert_util.hackfmt
    (*
    * We strip the leading <?hh regardless of whether we added it
    * or the user wrote it.
    * While <?hh is technically allowed in the first notebook cell,
    * it's unidiomatic.
    *)
    |> String.chop_prefix_if_exists ~prefix
  in
  Notebook_chunk.{ chunk with contents }

let hack_to_notebook (hack : string) :
    (Hh_json.json, Notebook_convert_error.t) result =
  let open Result.Let_syntax in
  let* Notebook_level_metadata.{ notebook_number = _; kernelspec } =
    List.take (String.split_lines hack) 50
    |> List.find_map ~f:Notebook_level_metadata.of_comment
    |> Result.of_option
         ~error:
           (Notebook_convert_error.Invalid_input
              {|Could not find notebook-level metadata. Expected a valid comment like: //@bento-notebook:{"notebook_number": "notebook_number", kernelspec: $THE_KERNEL_SPEC_SEE_IPYNB_SPEC}|})
  in
  let* chunks = Hack_to_notebook_chunks.go hack in
  let chunks = List.map ~f:format_chunk chunks in
  let+ ipynb = Ipynb.ipynb_of_chunks chunks ~kernelspec in
  Ipynb.ipynb_to_json ipynb
