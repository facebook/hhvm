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
interface IHasPrivacyPolicy {
  public static function getPrivacyPolicy(): PrivacyPolicyBase<this>;
}

class PrivacyPolicyBase<-T> {}

class PrivacyPolicy<-T> extends PrivacyPolicyBase<T> {}

class ParentPolicy extends PrivacyPolicy<ParentEnt> {}

abstract class ParentEnt implements IHasPrivacyPolicy {
  abstract public static function getPrivacyPolicy(): PrivacyPolicy<this>;
}

class ChildEnt extends ParentEnt {
  public static function getPrivacyPolicy(): PrivacyPolicy<this> {
    return new ParentPolicy();
  }
}
