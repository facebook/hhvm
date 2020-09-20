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

class D extends C {
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

class E extends C {
  enum E {
    case int x;
    :@S(
      x = 42
    );
    :@S2(
      val = "bli",
      x = 1664
    );
  }
}


class Base {}
class Foo extends Base {
  enum E {
    case string foo;
    :@X(
      foo = "X"
    );
  }
}

<<__EntryPoint>>
function main(): void {
  echo "Members in Foo\n";
  foreach (Foo:@E::Members() as $k) {
    echo $k;
    echo "\n";
  }

  echo "Members in C\n";
  foreach (C:@E::Members() as $k) {
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

  echo "Members in E\n";
  foreach (E:@E::Members() as $k) {
    echo $k;
    echo " ";
    echo E:@E::x($k);
    echo "\n";
  }
}
