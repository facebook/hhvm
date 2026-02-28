<?hh

class A {
  public $foo;
  public $bar;
  function q($a) :mixed{
    echo $a;
    $this->foo = 9;
    $this->bar = '3';
    return $this;
  }
}

<<__EntryPoint>>
function main_1505() :mixed{
$a = new A();
var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic($a->q('1')->foo) + HH\Lib\Legacy_FIXME\cast_for_arithmetic($a->q('2')->bar));
var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic($a->q('1')->foo) - HH\Lib\Legacy_FIXME\cast_for_arithmetic($a->q('2')->bar));
var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic($a->q('1')->foo) / HH\Lib\Legacy_FIXME\cast_for_arithmetic($a->q('2')->bar));
var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic($a->q('1')->foo) * HH\Lib\Legacy_FIXME\cast_for_arithmetic($a->q('2')->bar));
var_dump((int)($a->q('1')->foo) % (int)($a->q('2')->bar));
var_dump((int)($a->q('1')->foo) << (int)($a->q('2')->bar));
var_dump((int)($a->q('1')->foo) >> (int)($a->q('2')->bar));
var_dump($a->q('1')->foo && $a->q('2')->bar);
var_dump($a->q('1')->foo || $a->q('2')->bar);
var_dump($a->q('1')->foo . $a->q('2')->bar);
var_dump((int)$a->q('1')->foo & (int)$a->q('2')->bar);
var_dump((int)$a->q('1')->foo | (int)$a->q('2')->bar);
var_dump((int)$a->q('1')->foo ^ (int)$a->q('2')->bar);
var_dump($a->q('1')->foo == $a->q('2')->bar);
var_dump($a->q('1')->foo === $a->q('2')->bar);
var_dump($a->q('1')->foo != $a->q('2')->bar);
var_dump($a->q('1')->foo !== $a->q('2')->bar);
var_dump(HH\Lib\Legacy_FIXME\gt($a->q('1')->foo, $a->q('2')->bar));
var_dump(HH\Lib\Legacy_FIXME\gte($a->q('1')->foo, $a->q('2')->bar));
var_dump(HH\Lib\Legacy_FIXME\lt($a->q('1')->foo, $a->q('2')->bar));
var_dump(HH\Lib\Legacy_FIXME\lte($a->q('1')->foo, $a->q('2')->bar));
}
