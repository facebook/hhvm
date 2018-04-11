(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module ValueKey = struct
  type t = Typed_value.t
  let compare (x: t) (y: t) = Pervasives.compare x y
end

include Unique_list.WithValue (ValueKey)
