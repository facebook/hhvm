<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

trait MyTraitA {
  enum E {
    :@J(
      type T = float,
      value = 1664
    );
  }
}

class C  {
  use MyTraitA;

  enum E {
    case type T;
    case T value;
    :@K(
      type T = string,
      value = "yo"
    );
  }
}

class D extends C {}

<<__EntryPoint>>
function main() : void {
  echo D:@E::value(:@K)."\n";
  echo D:@E::value(:@J)."\n";
}
