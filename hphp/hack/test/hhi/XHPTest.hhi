<?hh

abstract class XHPTest {
  public function __construct(
    public darray<string,mixed> $_, // Attributes
    public varray<mixed> $_, // Children
    public string $_, // Filename
    public int $_, // Line number
  );
}
