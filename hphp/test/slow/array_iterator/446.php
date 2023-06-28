<?hh

function ex($m) :mixed{
  var_dump('Throwing: '.$m);
  throw new Exception($m);
}
class A {
  public function __construct() {
    var_dump(__METHOD__);
  }
}
class B {
  public $a;
  public function gen() :AsyncGenerator<mixed,mixed,void>{
    ex('die!');
    yield(2);
  }
}
class II {
  private $tn, $tv;
  function __construct($tn, $tv) {
 $this->tn = $tn;
 $this->tv = $tv;
 }
  public function gen() :mixed{
 return new I($this->tn, $this->tv);
 }
}
class JJ {
  private $tn, $tv;
  function __construct($tn, $tv) {
 $this->tn = $tn;
 $this->tv = $tv;
 }
  public function gen() :mixed{
 return new J($this->tn, $this->tv);
 }
}
class KK {
  private $tn, $tv;
  function __construct($tn, $tv) {
 $this->tn = $tn;
 $this->tv = $tv;
 }
  public function gen() :mixed{
 return new K($this->tn, $this->tv);
 }
}
class LL {
  private $tn, $tv;
  function __construct($tn, $tv) {
 $this->tn = $tn;
 $this->tv = $tv;
 }
  public function gen() :mixed{
 return new L($this->tn, $this->tv);
 }
}
class I implements Iterator{
  private $tn, $tv, $i = 0;
  public function gen() :mixed{
 return $this;
 }
  public function __construct($tn, $tv) {
    $this->tn = $tn;
    $this->tv = $tv;
  }
  public function rewind() :mixed{
    var_dump(__METHOD__);
    if ($this->tn == 0) ex(__METHOD__);
    $this->i = 1;
  }
  public function current() :mixed{
    var_dump(__METHOD__);
    return $this->i;
  }
  public function key() :mixed{
    var_dump(__METHOD__);
    return $this->i;
  }
  public function next() :mixed{
    var_dump(__METHOD__);
    if ($this->tn == $this->i) ex(__METHOD__);
    return ++$this->i;
  }
  public function valid() :mixed{
    var_dump(__METHOD__);
    if ($this->tv == $this->i) ex(__METHOD__);
    return $this->i < 10;
  }
}
class J implements IteratorAggregate {
  private $i;
  public function __construct($tn, $tv) {
    $this->i = new I($tn, $tv);
  }
  public function getIterator() :mixed{
 return $this->i;
 }
  public function gen() :mixed{
 return $this;
 }
}
class K implements IteratorAggregate {
  private $tn, $tv;
  public function __construct($tn, $tv) {
    $this->tn = $tn;
    $this->tv = $tv;
  }
  public function getIterator() :mixed{
 return new I($this->tn, $this->tv);
 }
  public function gen() :mixed{
 return $this;
 }
}
class L implements IteratorAggregate {
  public function getIterator() :mixed{
 ex(__METHOD__);
 }
  public function gen() :mixed{
 return $this;
 }
}
function run($n, $tn, $tv) :mixed{
  var_dump('>>>main');
  $a = new A();
  $b = new $n($tn, $tv);
  $b->a = $a;
  try {
    foreach ($b->gen() as $k) {
      var_dump('got '.$k);
    }
  }
 catch(Exception $e) {
    var_dump('Exception: ' . $e->getMessage());
    unset($e);
  }
  unset($b);
  var_dump('<<<main');
}
function test($n, $tn, $tv) :mixed{
  run($n, $tn, $tv);
  var_dump('Done: '.$n);
}
function triple($n) :mixed{
  test($n, 0, 0);
  test($n, -1, 1);
  test($n, 2,-1);
}
function main() :mixed{
  triple('I');
  triple('J');
  triple('K');
  triple('L');
  triple('II');
  triple('JJ');
  triple('KK');
  triple('LL');
  test('B', 0, 0);
}

<<__EntryPoint>>
function main_446() :mixed{
main();
}
