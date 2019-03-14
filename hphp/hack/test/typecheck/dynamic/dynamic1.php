<?hh // partial

function expect_string(string $x): dynamic {
  return $x;
}

function expect_bool(bool $x): dynamic {
  return $x;
}

async function test(dynamic $x, classname<dynamic> $cx, $y): Awaitable<mixed> {
  $y = $x->read_property;
  hh_show($y);
  $y->write_property = 10;
  hh_show($y);
  $y = $x->any_meth();
  hh_show($y);
  $y = $x->any_meth(1, 'f', null);
  hh_show($y);
  $y = $cx::$static_prop;
  hh_show($y);
  $y = $x::$static_prop;
  hh_show($y);
  $cx::$write_prop = '32';
  hh_show($cx);
  $x::$write_prop = null;
  hh_show($x);
  $y = $cx::CONST;
  hh_show($y);
  $y = $x::CONST;
  hh_show($y);
  $y = $cx::static_meth();
  hh_show($y);
  $y = $x::static_meth(true, 19, 'afe');
  hh_show($y);
  $y = $x * 10;
  hh_show($y);
  $y = $x + 10.0;
  hh_show($y);
  $y = expect_string($x.$x);
  hh_show($y);
  $y = expect_string($x.'foo');
  hh_show($y);
  $y = expect_string('foo'.$x);
  hh_show($y);
  $y = expect_string("$x");
  hh_show($y);
  invariant($x is string, "");
  hh_show($x);
  $x = expect_string($x);
  hh_show($x);
  $x[] = 10;
  $x['foo'] = 2;
  hh_show($x);
  $y = $x[0][10]['f'][true];
  hh_show($y);
  $y = -$x;
  hh_show($y);
  $x = expect_bool(!$x);
  hh_show($y);
  hh_show($x);
  $y = await $x;
  hh_show($y);
  list($a, $b, $c) = $x;
  hh_show($a);
  hh_show($b);
  hh_show($c);
  foreach ($x as $y) {
    $x = $y;
  }
  hh_show($x);
  return $x;
}
