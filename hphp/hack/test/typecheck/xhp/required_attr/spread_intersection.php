<?hh

class :x extends XHPTest {
  attribute int a @required;
}
class :y extends XHPTest {
  attribute int b @required;
}
class :z extends XHPTest {
  attribute int a @required;
  attribute int b @required;
}

function bar1(dynamic $d, bool $b): void {
  $x = $d;
  $x as nonnull;
  if ($b) $x = <x a={1} />;
  $y = <y b={1} />;
  $z = <z {...$x} {...$y} />;
}
