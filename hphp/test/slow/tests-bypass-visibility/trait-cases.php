<?hh

class WWWTest {}

trait CopiedTrait {
  <<__TestsBypassVisibility>>
  private function traitPriv(): string {
    return 'trait_priv';
  }

  <<__TestsBypassVisibility>>
  protected function traitProt(): string {
    return 'trait_prot';
  }
}

class UsesCopiedTrait {
  use CopiedTrait;
}

trait CopiedTraitOne {
  <<__TestsBypassVisibility>>
  private function fromTraitOne(): string {
    return 'trait_one';
  }
}

trait CopiedTraitTwo {
  <<__TestsBypassVisibility>>
  private function fromTraitTwo(): string {
    return 'trait_two';
  }
}

class UsesBothCopiedTraits {
  use CopiedTraitOne;
  use CopiedTraitTwo;
}

trait CopiedSubclassTrait {
  <<__TestsBypassVisibility>>
  private function traitSubclassPriv(): string {
    return 'trait_subclass_priv';
  }
}

class TraitBase {
  use CopiedSubclassTrait;
}

class TraitChild extends TraitBase {}

class CopiedTraitTest extends WWWTest {
  public function test(): void {
    $obj = new UsesCopiedTrait();
    echo $obj->traitPriv()."\n";
    echo $obj->traitProt()."\n";

    echo (new TraitBase())->traitSubclassPriv()."\n";
    echo (new TraitChild())->traitSubclassPriv()."\n";

    $multi = new UsesBothCopiedTraits();
    echo $multi->fromTraitOne()."\n";
    echo $multi->fromTraitTwo()."\n";
  }
}

<<__EntryPoint>>
function main(): void {
  (new CopiedTraitTest())->test();
}
