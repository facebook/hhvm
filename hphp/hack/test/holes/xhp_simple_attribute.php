<?hh
namespace XhpSimple {

  class :element {
    attribute int prop @required;
    public function __construct(
      public darray<string, dynamic> $attributes,
      public varray<dynamic> $children,
      public string $file,
      public int $line,
    ) {}
  }

  function xhp_simple_attribute(string $prop): void {
    /* HH_FIXME[4343] */
    $elem = <element prop={$prop} />;
  }

}
