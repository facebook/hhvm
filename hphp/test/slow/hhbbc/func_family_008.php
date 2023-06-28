<?hh

class Base {
  function concrete_override() :mixed{ return "base_or"; }
}

abstract class Derived extends Base {
  abstract function abs():mixed;

  function concrete_override() :mixed{
    $x = parent::concrete_override();
    $x .= "_derived";
    return $x;
  }
}

class MoreDerived extends Derived {
  function abs() :mixed{}
}

function main(Base $b) :mixed{
  $x = $b->concrete_override();
  var_dump($x);
}



<<__EntryPoint>>
function main_func_family_008() :mixed{
main(new Base);
main(new MoreDerived);
}
