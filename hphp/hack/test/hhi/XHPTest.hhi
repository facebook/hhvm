<?hh

abstract class XHPTest {
  public function __construct(
    public darray<string,mixed> $_attributes,
    public varray<mixed> $_children,
    public string $_filename,
    public int $_line_number,
  );

  public function getAttribute(
    string $_attribute,
    mixed $_default = null
  ): mixed;
}
