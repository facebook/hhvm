<?hh

class a1 {
  public function __call($func, $args) {
    var_dump('a1::__call');
  }
  public function test() {
    a1::foo();
  }
}

<<__EntryPoint>>
function main_1893() {
$obj = new a1;
$obj->test();
}
