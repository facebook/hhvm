<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class A1 {
  final public function __clone(int $_): this {
    return $this;
  }
}
