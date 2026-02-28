<?hh

class Base {
  function concrete_override() :mixed{ return "base_or"; }
}

class Derived extends Base {
  function concrete_override() :mixed{
    $x = parent::concrete_override();
    $x .= "_derived";
    return $x;
  }
}

function main(Base $b) :mixed{
  $x = $b->concrete_override();
  var_dump($x);
}



<<__EntryPoint>>
function main_func_family_007() :mixed{
main(new Base);
main(new Derived);
}
