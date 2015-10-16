<?hh // strict

function maybe<T>(Container<T> $x): ?T {
  return null;
}

function test(): void {
  // Create Recursive Type Var
  $arr = array();
  for ($i = 1; $i < 10; $i++) {
    $arr[$i] = maybe($arr);
  }
  $recurse = maybe($arr);
  hh_show($recurse);

  // Force Typing_subtype.sub_string to run
  $recurse.'';
}
