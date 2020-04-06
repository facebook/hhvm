<?hh // strict
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  enum Entry {
    case type T;
    case ?(function(): Awaitable<?T>) generator;
  }

  public static function derive<TE as this:@Entry>(
    TE $field) : void {
    $_ = static:@Entry::generator($field);
  }
}
