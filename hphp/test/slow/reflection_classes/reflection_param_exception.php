<?hh

class A {
  public static function bar($baz) :mixed{}
}


// Bad function.
<<__EntryPoint>>
function main_reflection_param_exception() :mixed{
try {
  new ReflectionParameter(1, 'baz');
} catch (ReflectionException $e) {
  echo $e->getMessage() . "\n";
}

// Bad param name.
try {
  new ReflectionParameter(vec['A', 'bar'], 'not-baz');
} catch (ReflectionException $e) {
  echo $e->getMessage() . "\n";
}

// Out of bound param.
new ReflectionParameter(vec['A', 'bar'], 0);
try {
  new ReflectionParameter(vec['A', 'bar'], 1);
} catch (ReflectionException $e) {
  echo $e->getMessage() . "\n";
}
try {
  new ReflectionParameter(vec['A', 'bar'], -1);
} catch (ReflectionException $e) {
  echo $e->getMessage() . "\n";
}

// Bad param type.
try {
  new ReflectionParameter(vec['A', 'bar'], vec[]);
} catch (ReflectionException $e) {
  echo $e->getMessage() . "\n";
}
}
