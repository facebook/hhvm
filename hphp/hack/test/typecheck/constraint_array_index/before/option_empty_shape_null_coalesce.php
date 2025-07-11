<?hh
function f(?shape() $v): int {
  return $v["a"] ?? 0;
}
