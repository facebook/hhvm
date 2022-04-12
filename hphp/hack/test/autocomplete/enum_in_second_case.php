<?hh

enum MyEnum: string {
  TYPE_A = "A value";
  TYPE_B = "B value";
  TYPE_C = "C value";
}

function takes_enum(MyEnum $me): void {
  switch ($me) {
    case MyEnum::TYPE_A:
      break;
    // We should only offer B and C in the completion.
    case AUTO332
  }
}
