<?hh
function f(~shape("a" => ?shape()) $a): void {
  $x = $a["a"]["a"] ?? null;
}
