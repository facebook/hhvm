<?hh // partial
class C {}
function test($x) : void {
  $y = $x instanceof (C::class);
  var_dump($y);
}

test(new C());
