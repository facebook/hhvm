<?hh

function test<<<__Enforceable>> reify T>(mixed $x): nonnull {
  if ($x is T) {
    //
  } else {
    if ($x is null) {
      hh_show($x); // should be not T & null
    } else {
      hh_show($x); // should be not T & nonnull
      return $x;
    }
  }
  return 0;
}
