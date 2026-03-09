//// defs.php
<?hh

enum MyEnum2: int {
  A = 1;
  B = 2;
}

newtype MyEnumWrapper2 as MyEnum2 = MyEnum2;

//// test.php
<?hh

function test_newtype_nonexhaustive(MyEnumWrapper2 $x): void {
  switch ($x) {
    case MyEnum2::A:
      return;
  }
}
