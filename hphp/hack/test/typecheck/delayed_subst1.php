<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.
interface Test extends BaseXXX<int>, C {}

interface BaseXXX<+T as num> {
  public function get(): T;
}

interface C extends BaseXXX<num> {}
