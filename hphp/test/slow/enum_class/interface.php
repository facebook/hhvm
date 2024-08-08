<?hh

enum class E: int {
  int A = 42;
  int B = 43;
}

<<__EntryPoint>>
function main(): void {
  var_dump((new ReflectionClass(E::class))->getInterfaceNames());
}
