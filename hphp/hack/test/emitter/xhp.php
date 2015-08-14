<?hh // strict

// Hacky replacement XHP base class for testing. Modeled after
// :x:composable-element
abstract class :base {
  private Map<string, mixed> $attributes = Map {};
  private Vector<XHPChild> $children = Vector {};

  final public function __construct(KeyedTraversable<string, mixed> $attributes,
                                    Traversable<XHPChild> $children) {
    foreach ($children as $child) {
      $this->children->add($child);
    }
    foreach ($attributes as $key => $value) {
      $this->setAttribute($key, $value);
    }
  }
  final public function setAttribute(string $attr, mixed $value): this {
    /* HH_FIXME[4053]: hh_single_typecheck :( */
    $this->attributes->set($attr, (string)$value);
    return $this;
  }
  final public function getAttribute(string $attr): mixed {
    // Return the attribute if it's there
    /* HH_FIXME[4053]: hh_single_typecheck :( */
    if ($this->attributes->containsKey($attr)) {
      /* HH_FIXME[4053]: hh_single_typecheck :( */
      return $this->attributes->get($attr);
    }
    // Instead get the default
    foreach (static::xhpAttributeDeclaration() as $name => $attr_decl) {
      if ($name === $attr) {
        return $attr_decl[2];
      }
    }
    return null;
  }
  final public function getChildren(): Vector<XHPChild> {
    /* HH_FIXME[4012]: hh_single_typecheck :( */
    return new Vector($this->children);
  }

  protected static function __xhpAttributeDeclaration():
    array<string, array<int, mixed>> {
    return array();
  }
  public static function xhpAttributeDeclaration():
    array<string, array<int, mixed>> {
    return static::__xhpAttributeDeclaration();
  }

  protected function __xhpCategoryDeclaration(): array<string, int> {
    return array();
  }
  public static function xhpCategoryDeclaration(): array<string, int> {
    /* HH_FIXME[4090]: __xhpCategoryDeclaration ought to be static but is not.
     * See https://github.com/facebook/xhp-lib/blob/master/src/core/ComposableElement.php#L283-L285 */
    return static::__xhpCategoryDeclaration();
  }
}
///////////////////////////////////////////////////

class Cns {
  const int X = 12;
}
class A {}

class :foo extends :base {
  category %foo, %bar;

  attribute
    int anoube = 1337,
    string val,
    int cns = Cns::X,
    // Some objects
    A an:obj,
    Awaitable<int> foo,
    arraykey ak,
    // Weird trait thing
    :blah1,
    :blah2;
}
class :lol extends :base {}
class :blah1 extends :base {
  attribute int arg1 = 1;
}
class :blah2 extends :base {
  attribute int arg2 = 2;
}

function escape(string $s): :lol {
  return <lol>{$s}</lol>;
}

function texty(): :lol {
  $x = 'hi';
  return <lol>  hmm  <!-- -->bar   amargh<!-- --> arrrrrrgh{$x}sigh
    welp {$x}
a \\ \n \u \'</lol>;
}

function make(): :foo {
  return <foo val="test">
           <lol>hello</lol>
           <lol>world</lol>
           <lol>testing...</lol>
           <lol>...</lol>
         </foo>;
}

function dump(array<string, mixed> $x): void {
  /* HH_FIXME[2049] */
  /* HH_FIXME[4107] */
  ksort($x);
  var_dump($x);
}

function test(): void {
  $x = make();
  var_dump($x);
  var_dump($x->getChildren());
  var_dump($x->:anoube);
  var_dump($x->:cns);
  var_dump($x->:arg2);
  var_dump($x->:val);

  var_dump(texty());
  var_dump(texty()->getChildren());

  var_dump(escape("wheeeee"));

  dump(:foo::xhpCategoryDeclaration());

  // Testing this only works because none of the fields are marked @required,
  // since we don't implement that yet...
  dump(:foo::xhpAttributeDeclaration());
}
