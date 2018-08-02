<?hh // experimental

function foo(int $i): int {
  let add_one = $x ==> $x + 1;
  let apply_twice = $f ==> (($x) ==> $f($f($x)));
  return apply_twice(add_one)(42);
}
