<?hh // strict

class :x extends XHPTest { attribute int a @required; }
class :x1 extends :x {}
class :y extends XHPTest { attribute int b @required; }
class :z extends XHPTest { attribute int a @required; attribute int b @required; }

function bar1(): void {
  $x = <x a={1}/>;
  $x1 = <x1 a={1}/>;
  $y = <y b={1}/>;
  $z = <z {...$x} {...$y} />;
  $z = <z {...$x1} {...$y} />;
}

function bar2(bool $b): void {
  if ($b) {
    $x = <x a={1}/>;
  } else {
    $x = <y b={1}/>;
  }
  $z = <z {...$x} />;

  if ($b) {
    $x = <x a={1} />;
  } else {
    $x = <z a={2} b={1} />;
  }
  $z = <y {...$x} />;

  if ($b) {
    $x = <y b={1}/>;
  } else {
    $x = <z a={2} b={1} />;
  }
  $z = <x {...$x} />;

  if ($b) {
    $x = <y b={1}/>;
  } else {
    $x = <z a={2} b={1} />;
  }
  $z = <y {...$x} />;
}
