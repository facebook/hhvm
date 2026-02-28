<?hh

function aux($fun) :mixed{
  $func = new ReflectionFunction($fun);
  $parameters = $func->getParameters();
  foreach($parameters as $parameter) {
    echo "Name: ", $parameter->getName(), "\n";
    echo "Is passed by reference: ", $parameter->isPassedByReference()?"yes":"no", "\n";
    echo "Can be passed by value: ", $parameter->canBePassedByValue()?"yes":"no", "\n";
    echo "\n";
  }
}

function ufunc(inout $arg1, $arg2) :mixed{}

<<__EntryPoint>>
function main_reflection_parameter_can_be_passed_by_value() :mixed{
aux('ufunc');
}
