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

function terminal_catch_can_do_whatever(): int {
  $a = 23;
  try {
    $a = 456;
    might_throw();
  } catch (YourException $e) {
    return $a;
  } catch (MyException $e) {
    // It is ok to make this a string, since this clause is terminal
    $a = 'duck';
    return 23904;
  } catch (Exception $e) {
    return $a;
  } finally {
    $a = 4;
  }

  return $a;
}

class YourException extends Exception {}
class MyException extends Exception {}
