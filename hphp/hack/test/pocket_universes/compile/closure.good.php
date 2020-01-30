<?hh // strict
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  enum E {
    case type T;
    case ?(function(): Awaitable<?T>) generator;
  }

  public static function derive<TE as this:@Entry>(
    TE $field, mixed $data) : void {
      $_ = $data as ?this:@Entry:@TE:@T;
    }
}
