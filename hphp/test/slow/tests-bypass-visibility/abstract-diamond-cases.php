<?hh

class WWWTest {}

abstract class CopiedAbstractParent {
  <<__TestsBypassVisibility>>
  abstract protected function absProt(): string;
}

class CopiedConcreteChild extends CopiedAbstractParent {
  <<__Override, __TestsBypassVisibility>>
  protected function absProt(): string {
    return 'abstract_protected';
  }
}

class CopiedAbstractTest extends WWWTest {
  public function test(CopiedConcreteChild $obj): void {
    echo $obj->absProt()."\n";
  }
}

class DiamondBase {
  <<__TestsBypassVisibility>>
  protected function shared(): string {
    return 'diamond';
  }
}

class DiamondLeft extends DiamondBase {}
class DiamondRight extends DiamondBase {}

class CopiedDiamondTest extends WWWTest {
  public function test(DiamondBase $base, DiamondLeft $left, DiamondRight $right): void {
    echo $base->shared()."\n";
    echo $left->shared()."\n";
    echo $right->shared()."\n";
  }
}

<<__EntryPoint>>
function main(): void {
  (new CopiedAbstractTest())->test(new CopiedConcreteChild());
  (new CopiedDiamondTest())->test(new DiamondBase(), new DiamondLeft(), new DiamondRight());
}
