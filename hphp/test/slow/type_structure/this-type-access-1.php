<?hh

abstract class C {
  abstract const type TData = int;
  function __construct() {
    var_dump(type_structure(static::class, 'TData'));
  }
  function f($x) {
    var_dump($x is this::TData);
    var_dump($x is ?this::TData);
    return $this;
  }
}

class A extends C {}
class B extends C {
  const type TData = string;
}

<<__EntryPoint>>
function main() {
  (new A)->f('hi')->f(null);
  (new B)->f('hi')->f(null);
}

