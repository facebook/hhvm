<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum A : int {
  V1 = 0;
}

enum B1 : int {
  use A;
  V2 = 1;
}

enum B2 : int {
  V3 = 2;
}

enum C : int {
  use B1, B2;
  V4 = 3;
}


<<__EntryPoint>>
function main() : void {
  $values = A::getValues();
  echo "A::getValues() ---\n";
  foreach ($values as $key => $value) {
    echo "\tkey >$key< has value >$value<\n";
  }
  echo "\n";

  $values = B1::getValues();
  echo "B1::getValues() ---\n";
  foreach ($values as $key => $value) {
    echo "\tkey >$key< has value >$value<\n";
  }
  echo "\n";

  $values = B2::getValues();
  echo "B2::getValues() ---\n";
  foreach ($values as $key => $value) {
    echo "\tkey >$key< has value >$value<\n";
  }
  echo "\n";

  $values = C::getValues();
  echo "C::getValues() ---\n";
  foreach ($values as $key => $value) {
    echo "\tkey >$key< has value >$value<\n";
  }
  echo "\n";
}
