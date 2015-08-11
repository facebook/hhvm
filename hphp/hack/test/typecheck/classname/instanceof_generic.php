<?hh // strict

function f<T>(mixed $x, classname<T> $class): T {
  if (!$x instanceof $class) {
    throw new Exception('');
  }
  hh_show($x);
  return $x;
}
