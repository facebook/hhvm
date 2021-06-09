(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shallow_decl_defs

module Capacity = struct
  let capacity = 1000
end

module Class = struct
  type t = shallow_class

  let prefix = Prefix.make ()

  let description = "Decl_ShallowClass"
end

module Classes =
  SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (Class)
    (Capacity)

module FilterCapacity = struct
  (* Filters are typically small (on average < 40 bytes) so we can
   * store more than we usually do in our cache *)
  let capacity = 2000
end

module Filter = struct
  type t = BloomFilter.t

  let prefix = Prefix.make ()

  let description = "Decl_MemberFilter"
end

module MemberFilters = struct
  include SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (Filter)
            (FilterCapacity)

  let add
      ( {
          sc_name = (_, cls_name);
          sc_consts;
          sc_typeconsts;
          sc_props;
          sc_sprops;
          sc_constructor;
          sc_static_methods;
          sc_methods;
          _;
        } as cls ) =
    (* We add the name of all (type) constants, properties and methods in
     * order to de-dupe names. This will ensure we have an accurate count
     * when determining the capacity of the bloom filter we need. *)
    let names = HashSet.create () in
    let add_name = HashSet.add names in
    List.iter (fun { scc_name = (_, n); _ } -> add_name n) sc_consts;
    List.iter (fun { stc_name = (_, n); _ } -> add_name n) sc_typeconsts;
    List.iter (fun { sp_name = (_, n); _ } -> add_name n) sc_props;
    List.iter (fun { sp_name = (_, n); _ } -> add_name n) sc_sprops;
    List.iter (fun { sm_name = (_, n); _ } -> add_name n) sc_static_methods;
    List.iter (fun { sm_name = (_, n); _ } -> add_name n) sc_methods;

    (* Constructors need to be handled specially. If a class doesn't define
     * a constructor, but is marked final or consistent construct we will
     * consider there to be a constructor present in the class *)
    Option.iter (fun { sm_name = (_, n); _ } -> add_name n) sc_constructor;
    (match Decl_utils.consistent_construct_kind cls with
    | Typing_defs.Inconsistent -> ()
    | Typing_defs.ConsistentConstruct
    | Typing_defs.FinalClass ->
      add_name Naming_special_names.Members.__construct);

    let filter = BloomFilter.create ~capacity:(HashSet.length names) in
    HashSet.iter names ~f:(fun name ->
        let hashes = BloomFilter.hash name in
        BloomFilter.add filter hashes);
    add cls_name filter
end
