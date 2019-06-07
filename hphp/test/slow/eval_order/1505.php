<?hh

class A {
  public $foo;
  public $bar;
  function q($a) {
    echo $a;
    $this->foo = 9;
    $this->bar = '3';
    return $this;
  }
}

<<__EntryPoint>>
function main_1505() {
$a = new A();
var_dump($a->q('1')->foo + $a->q('2')->bar);
var_dump($a->q('1')->foo - $a->q('2')->bar);
var_dump($a->q('1')->foo / $a->q('2')->bar);
var_dump($a->q('1')->foo * $a->q('2')->bar);
var_dump($a->q('1')->foo % $a->q('2')->bar);
var_dump($a->q('1')->foo << $a->q('2')->bar);
var_dump($a->q('1')->foo >> $a->q('2')->bar);
var_dump($a->q('1')->foo && $a->q('2')->bar);
var_dump($a->q('1')->foo || $a->q('2')->bar);
var_dump($a->q('1')->foo . $a->q('2')->bar);
var_dump($a->q('1')->foo & $a->q('2')->bar);
var_dump($a->q('1')->foo | $a->q('2')->bar);
var_dump($a->q('1')->foo ^ $a->q('2')->bar);
var_dump($a->q('1')->foo == $a->q('2')->bar);
var_dump($a->q('1')->foo === $a->q('2')->bar);
var_dump($a->q('1')->foo != $a->q('2')->bar);
var_dump($a->q('1')->foo !== $a->q('2')->bar);
var_dump($a->q('1')->foo > $a->q('2')->bar);
var_dump($a->q('1')->foo >= $a->q('2')->bar);
var_dump($a->q('1')->foo < $a->q('2')->bar);
var_dump($a->q('1')->foo <= $a->q('2')->bar);
}
