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

trait MyTraitB {
  enum E {
    :@X(
      type T = float,
      value = 42
    );
  }
}

class C  {
  use MyTraitA;
  use MyTraitB;

  enum E {
    case type T;
    case T value;
    :@K(
      type T = string,
      value = "yo"
    );
  }
}

<<__EntryPoint>>
function main() : void {
  echo C:@E::value(:@K)."\n";
  echo C:@E::value(:@J)."\n";
  echo C:@E::value(:@X)."\n";
  echo C:@E::value(:@X)."\n";
  echo C:@E::value(:@K)."\n";
}
