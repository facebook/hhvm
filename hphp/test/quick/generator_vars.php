<?hh

class logger {
  public static $x = 0;
  private $idx;
  function __construct() {
    $this->idx = self::$x++;
    printf("logger %d constructing\n", $this->idx);
  }
}

function create() :AsyncGenerator<mixed,mixed,void>{
  $x = 5;
  yield $x;
  $s = 'foo';
  yield $s;
  $foo = 1234;
  $z = $foo;
  yield $z;
  yield $foo;
}

function unusedarg($x, $y) :AsyncGenerator<mixed,mixed,void>{
  $z = 5;
  yield dict['x' => $x, 'z' => $z];
  $s = 'foo';
  yield 'almost there';
  $foo = 'inside foo';
  yield dict['foo' => $foo, 's' => $s];
  yield dict['x' => $x, 'y' => $y, 'foo' => $foo, 'z' => $z];
}

function dumpgen($g) :mixed{
  foreach ($g as $v) {
    var_dump($v);
  }
}

function getargs(...$args) :AsyncGenerator<mixed,mixed,void>{
  yield 0xdeadbeef;
  yield $args;
  yield $args[3];
}

function genthrow() :AsyncGenerator<mixed,mixed,void>{
  throw new Exception();
  yield 5;
}

function manylocals() :AsyncGenerator<mixed,mixed,void>{
  $a = 1;
  $b = 2;
  $c = 3;
  $d = 4;
  $e = 5;
  $f = 6;
  $g = 7;
  $h = 8;
  $i = 9;
  $j = 10;
  $k = 11;
  $l = 12;
  $a = yield dict['a' => $a, 'b' => $b, 'c' => $c, 'd' => $d, 'e' => $e, 'f' => $f, 'g' => $g, 'h' => $h, 'i' => $i, 'j' => $j, 'k' => $k, 'l' => $l];
  $b = 0xdeadbeef;
  $c = yield dict['a' => $a, 'b' => $b, 'c' => $c, 'd' => $d, 'e' => $e, 'f' => $f, 'g' => $g, 'h' => $h, 'i' => $i, 'j' => $j, 'k' => $k, 'l' => $l];
  $d = $e = 0xba53b411;
  yield dict['a' => $a, 'b' => $b, 'c' => $c, 'd' => $d, 'e' => $e, 'f' => $f, 'g' => $g, 'h' => $h, 'i' => $i, 'j' => $j, 'k' => $k, 'l' => $l];
}

<<__EntryPoint>> function main(): void {
  dumpgen(create());
  dumpgen(unusedarg(new logger(), 5));
  dumpgen(getargs(1, 2, 3, 4, 5));
  $g = genthrow();
  try {
    $g->next();
  } catch (Exception $e) {}
  try {
    $g->next();
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  $g = manylocals();
  $g->next();
  var_dump($g->current());
  $g->send(new stdClass);
  var_dump($g->current());
  $g->send($g);
  var_dump($g->current());
  $g->next();
  var_dump($g->current());
  var_dump($g->valid());
}
