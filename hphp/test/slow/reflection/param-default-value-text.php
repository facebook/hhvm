<?hh

function x($x, $y, $a = varray[1,2,3], $b = vec[]) {
  return $x + $y;
}

class F {
  function x($x, $y, $a = varray[1,2,3], $b = vec[]) {
    return $x + $y;
  }
}


<<__EntryPoint>>
function main_param_default_value_text() {
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
