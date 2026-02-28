<?hh

abstract class :base {
  // This is necessary because the generated __xhpAttributeDeclaration() has a
  // call to parent::__xhpAttributeDeclaration().
  protected static function __xhpAttributeDeclaration()[] :mixed{
    return dict[];
  }

  public static function xhpAttributeDeclaration()[] :mixed{
    return static::__xhpAttributeDeclaration();
  }
}

class :node1 extends :base {
  attribute
    mixed checkme;
}

class :node2 extends :base {
  attribute
    Exception e,
    mixed checkme;
}

class :node3 extends :base {
  attribute
    int num,
    mixed checkme,
    var beans,
    string cheese;
}

class :node4 extends :base {
  attribute
    int num = 2,
    mixed checkme,
    var beans,
    string cheese = "hi";
}

function mixedFn1(mixed $test) :mixed{ return 'fn'; }
function mixedFn2(string $a1, mixed $v2) :mixed{ return 'fn'; }
<<__EntryPoint>> function main(): void {
echo
  :node1::xhpAttributeDeclaration()['checkme'][0].
  :node2::xhpAttributeDeclaration()['checkme'][0].
  :node3::xhpAttributeDeclaration()['checkme'][0].
  :node4::xhpAttributeDeclaration()['checkme'][0].
  mixedFn1(1).
  mixedFn2('a1', 'hi')."\n";
}
