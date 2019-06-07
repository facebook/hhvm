<?hh

class Test {
  public function __call($name, $args) {
    var_dump($args);
  }
}

<<__EntryPoint>>
function main_746() {
$test = new Test();
$test->test();
}
