<?hh // strict
class Foobar {
  <<__PHPStdLib>> public mixed $bar;
}


function test(mixed $_) : void {
  $x = new Foobar();
  test($x->bar);
}
