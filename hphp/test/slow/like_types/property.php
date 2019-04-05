<?hh

final class C {
  private ~int $x = 1;
  public function setX(mixed $x): this {
    // Like-types are treated as mixed, allowing any property value.
    $this->x = $x;
    var_dump($this->x);
    return $this;
  }
}

<<__EntryPoint>>
function main(): void {
  (new C())
    ->setX(1)
    ->setX(1.5)
    ->setX('foo')
    ->setX(false)
    ->setX(STDIN)
    ->setX(new stdClass())
    ->setX(tuple(1, 2, 3))
    ->setX(shape('a' => 1, 'b' => 2));
}
