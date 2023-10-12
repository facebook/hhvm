<?hh // strict

<<__SupportDynamicType>>
interface ILegacyEnum<-TIn as supportdyn<mixed> , TOut  as supportdyn<mixed> as TIn> { };
<<__SupportDynamicType>>
abstract class Enum implements ILegacyEnum<this::TInner, this::TInner> {
  abstract const type TInner as arraykey;
}

<<__SupportDynamicType>>
final class F extends Enum {
  const type TInner = string;

  const
    A = 'a';
}

<<__SupportDynamicType>>
abstract class XHPTest {
  public function __construct(
    public ~darray<string,supportdyn<mixed>> $x, // Attributes
    public ~varray<supportdyn<mixed>> $y, // Children
    public string $z, // Filename
    public int $s, // Line number
  ) {}
}

<<__SupportDynamicType>>
class :foo extends XHPTest {
  attribute ~F bar @required;
}

<<__SupportDynamicType>>
function testit(): void {
  <foo bar={F::A}/>;
}
