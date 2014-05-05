<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class Bob extends Exception {
  
}

function main(): void {
  try {
    throw new Exception('test');
  }
  catch(Bob $e) {
    echo 'Failure: test_try.1';
  }
  catch(Exception $e) {
    echo 'OK';
  }
}
