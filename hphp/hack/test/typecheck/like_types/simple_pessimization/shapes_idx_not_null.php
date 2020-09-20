<?hh

function f(shape(?'a' => int) $s): void {
  if (Shapes::idx($s, "a") !== null) {
    $s["a"];
  }
}
