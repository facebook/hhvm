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

class Dude {
  private function blah(): :fbt {
    return <fbt project="blah">Some Text</fbt>;
  }
}

class :fbt {
  attribute string project;
}
