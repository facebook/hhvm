<?hh

function reflect_func($name) :mixed{
  echo "\n", '* ',__FUNCTION__, ": ", $name, "\n";
  $rf = new ReflectionFunction($name);
  var_dump($rf->isVariadic());
  foreach ($rf->getParameters() as $param) {
    reflect_param($param);
  }
}

function reflect_meth($class, $name) :mixed{
  echo "\n", '* ', __FUNCTION__, ": ", $class, '::', $name, "\n";
  $rf = new ReflectionMethod($class, $name);
  var_dump($rf->isVariadic());
  foreach ($rf->getParameters() as $param) {
    reflect_param($param);
  }
}

function reflect_param(ReflectionParameter $param) :mixed{
  var_dump($param->getName());
  echo '  ', 'optional: ', var_export($param->isOptional(), true), "\n";
  echo '  ', 'variadic: ', var_export($param->isVariadic(), true), "\n";
  echo '  ', 'hasDefaultAvail: ',
    var_export($param->isDefaultValueAvailable(), true), "\n";
}

function main() :mixed{
  reflect_func('variadic_only_no_vv');
  reflect_func('variadic_some');
  reflect_meth('C', 'variadic_only');
  reflect_meth('C', 'st_variadic_some');

  echo "\n", '* reflect func (direct construction)', "\n";
  reflect_param(new ReflectionParameter(vec['C', 'variadic_some'], 'v'));
  echo "\n", '* reflect meth (direct construction)', "\n";
  reflect_param(new ReflectionParameter('variadic_some', 'v'));
}



<<__EntryPoint>>
function main_reflection() :mixed{
error_reporting(-1);
require_once __DIR__.'/variadic_funcs.inc';
main();
}
