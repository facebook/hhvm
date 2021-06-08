<?hh

enum class EE : int {
  int Foo = 1;
  int Bar = 2;
}

function get<T>(HH\EnumClass\Label<EE, T> $z) : T {
  return $z;
}

function test(): void {
  get#AUTO332
}
