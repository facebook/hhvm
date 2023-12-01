<?hh
function foo<T as ?Container<Tv>, Tv>(inout T $arr): dynamic {
  return false;
}
function bar(): dynamic {
  return vec[];
}
function qux<T as ?Container<Tv>, Tv>(T $arr): Tv {
  throw new \Exception();
}

function call_refinement(): void {
  $arr = bar();
  /* HH_FIXME[4110] */
  if (foo(inout $arr)) {
    return;
  }
  $whatever = qux($arr);
}
