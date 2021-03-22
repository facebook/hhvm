(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module BitSet = struct
  type t = bytes

  type idx = {
    bit_idx: int;
    byte_idx: int;
  }

  let zero = Char.unsafe_chr 0

  let create number_of_bits =
    if number_of_bits = 0 then
      Bytes.empty
    else
      (* Round up to nearest byte *)
      let number_of_bytes = (number_of_bits + 7) / 8 in
      Bytes.make number_of_bytes zero

  let index i =
    {
      (* i / 8 *)
      byte_idx = Int.shift_right_logical i 3;
      (* i % 8 *)
      bit_idx = Int.logand i 7;
    }

  (* test if the ith bit is set in the byte *)
  let is_set byte i = byte land (1 lsl i) <> 0

  (* sets the ith bit in the byte *)
  let set byte i = byte lor (1 lsl i)

  (* checks if the ith bit is set in the bitset *)
  let mem b i =
    let i = index i in
    let byte = Bytes.get_uint8 b i.byte_idx in
    is_set byte i.bit_idx

  let set (b : t) i =
    let i = index i in
    let byte1 = Bytes.get_uint8 b i.byte_idx in
    let byte = set byte1 i.bit_idx in
    Bytes.set_uint8 b i.byte_idx byte

  let length b = Bytes.length b * 8
end

type t = BitSet.t

type elt = {
  h1: int;
  h2: int;
  h3: int;
  h4: int;
  h5: int;
  h6: int;
  h7: int;
}

(* Builds a Bloomfilter that will maintain a false positive rate of ~1%
 * for the given capacity
 *)
let create ~(capacity : int) : t =
  (* The smallest filter we use will be 64 bits. This should achieve a false positive < 1%
   * if there are less than 7 items to store. Otherwise we compute roughly how many bits
   * we need to maintain ~1% false positive rate for the given capacity
   *)
  let number_of_bits =
    if capacity = 0 then
      0
    else if capacity < 7 then
      64
    else
      (* see: https://en.wikipedia.org/wiki/Bloom_filter#Optimal_number_of_hash_functions
        number of bits = -n * ln error_rate / (ln 2)^2
      *)
      let cap = float_of_int capacity in
      let error_rate = 0.01 in
      let n = ~-.cap *. (log error_rate /. (log 2. *. log 2.)) in
      int_of_float @@ ceil n
  in
  BitSet.create number_of_bits

(* The most space efficient bloom filter uses ~7 hashes. Since we will use these same hashes
 * to examine potentially several bloom filters we store them in a record so they can be
 * reused
 *)
let hash (s : string) : elt =
  {
    h1 = Hashtbl.seeded_hash 1 s;
    h2 = Hashtbl.seeded_hash 2 s;
    h3 = Hashtbl.seeded_hash 3 s;
    h4 = Hashtbl.seeded_hash 4 s;
    h5 = Hashtbl.seeded_hash 5 s;
    h6 = Hashtbl.seeded_hash 6 s;
    h7 = Hashtbl.seeded_hash 7 s;
  }

let add (bf : t) (hashes : elt) : unit =
  match BitSet.length bf with
  | len when len > 0 ->
    let set h = BitSet.set bf (h mod len) in
    set hashes.h1;
    set hashes.h2;
    set hashes.h3;
    set hashes.h4;
    set hashes.h5;
    set hashes.h6;
    set hashes.h7
  | _ -> ()

let mem (bf : t) (hashes : elt) : bool =
  match BitSet.length bf with
  | len when len > 0 ->
    let mem h = BitSet.mem bf (h mod len) in
    mem hashes.h1
    && mem hashes.h2
    && mem hashes.h3
    && mem hashes.h4
    && mem hashes.h5
    && mem hashes.h6
    && mem hashes.h7
  | _ -> false
