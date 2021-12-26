<?hh

enum class E : mixed {
  int A = 42;
}

<<__EntryPoint>>
function main(): void {
  var_dump(vec[E#A][0]);
  $x = E#A;
  var_dump($x);
}
