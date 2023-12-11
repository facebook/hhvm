<?hh

function x($x, $y, $a = vec[1,2,3], $b = vec[]) :mixed{
  return $x + $y;
}

class F {
  function x($x, $y, $a = vec[1,2,3], $b = vec[]) :mixed{
    return $x + $y;
  }
}


<<__EntryPoint>>
function main_param_default_value_text() :mixed{
var_dump(
  array_map(
    $x ==> $x->isOptional() ? $x->getDefaultValueText() : '',
    (new ReflectionFunction('x'))->getParameters()
  )
);

var_dump(
  array_map(
    $x ==> $x->isOptional() ? $x->getDefaultValueText() : '',
    (new ReflectionMethod('F', 'x'))->getParameters()
  )
);
}
