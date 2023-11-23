(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module type Map_S = Reordered_argument_collections_sig.Map_S

module Reordered_argument_map (S : WrappedMap.S) :
  Map_S with type key = S.key with type 'a t = 'a S.t = struct
  include S

  let add m ~key ~data = add key data m

  let filter m ~f = filter f m

  let filter_map m ~f = filter_map f m

  let fold m ~init ~f = fold f m init

  let find_opt m k = find_opt k m

  let find m k = find k m

  let iter m ~f = iter f m

  let map m ~f = map f m

  let mapi m ~f = mapi f m

  let mem m v = mem v m

  let remove m v = remove v m

  let exists m ~f = exists f m

  let merge m1 m2 ~f = merge f m1 m2

  let partition m ~f = partition f m
end

module type Set_S = Reordered_argument_collections_sig.Set_S

module Reordered_argument_set (S : Set.S) :
  Set_S with type elt = S.elt with type t = S.t = struct
  include S

  let add s v = add v s

  let filter s ~f = filter f s

  let fold s ~init ~f = fold f s init

  let iter s ~f = iter f s

  let mem s v = mem v s

  let remove s v = remove v s

  let exists s ~f = exists f s

  let of_list l = List.fold_left add empty l

  let make_pp pp fmt x =
    Format.fprintf fmt "@[<hv 2>{";
    let elts = elements x in
    (match elts with
    | [] -> ()
    | _ -> Format.fprintf fmt " ");
    ignore
      (List.fold_left
         (fun sep elt ->
           if sep then Format.fprintf fmt ";@ ";
           let () = pp fmt elt in
           true)
         false
         elts);
    (match elts with
    | [] -> ()
    | _ -> Format.fprintf fmt " ");
    Format.fprintf fmt "}@]"

  (** Find an element that satisfies some boolean predicate.
      No other guarantees are given (decidability, ordering, ...).  *)
  let find_one_opt elts ~f =
    let exception FoundFirst of elt in
    try
      iter ~f:(fun elt -> if f elt then raise (FoundFirst elt)) elts;
      None
    with
    | FoundFirst elt -> Some elt
end

module type SSet_S = Reordered_argument_collections_sig.SSet_S

module SSet = struct
  include Reordered_argument_set (SSet)

  let pp = SSet.pp

  let show = SSet.show
end

module type SMap_S = Reordered_argument_collections_sig.SMap_S

module SMap = struct
  include Reordered_argument_map (SMap)

  let pp = SMap.pp

  let show = SMap.show
end
