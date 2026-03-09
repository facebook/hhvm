//// defs.php
<?hh

enum MyEnum: int {
  A = 1;
  B = 2;
}

newtype MyEnumWrapper as MyEnum = MyEnum;

//// test.php
<?hh

function test_newtype_exhaustive(MyEnumWrapper $x): void {
  switch ($x) {
    case MyEnum::A:
      return;
    case MyEnum::B:
      return;
  }
}

function test_newtype_nullable_exhaustive(?MyEnumWrapper $x): void {
  switch ($x) {
    case MyEnum::A:
      return;
    case MyEnum::B:
      return;
    case null:
      return;
  }
}
