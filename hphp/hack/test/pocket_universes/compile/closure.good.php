<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class C {
  enum Entry {
    case type T;
    case ?(function(): Awaitable<?T>) generator;
  }

  public static function derive<TE as this:@Entry>(TE $field): void {
    $_ = static:@Entry::generator($field);
  }
}
