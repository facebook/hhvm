(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module ValueKey = struct
  type t = Typed_value.t
  let compare (x: t) (y: t) = Pervasives.compare x y
end

include Unique_list.WithValue (ValueKey)
