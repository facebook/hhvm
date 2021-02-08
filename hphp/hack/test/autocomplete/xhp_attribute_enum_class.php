<?hh

enum MyEnum: string as string {
  TYPE_A = "A value";
  TYPE_B = "B value";
  TYPE_C = "C value";
}

class :foo:bar {
  attribute MyEnum my-attribute;
}

function main(): void {
  <foo:bar my-attribute={AUTO332}
}
