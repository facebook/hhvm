<?hh

<<__SupportDynamicType>>
abstract class XHPTest {
  public function __construct(
    public ~darray<string,supportdyn<mixed>> $_, // Attributes
    public ~varray<supportdyn<mixed>> $_, // Children
    public ~string $_, // Filename
    public ~int $_, // Line number
  );
}
