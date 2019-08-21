<?hh

class a1 {
  public function __call($func, $args) {
    var_dump('a1::__call');
  }
}
class b1 {
  public function test() {
    a1::foo();
  }
}
 function h() {
 var_dump('errored');
}

<<__EntryPoint>>
function main_1894() {
set_error_handler(fun('h'));
$obj = new b1;
$obj->test();
var_dump('end');
}
