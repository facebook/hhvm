<?hh // strict

async function foo(): Awaitable<void> {
  $x = readonly shape("foo" => 5);
  $y = readonly HH\Readonly\Shapes::toDict($x);
  hh_show($y);
  $z = readonly HH\Readonly\Shapes::idx($x, "foo");
  hh_show($z);
  $null = readonly HH\Readonly\Shapes::idx($x, "bar");
  hh_show($null);
  $w = readonly HH\Readonly\Shapes::toArray($x);
  hh_show($w);
  $a = readonly HH\Readonly\Shapes::at($x, "foo");
  hh_show($a);

  // error, this function returns readonly, must be wrapped
  HH\Readonly\Shapes::toDict($x);
}
