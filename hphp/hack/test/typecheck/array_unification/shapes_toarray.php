<?hh

function f(shape('a' => int) $s): dict<string, int> {
  return Shapes::toArray($s);
}
