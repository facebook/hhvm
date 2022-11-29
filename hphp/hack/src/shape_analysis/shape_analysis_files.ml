open Hh_prelude
module T = Shape_analysis_types

type state = {
  entry_out_channel: Out_channel.t;
  worker: int;
}

let constraints_file_extension = "dmpc"

let state_opt = ref None

let constraints_filename ~constraints_dir ~worker =
  Filename.concat
    constraints_dir
    (Format.sprintf "%d.%s" worker constraints_file_extension)

let get_state ~constraints_dir ~worker =
  let create () =
    let s =
      let entry_out_channel =
        Out_channel.create ~append:true
        @@ constraints_filename ~constraints_dir ~worker
      in
      { entry_out_channel; worker }
    in
    state_opt := Some s;
    s
  in
  match !state_opt with
  | Some s when s.worker = worker -> s
  | Some s ->
    Out_channel.close s.entry_out_channel;
    create ()
  | None -> create ()

let persist channel (entry : T.ConstraintEntry.t) =
  (* note: We don't marshal closures because we want to be able to unmarshal from a distinct executable. *)
  let flags = [] in
  try Marshal.to_channel channel entry flags with
  | Invalid_argument msg ->
    let msg =
      Format.sprintf
        "%s. This error is likely caused by trying to Marshal a closure, which is likely caused by forgetting to strip `Typing_reason.t_`s from `locl_ty`s"
        msg
    in
    raise @@ Invalid_argument msg

let write_constraints ~constraints_dir ~worker entry : unit =
  let { entry_out_channel; _ } = get_state ~constraints_dir ~worker in
  persist entry_out_channel entry

let next_entry channel buf : T.ConstraintEntry.t option =
  Buffer.reset buf;
  match In_channel.input_buffer channel buf ~len:Marshal.header_size with
  | None -> None
  | Some () ->
    let data_size = Marshal.data_size (Buffer.contents_bytes buf) 0 in
    Option.value_exn @@ In_channel.input_buffer channel buf ~len:data_size;
    let entry : T.ConstraintEntry.t =
      Marshal.from_bytes (Buffer.contents_bytes buf) 0
    in
    Buffer.reset buf;
    Some entry

let read_entries_by_grain ~constraints_file ~same_grain :
    T.ConstraintEntry.t list Sequence.t =
  let channel = In_channel.create constraints_file in
  let capacity_guess = Int.pow 2 19 (* based on logging Marshal.total_size *) in
  let buf = Buffer.create capacity_guess in
  let rec read (prev_entry_opt, entries) =
    match next_entry channel buf with
    | None ->
      if List.is_empty entries then
        None
      else
        Some (entries, (prev_entry_opt, []))
    | Some entry ->
      let is_same_grain =
        match prev_entry_opt with
        | None -> true
        | Some prev -> same_grain prev entry
      in
      if is_same_grain then
        read @@ (Some entry, entry :: entries)
      else
        Some (entries, (Some entry, [entry]))
  in
  Sequence.unfold ~init:(None, []) ~f:read

let read_entries_by_source_file ~constraints_file =
  let same_grain entry1 entry2 =
    T.ConstraintEntry.(
      Relative_path.equal entry1.source_file entry2.source_file)
  in
  read_entries_by_grain ~constraints_file ~same_grain

let read_entries_by_callable ~constraints_file =
  let same_grain _ _ = false in
  read_entries_by_grain ~constraints_file ~same_grain

let flush () =
  Option.iter !state_opt ~f:(fun { entry_out_channel; _ } ->
      Out_channel.flush entry_out_channel)
