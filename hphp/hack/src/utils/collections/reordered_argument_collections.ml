(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module type Reordered_argument_map_S = sig
  include WrappedMap.S

  val add : 'a t -> key:key -> data:'a -> 'a t

  val filter : 'a t -> f:(key -> 'a -> bool) -> 'a t

  val filter_map : 'a t -> f:('a -> 'b option) -> 'b t

  val fold : 'a t -> init:'b -> f:(key -> 'a -> 'b -> 'b) -> 'b

  val find_opt : 'a t -> key -> 'a option

  val find : 'a t -> key -> 'a

  val iter : 'a t -> f:(key -> 'a -> unit) -> unit

  val map : 'a t -> f:('a -> 'b) -> 'b t

  val mapi : 'a t -> f:(key -> 'a -> 'b) -> 'b t

  val mem : 'a t -> key -> bool

  val remove : 'a t -> key -> 'a t

  val exists : 'a t -> f:(key -> 'a -> bool) -> bool

  val merge :
    'a t -> 'b t -> f:(key -> 'a option -> 'b option -> 'c option) -> 'c t

  val partition : 'a t -> f:(key -> 'a -> bool) -> 'a t * 'a t
end

module Reordered_argument_map (S : WrappedMap.S) = struct
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

module type Reordered_argument_set_S = sig
  include Set.S

  val add : t -> elt -> t

  val filter : t -> f:(elt -> bool) -> t

  val fold : t -> init:'a -> f:(elt -> 'a -> 'a) -> 'a

  val iter : t -> f:(elt -> unit) -> unit

  val mem : t -> elt -> bool

  val remove : t -> elt -> t

  val exists : t -> f:(elt -> bool) -> bool

  val of_list : elt list -> t

  val make_pp :
    (Format.formatter -> elt -> unit) -> Format.formatter -> t -> unit
end

module Reordered_argument_set (S : Set.S) = struct
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
end

module SSet = struct
  include Reordered_argument_set (SSet)

  let pp = SSet.pp

  let show = SSet.show
end

module SMap = struct
  include Reordered_argument_map (SMap)

  let pp = SMap.pp

  let show = SMap.show
end
