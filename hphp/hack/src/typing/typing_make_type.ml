(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

module SN = Naming_special_names
module Reason = Typing_reason

let class_type r name tyl = (r, Tclass ((Reason.to_pos r, name), Nonexact, tyl))

let traversable r ty =
  class_type r SN.Collections.cTraversable [ty]
let keyed_traversable r kty vty =
  class_type r SN.Collections.cKeyedTraversable [kty; vty]
let keyed_container r kty vty =
  class_type r SN.Collections.cKeyedContainer [kty; vty]
let awaitable r ty =
  class_type r SN.Classes.cAwaitable [ty]
let generator r key value send =
  class_type r SN.Classes.cGenerator [key; value; send]
let async_generator r key value send =
  class_type r SN.Classes.cAsyncGenerator [key; value; send]
let async_iterator r ty =
  class_type r SN.Classes.cAsyncIterator [ty]
let async_keyed_iterator r ty1 ty2 =
  class_type r SN.Classes.cAsyncKeyedIterator [ty1; ty2]
let pair r ty1 ty2 =
  class_type r SN.Collections.cPair [ty1; ty2]
let dict r kty vty =
  class_type r SN.Collections.cDict [kty; vty]
let keyset r ty =
  class_type r SN.Collections.cKeyset [ty]
let vec r ty =
  class_type r SN.Collections.cVec [ty]
let container r ty =
  class_type r SN.Collections.cContainer [ty]
let throwable r =
  class_type r SN.Classes.cThrowable []
let datetime r =
  class_type r SN.Classes.cDateTime []
let datetime_immutable r =
  class_type r SN.Classes.cDateTimeImmutable []
let const_vector r ty =
  class_type r SN.Collections.cConstVector [ty]
