<?hh

class X {
  public $x = 1;
  }
function test() {
  $x = new X;
  var_dump($x->x);
}

<<__EntryPoint>>
function main_666() {
test();
}
