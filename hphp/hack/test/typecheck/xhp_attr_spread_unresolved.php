<?hh // strict

class :foo extends XHPTest { }
class :baz extends XHPTest {
  attribute string name;
}

function f(mixed $z): void {
  $x = <foo />;
  if ($z === 1) {
    $x = vec[];
  } else if ($z === 2) {
    $x = <baz />;
  }
  // $x might be a vec, so this is not legal
  <baz name="baz" {...$x} />;
}
