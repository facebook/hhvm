<?hh

function hopefully_doesnt_dangle() {
  $x = new HH\Set();
  $x[] = 42;
  $x[] = "hello";
  return $x->toDArray();
}
<<__EntryPoint>> function main(): void {
var_dump(hopefully_doesnt_dangle());
}
