<?hh

enum MyEnum: string {
  TYPE_A = "A value";
  TYPE_B = "B value";
  TYPE_C = "C value";
}

function takes_enum(MyEnum $_): void {}

function demo(): void {
  // We should only offer concrete values here, not e.g. MyEnum::getValues().
  takes_enum(MyEnum::AUTO332);
}
