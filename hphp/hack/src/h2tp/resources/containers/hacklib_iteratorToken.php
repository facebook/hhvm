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

  class HACKLIB_IteratorToken {
    public function __construct() {
      $this->expired = false;
    }
    public function isExpired() {
      return $this->expired;
    }
    public function isNotExpired() {
      return !$this->expired;
    }
    public function expire() {
      $this->expired = true;
    }
  }
}
