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

function terminal_catch_can_do_whatever(): int {
  $a = 23;
  try {
    $a = 456;
  } catch (YourException $e) {
    return $a;
  } catch (MyException $e) {
    // It is ok to make this a string, since this clause is terminal
    $a = 'duck';
    return 23904;
  } catch (Exception $e) {
    return $a;
  }
  return $a;
}
