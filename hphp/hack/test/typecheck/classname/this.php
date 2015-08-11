<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  public static function classname(): classname<this> {
    return static::class;
  }
}
