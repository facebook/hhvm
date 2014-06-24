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

abstract class Entish {
  abstract protected static function getPrivacyPolicy(): PrivacyPolicyBase<this>;
}

abstract class PrivacyPolicyBase<T> {}

class MyEnt extends Entish {
  protected static function getPrivacyPolicy(): PrivacyPolicyBase<this> {
    return new MyEntPrivacyPolicy();
  }
}

class MyEntPrivacyPolicy<T as MyEnt> extends PrivacyPolicyBase<T> {}
