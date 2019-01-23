<?hh

function foo(): vec<int> {
  $a = vec[];
  ($a as nonnull)[] = 1;
  return $a;
}

var_dump(foo());
