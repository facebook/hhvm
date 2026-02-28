<?hh
function f(?shape('a' => int) $v): int {
  return $v["a"] ?? 0;
}
