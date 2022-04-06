<?hh

function f(shape('key' => int) $s): void {
  Shapes::keyExists($s,'kex');
}
