<?hh
namespace XhpSpread {

  class :element {
    attribute int prop1;
    attribute int prop3;
    public function __construct(
      public darray<string, dynamic> $attributes,
      public varray<dynamic> $children,
      public string $file,
      public int $line,
    ) {}
  }
  class :element2 {
    attribute string prop1;
    attribute int prop2;
    public function __construct(
      public darray<string, dynamic> $attributes,
      public varray<dynamic> $children,
      public string $file,
      public int $line,
    ) {}
  }

  function xhp_spread_attribute(:element2 $props): void {
    /* HH_FIXME[4343] */
    $elem = <element {...$props} />;
  }

}
