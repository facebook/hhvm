<?hh

function uhoh<reify T>() :mixed{
  var_dump(HH\ReifiedGenerics\get_type_structure<T>());
}

class F {
  const type TWat = vec<HelloAlias>;

  function go() :mixed{
    uhoh<this::TWat>();
  }
}

<<__EntryPoint>>
function main() :mixed{
  include "bad-reified.inc";
  (new F)->go();
}
