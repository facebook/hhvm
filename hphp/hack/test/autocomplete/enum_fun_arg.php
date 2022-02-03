<?hh

enum MyEnum: string {
  TYPE_A = "A value";
  TYPE_B = "B value";
  TYPE_C = "C value";
}

function takes_enum(MyEnum $_): void {}

function demo(): void {
  takes_enum(AUTO332);
}
