<?hh  //strict

/** :ab:cd:text docblock */
final class :ab:cd:text implements XHPChild {
  attribute string color, int width;
}

/** :ab:cd:alpha docblock */
final class :ab:cd:alpha implements XHPChild {
  attribute string name;
}

enum MyEnum: string as string {
  TYPE_A = "A value";
  TYPE_B = "B value";
  TYPE_C = "C value";
}

/** :xhp:enum-attribute docblock */
final class :xhp:enum-attribute implements XHPChild {
  attribute MyEnum enum-attribute, string name;

  public function __construct(
    public darray<string,mixed> $attributes,
    public varray<mixed> $children,
    public string $filename,
    public int $line,
  ) {}
}
