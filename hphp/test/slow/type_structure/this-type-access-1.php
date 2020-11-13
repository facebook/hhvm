<?hh

abstract class C {
  abstract const type TData = int;
  function f($x) {
    var_dump(type_structure(static::class, 'TData'));
    var_dump($x is this::TData);
  }
}

class A extends C {}
class B extends C {
  const type TData = string;
}

<<__EntryPoint>>
function main() {
  (new A)->f('hi');
  (new B)->f('hi');
}

