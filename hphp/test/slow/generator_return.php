<?hh

class A {
  public function Gen() :AsyncGenerator<mixed,mixed,void>{
    var_dump($this);
    yield 1; yield 2; yield 3;
    return 11;
  }

  public static function SGen() :AsyncGenerator<mixed,mixed,void>{
    var_dump(static::class);
    yield 4; yield 5; yield 6;
    return 22;
  }
}

function basicGen() :AsyncGenerator<mixed,mixed,void>{
  yield 7; yield 8;
  return 33;
}

function noReturnGen() :AsyncGenerator<mixed,mixed,void>{
  yield 9; yield 10;
}


<<__EntryPoint>>
function main_generator_return() :mixed{
$a = new A();
$g = $a->Gen();
foreach ($g as $num) { var_dump($num); }
var_dump($g->getReturn());

$g2 = A::SGen();
foreach ($g2 as $num) { var_dump($num); }
var_dump($g2->getReturn());

$g3 = basicGen();
foreach ($g3 as $num) { var_dump($num); }
var_dump($g3->getReturn());

$g4 = noReturnGen();
foreach ($g4 as $num) { var_dump($num); }
var_dump($g4->getReturn());
}
