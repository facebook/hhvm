<?php
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

namespace HH {
  require_once(__DIR__.SEP.'hacklib_constSetLike.php');
  require_once(__DIR__.SEP.'hacklib_commonImmMutableContainerMethods.php');

  trait HACKLIB_ImmSetLike {
    use HACKLIB_ConstSetLike;
    use HACKLIB_CommonImmMutableContainerMethods;

    public function offsetSet($offset, $value) {
      if (is_null($offset)) {
        throw new \InvalidOperationException(
          'Cannot modify immutable object of type '.get_class($this));
      } else {
        throw new \InvalidOperationException(
          '[] operator cannot be used to modify elements of a Set');
      }
    }

    public function offsetUnset($offset) {
      throw new \InvalidOperationException(
        'Cannot modify immutable object of type '.get_class($this));
    }

    public function immutable() {
      return $this;
    }

    protected function hacklib_isImmutable() {
      return true;
    }
  }
}
