<?hh

function foo<T as shape('x' => int, ...)>(
  T $arg,
): shape('x' => int, 'y' => int, ...) {
  return Shapes::put($arg, 'y', 10);
}
