(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type 'a t =
  | DoesNotExist
  | NotYetAvailable
  | Found of 'a

let of_option_or_doe_not_exist = function
  | Some x -> Found x
  | None -> DoesNotExist

let to_option = function
  | DoesNotExist
  | NotYetAvailable ->
    None
  | Found x -> Some x

let bind x f =
  match x with
  | DoesNotExist -> DoesNotExist
  | NotYetAvailable -> NotYetAvailable
  | Found x -> f x

let map x ~f =
  match x with
  | DoesNotExist -> DoesNotExist
  | NotYetAvailable -> NotYetAvailable
  | Found x -> Found (f x)
