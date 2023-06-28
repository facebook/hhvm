<?hh

class Direct {
  use TestTrait;
}
class IndirectInheritance extends Direct {
}
trait TestTraitIndirect {
  use TestTrait;
}
class Indirect {
  use TestTraitIndirect;
}
function test() :mixed{
  return "__TRAIT__: <" . __TRAIT__ .
    "> __CLASS__: <" . __CLASS__ .
    "> __METHOD__: <" . __METHOD__ . ">";
}
class NoTraitUsed {
  public static function test() :mixed{
    return "__TRAIT__: <" . __TRAIT__ .
           "> __CLASS__: <" . __CLASS__ .
           "> __METHOD__: <" . __METHOD__ . ">";
  }
}
trait TestTrait {
  public static function test() :mixed{
    return "__TRAIT__: <" . __TRAIT__ .
           "> __CLASS__: <" . __CLASS__ .
           "> __METHOD__: <" . __METHOD__ . ">";
  }
}

<<__EntryPoint>>
function main_1990() :mixed{
echo Direct::test()."\n";
echo IndirectInheritance::test()."\n";
echo Indirect::test()."\n";
echo NoTraitUsed::test()."\n";
echo test()."\n";
}
