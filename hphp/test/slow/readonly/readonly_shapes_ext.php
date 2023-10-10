<?hh
class Foo{}

<<__EntryPoint>>
async function foo(): Awaitable<void> {
  $x = readonly shape("foo" => 5);
  $y = readonly HH\Readonly\Shapes::toDict($x);
  var_dump($y);
  $z = readonly HH\Readonly\Shapes::idx($x, "foo");
  var_dump($z);
  $null = readonly HH\Readonly\Shapes::idx($x, "bar");
  var_dump($null);
  $w = readonly HH\Readonly\Shapes::toArray($x);
  var_dump($w);
  $a = readonly HH\Readonly\Shapes::at($x, "foo");
  var_dump($a);

}
