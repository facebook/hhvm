<?hh

function uhoh<reify T>() {
  var_dump(HH\ReifiedGenerics\get_type_structure<T>());
}

class F {
  const type TWat = vec<HelloAlias>;

  function go() {
    uhoh<this::TWat>();
  }
}

<<__EntryPoint>>
function main() {
  include "bad-reified.inc";
  (new F)->go();
}
