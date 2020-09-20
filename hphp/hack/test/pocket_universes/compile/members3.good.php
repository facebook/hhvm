<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class C {
  enum E {
    case string val;
    :@S(
      val = "Hello"
    );
  }
}

class XX extends C {}

class D extends XX {
  enum E {
    :@S2(
      val = "youpi"
    );
  }
  enum F {
    case string fal;
    :@S(
      fal = "blu"
    );
  }
}

<<__EntryPoint>>
function main(): void {

  echo "Members in C\n";
  foreach (C:@E::Members() as $k) {
    echo $k;
    echo "\n";
  }

  echo "Members in XX\n";
  foreach (XX:@E::Members() as $k) {
    echo $k;
    echo "\n";
  }

  echo "Members in D\n";
  echo "From E\n";
  foreach (D:@E::Members() as $k) {
    echo $k;
    echo "\n";
  }
  echo "From F\n";
  foreach (D:@F::Members() as $k) {
    echo $k;
    echo "\n";
  }

}
