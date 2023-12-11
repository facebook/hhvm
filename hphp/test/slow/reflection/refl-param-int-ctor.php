<?hh

class C {
  public function m($a, $b) :mixed{ }
}


// same as doing 'a' instead of 0
<<__EntryPoint>>
function main_refl_param_int_ctor() :mixed{
$refl = new ReflectionParameter(vec['C', 'm'], 0);

var_dump($refl->getDeclaringClass()->getName());
var_dump($refl->getDeclaringFunction()->getName());
var_dump($refl->getName());

$refl = new ReflectionParameter(vec['C', 'm'], 1);

var_dump($refl->getDeclaringClass()->getName());
var_dump($refl->getDeclaringFunction()->getName());
var_dump($refl->getName());
}
