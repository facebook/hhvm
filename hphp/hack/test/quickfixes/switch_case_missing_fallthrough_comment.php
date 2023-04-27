<?hh

enum MyEnum: int {
  X = 1;
  Y = 2;
  Z = 3;
}

function foo(MyEnum $me): bool {
  switch ($me) {
    case MyEnum::X:
      return true;
    case MyEnum::Y:
      echo "";
    case MyEnum::Z:
      return false;
  }
}
