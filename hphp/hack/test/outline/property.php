<?hh

class C {
  public ?string
    $foo,
    $bar = "aaa";

  attribute
    string xhp_property @required,
    enum {'id', 'name'} xhp_enum_property = 'id';

  public function __construct(
    $not_a_property,
    public string $implicit_property,
    public int $implicit_property_with_init = 3,
  ) {
  }
}
