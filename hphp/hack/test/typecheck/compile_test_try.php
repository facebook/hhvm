<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
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
