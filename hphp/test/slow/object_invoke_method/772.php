<?hh

class X {
}
class Y {
  public function __invoke() {
}
}

<<__EntryPoint>>
function main_772() {
var_dump(is_callable(new X));
var_dump(is_callable(new Y));
}
