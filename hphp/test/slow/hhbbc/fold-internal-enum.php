<?hh

module M;

internal function bar(string $s): string {
  return $s;
}

internal enum class Foo : string {
  string A = bar("abc");
  string B = bar("def");
}

<<__EntryPoint>>
function main() {
  var_dump(Foo::A);
}
