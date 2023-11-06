(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core

type tast = string [@@deriving yojson_of]

type by_names = {
  fun_tasts: tast SMap.t;
  class_tasts: tast SMap.t;
  typedef_tasts: tast SMap.t;
  gconst_tasts: tast SMap.t;
  module_tasts: tast SMap.t;
}
[@@deriving yojson_of]

type t = by_names Relative_path.Map.t [@@deriving yojson_of]

let is_enabled opts = not @@ List.is_empty @@ TypecheckerOptions.dump_tasts opts

let error_while_mapping tasts msg =
  let { Tast.fun_tasts; class_tasts; typedef_tasts; gconst_tasts; module_tasts }
      =
    tasts
  in
  let error _ = msg in
  {
    fun_tasts = SMap.map error fun_tasts;
    class_tasts = SMap.map error class_tasts;
    typedef_tasts = SMap.map error typedef_tasts;
    gconst_tasts = SMap.map error gconst_tasts;
    module_tasts = SMap.map error module_tasts;
  }

let map ctx path (tasts : Tast.by_names) _errors : t =
  if
    List.mem
      (TypecheckerOptions.dump_tasts (Provider_context.get_tcopt ctx))
      (Relative_path.suffix path)
      ~equal:String.equal
  then
    let by_names =
      Timeout.with_timeout
        ~timeout:30
        ~on_timeout:(fun _timings ->
          error_while_mapping tasts "Timed out while dumping the TASTs")
        ~do_:(fun _timeout ->
          let tasts = Tast.map_by_names tasts ~f:(Tast_expand.expand_def ctx) in
          {
            fun_tasts = SMap.map Tast.show_def_with_dynamic tasts.Tast.fun_tasts;
            class_tasts =
              SMap.map Tast.show_def_with_dynamic tasts.Tast.class_tasts;
            typedef_tasts = SMap.map Tast.show_def tasts.Tast.typedef_tasts;
            gconst_tasts = SMap.map Tast.show_def tasts.Tast.gconst_tasts;
            module_tasts = SMap.map Tast.show_def tasts.Tast.module_tasts;
          })
    in
    Relative_path.Map.singleton path by_names
  else
    Relative_path.Map.empty

let reduce_by_names (x : by_names) (y : by_names) : by_names =
  {
    fun_tasts = SMap.union x.fun_tasts y.fun_tasts;
    class_tasts = SMap.union x.class_tasts y.class_tasts;
    typedef_tasts = SMap.union x.typedef_tasts y.typedef_tasts;
    gconst_tasts = SMap.union x.gconst_tasts y.gconst_tasts;
    module_tasts = SMap.union x.module_tasts y.module_tasts;
  }

let reduce (x : t) (y : t) : t =
  Relative_path.Map.union
    ~combine:(fun _key x y -> Some (reduce_by_names x y))
    x
    y

let finalize ~progress:_ ~init_id ~recheck_id (tasts : t) : unit =
  let tast_dir_path =
    Tmp.make_dir_in_tmp ~description_what_for:"tasts" ~root:None
  in
  let subdir_path = Filename.concat tast_dir_path init_id in
  let subdir_path =
    match recheck_id with
    | None -> subdir_path
    | Some recheck_id -> Filename.concat subdir_path recheck_id
  in
  Sys_utils.mkdir_p subdir_path;
  Relative_path.Map.iter tasts ~f:(fun path tasts ->
      let { fun_tasts; class_tasts; typedef_tasts; gconst_tasts; module_tasts }
          =
        tasts
      in
      List.iter
        [fun_tasts; class_tasts; typedef_tasts; gconst_tasts; module_tasts]
        ~f:(fun tasts ->
          SMap.iter
            (fun def_name tast ->
              let file_path =
                let file_name =
                  Printf.sprintf
                    "%s__%s.tast"
                    (Relative_path.suffix path
                    |> String.map ~f:(function
                           | '/' -> '_'
                           | c -> c))
                    (Str.replace_first (Str.regexp {|\\|}) "" def_name)
                in
                Filename.concat subdir_path file_name
              in
              Out_channel.with_file file_path ~f:(fun out ->
                  Out_channel.output_string out tast))
            tasts))
