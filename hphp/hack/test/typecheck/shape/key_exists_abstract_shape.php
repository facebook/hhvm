<?hh

abstract class AC {
  abstract const type TC as shape('a' => int, 'b' => string);

  public function foo(this::TC $s): void {
    if (Shapes::keyExists($s, 'c')) { // 'c' does not exist in TC
      // stuff
    }
  }
}
