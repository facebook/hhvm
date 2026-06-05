<?hh

class WWWTest {}

trait SubclassTrait {
  <<__TestsBypassVisibility>>
  private function trait_priv(): void {}
}

class TraitBase {
  use SubclassTrait;
}

class TraitChild extends TraitBase {}

class TraitSubclassTest extends WWWTest {
  public function test(): void {
    // The private trait method is re-owned to TraitBase and should be
    // accessible on both TraitBase and TraitChild instances from tests.
    (new TraitBase())->trait_priv(); // ok
    (new TraitChild())->trait_priv(); // ok: inherited from TraitBase
  }
}
