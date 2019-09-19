<?hh

class b2 {
}
class c2 extends b2 {
  public function __call($func, $args) {
    echo "c2::__call
";
  }
  public function test1a() {
    b2::foo();
  }
}
function h() {
 var_dump('errored');
}

<<__EntryPoint>>
function main_1891() {
set_error_handler(fun('h'));
$obj = new c2;
$obj->test1a();
var_dump('end');
}
