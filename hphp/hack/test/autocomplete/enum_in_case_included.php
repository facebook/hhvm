<?hh

enum MyEnumParent: string {
  TYPE_A = "A value";
  TYPE_B = "B value";
}

enum MyEnum: string {
  use MyEnumParent;
  TYPE_C = "C value";
  TYPE_D = "D value";
}

function takes_enum(MyEnum $me): void {
  switch ($me) {
    case AUTO332
  }
}
