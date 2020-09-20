<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */
function might_throw(): void {}

function naming_finally(): int {
  $a = 23;
  try {
    $a = 456;
    might_throw();
    $b = 789;
  } catch (YourException $e) {
    return $b == 23;
  } catch (Exception $e) {
    return $a == 456;
  } finally {
    echo $b; // b is unknown!
  }
  return $a;
}

class YourException extends Exception {}
