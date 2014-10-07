<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface IUseMemoize {
  <<__Memoize>>
  public function alwaysMemoize(): int;
}
