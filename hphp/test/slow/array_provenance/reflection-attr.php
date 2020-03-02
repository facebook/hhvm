<?hh

<<Zingy("Quux")>>
class Foo {
  public static $doop = 40;
  public function __construct(public $murr) {}
}

<<Blorf(42), Blarghgh>>
function zorp();

function attribute_contents($attrs) {
  foreach ($attrs as $v) {
    var_dump($v);
    var_dump(HH\get_provenance($v));
    echo "---------\n";
  }
}

<<__EntryPoint>>
function main() {
  attribute_contents(new ReflectionClass(Foo::class)
                     ->getAttributesNamespaced());
  attribute_contents(new ReflectionClass(Foo::class)
                     ->getAttributesRecursiveNamespaced());
  attribute_contents(new ReflectionClass(Foo::class)
                     ->getProperties()[0]
                     ->getAttributesNamespaced());
  attribute_contents(new ReflectionClass(Foo::class)
                     ->getProperties()[1]
                     ->getAttributesNamespaced());
  attribute_contents(new ReflectionFunction("zorp")
                     ->getAttributesNamespaced());
}
