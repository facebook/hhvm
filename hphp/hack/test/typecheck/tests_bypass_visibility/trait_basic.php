<?hh

class WWWTest {}

trait MyTrait {
  <<__TestsBypassVisibility>>
  private function trait_priv(): void {}

  <<__TestsBypassVisibility>>
  protected function trait_prot(): void {}
}

class UsesTraitClass {
  use MyTrait;
}

class TraitTest extends WWWTest {
  public function test(UsesTraitClass $obj): void {
    $obj->trait_priv(); // ok: bypass visibility in test
    $obj->trait_prot(); // ok: bypass visibility in test
  }
}

class TraitNonTest {
  public function test(UsesTraitClass $obj): void {
    $obj->trait_priv(); // error: not in test context
    $obj->trait_prot(); // error: not in test context
  }
}
