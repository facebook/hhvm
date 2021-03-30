<?hh

class :xhp:ham {
  public function __construct(
    darray<string, mixed> $attributes,
    varray<mixed> $children,
    string $file,
    int $line,
  ): void {}
}
function ham(mixed $p): void {
  <xhp:ham>{$p}</xhp:ham>;
}
