<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

trait MyTraitC {
  enum E {
    :@R(
      type T = float,
      value = 1792
    );
  }
}

trait MyTraitA {
  use MyTraitC;
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
//  use MyTraitA;
//  use MyTraitB;

  enum E {
    case type T;
    case T value;
    :@K(
      type T = string,
      value = "yo"
    );
  }
}

class D extends C {
  use MyTraitA;
  use MyTraitB;
  enum E {
    :@Y(
      type T = float,
      value = 44
    );
  }
}

<<__EntryPoint>>
function main() : void {
  foreach (D:@E::Members() as $member) {
    echo $member.": ".(D:@E::value($member))."\n";
  }
  foreach (D:@E::Members() as $member) {
    echo $member.": ".(D:@E::value($member))."\n";
  }
}
