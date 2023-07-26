<?hh

class C {
  public function f(int $w = 6 as ~int): void {
    var_dump($w);
  }
}

function f(int $i = 8 as ~int): void {
  var_dump($i);
  $j = 9 as ~int;
  var_dump($j);
}

<<__EntryPoint>>
function main(): void {
  $c = new C();
  $c->f();
  f();
}
