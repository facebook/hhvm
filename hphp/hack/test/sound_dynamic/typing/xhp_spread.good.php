<?hh // strict
<<__SupportDynamicType>>
abstract class XHPTest {
  public function __construct(
    public ~darray<string,supportdyn<mixed>> $a, // Attributes
    public ~varray<supportdyn<mixed>> $b, // Children
    public string $c, // Filename
    public int $d, // Line number
  ) { }

  public function getAttribute(
    string $_attribute,
    mixed $_default = null
  ): mixed {
    return null;
  }
}

<<__SupportDynamicType>>
class :other extends XHPTest { }
<<__SupportDynamicType>>
class :baz extends XHPTest {
  attribute ~:other x;
}
<<__SupportDynamicType>>
class :bar {
  // Import all attributes from :baz
  attribute :baz;
}
<<__SupportDynamicType>>
class :foo extends XHPTest {
  // Import all attributes from :bar
  attribute :bar;
}
<<__SupportDynamicType>>
function test1(:foo $obj): :baz {
  $x = $obj->:x;
  $x as nonnull;
  return <baz x={<other {...$x}/>}/>;
}
