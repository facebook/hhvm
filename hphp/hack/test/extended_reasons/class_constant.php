<?hh

class C {
  const string FOO = "foo";
}

function bar(): int {
  $x = C::FOO;
  return $x;
}
