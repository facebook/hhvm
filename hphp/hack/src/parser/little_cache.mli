(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type ('k, 'v) t
val memoize : ('k, 'v) t -> ('k -> 'v) -> ('k -> 'v)
val make : 'k -> 'v -> ('k, 'v) t
