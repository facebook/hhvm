<?hh

function foo(): vec<int> {
  $a = vec[];
  ($a as nonnull)[] = 1;
  return $a;
}
<<__EntryPoint>> function main(): void {
var_dump(foo());
}
