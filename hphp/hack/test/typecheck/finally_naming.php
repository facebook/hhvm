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

function naming_finally(): int {
  $a = 23;
  try {
    $a = 456;
    $b = 789;
  } catch (YourException $e) {
    return $b = 23;
  } catch (Exception $e) {
    return $a = 456;
  } finally {
    echo $b; // b is unknown!
  }
  return $a;
}
