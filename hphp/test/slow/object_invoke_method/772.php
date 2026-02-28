<?hh

class X {
}
class Y {
  public function __invoke() :mixed{
}
}

<<__EntryPoint>>
function main_772() :mixed{
var_dump(is_callable(new X));
var_dump(is_callable(new Y));
}
