<?hh

class WWWTest {}

trait TraitWithBypass {
  <<__TestsBypassVisibility>>
  protected function prot(): void {}

  <<__TestsBypassVisibility>>
  private function priv(): void {}
}

// Overriding protected trait method: ok with the attribute
class GoodOverridesTraitMethod {
  use TraitWithBypass;

  <<__TestsBypassVisibility>>
  protected function prot(): void {}
}

// Overriding protected trait method: error without the attribute
class BadOverridesTraitMethod {
  use TraitWithBypass;

  protected function prot(): void {}
}

// Shadowing private trait method: always an error
class ShadowsTraitMethod {
  use TraitWithBypass;

  private function priv(): void {}
}
