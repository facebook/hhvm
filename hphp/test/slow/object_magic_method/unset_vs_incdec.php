<?hh

class C {
  public $foo = 1;
  public $bar = 2;
  function __get($name) {
    if ($name == 'foo') {
      return $this->bar;
    }
  }
}


<<__EntryPoint>>
function main_unset_vs_incdec() {
$c = new C();
unset($c->foo);
var_dump($c->foo++);
var_dump($c);
}
