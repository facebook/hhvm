<?hh

abstract class AC {
  abstract const type TC as shape('a' => int, 'b' => string);

  public function foo(this::TC $s): void {
    $s['c'] ?? true; // 'c' does not exist in TC
  }
}
