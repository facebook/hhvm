(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Typing_deps

let report_collision hash sym1 sym2 =
  failwith
    (Printf.sprintf
       "Hash collision detected! Hash: %s; Symbols: %s, %s"
       (Dep.to_hex_string hash)
       (Dep.variant_to_string sym1)
       (Dep.variant_to_string sym2))

let from_nast (nast : Nast.program) : _ Dep.variant DepMap.t =
  let mapping variant = DepMap.singleton (Dep.make variant) variant in
  let mappings variants =
    List.fold variants ~init:DepMap.empty ~f:(fun map variant ->
        let dep = Dep.make variant in
        DepMap.add ~combine:(report_collision dep) dep variant map)
  in
  let visitor =
    object (this)
      inherit [_] Aast.reduce as super

      method zero = DepMap.empty

      method plus = DepMap.union ~combine:report_collision

      method! on_fun_def env fd =
        this#plus
          (mapping (Dep.Fun (snd fd.Aast.fd_name)))
          (super#on_fun_def env fd)

      method! on_method_ cls x =
        this#plus
          (mapping
             (if x.Aast.m_static then
               Dep.SMethod (Option.value_exn cls, snd x.Aast.m_name)
             else
               Dep.Method (Option.value_exn cls, snd x.Aast.m_name)))
          (super#on_method_ cls x)

      method! on_class_ _cls x =
        this#plus
          (mappings
             [
               Dep.Type (snd x.Aast.c_name);
               Dep.Constructor (snd x.Aast.c_name);
               Dep.Extends (snd x.Aast.c_name);
               Dep.AllMembers (snd x.Aast.c_name);
             ])
          (super#on_class_ (Some (snd x.Aast.c_name)) x)

      method! on_class_const cls x =
        this#plus
          (mapping (Dep.Const (Option.value_exn cls, snd x.Aast.cc_id)))
          (super#on_class_const cls x)

      method! on_class_typeconst_def cls x =
        this#plus
          (mapping (Dep.Const (Option.value_exn cls, snd x.Aast.c_tconst_name)))
          (super#on_class_typeconst_def cls x)

      method! on_class_var cls x =
        this#plus
          (mapping
             (if x.Aast.cv_is_static then
               Dep.SProp (Option.value_exn cls, snd x.Aast.cv_id)
             else
               Dep.Prop (Option.value_exn cls, snd x.Aast.cv_id)))
          (super#on_class_var cls x)

      method! on_typedef _cls x =
        this#plus
          (mapping (Dep.Type (snd x.Aast.t_name)))
          (super#on_typedef (Some (snd x.Aast.t_name)) x)

      method! on_gconst cls x =
        this#plus
          (mapping (Dep.GConst (snd x.Aast.cst_name)))
          (super#on_gconst cls x)
    end
  in
  visitor#on_program None nast

let from_nasts nasts =
  List.fold nasts ~init:DepMap.empty ~f:(fun acc nast ->
      DepMap.union ~combine:report_collision (from_nast nast) acc)

let dump nast =
  let map = from_nast nast in
  DepMap.iter
    (fun dep variant ->
      Printf.printf
        "%s %s\n"
        (Dep.to_hex_string dep)
        (Dep.variant_to_string variant))
    map
