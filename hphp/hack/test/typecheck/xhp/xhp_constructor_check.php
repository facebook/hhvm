<?hh

class :element {
  attribute string colour;

  public function __construct(
    public darray<string,dynamic> $attributes,
    public varray<dynamic> $children,
    public string $file,
    public int $line,
  ) {}
}

function ko(string $colour, string $child): void {
  // There should be two type errors complaining that mixed <: dynamic does not
  // hold. One due to the attributes and the other due to children.
  $elem = <element colour={$colour}>{$child}</element>;
}
