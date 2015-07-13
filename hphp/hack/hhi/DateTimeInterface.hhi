<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

interface DateTimeInterface {

  // Methods
  public function diff(DateTimeInterface $datetime2, $absolute = false);
  public function format($format);
  public function getOffset();
  public function getTimestamp();
  public function getTimezone();

}
