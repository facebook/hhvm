<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

trait TestTrait {
  public function trait_method(
    int $a,
    named string $label,
    named bool $enabled = true,
  ): void {}
}

class TraitUser {
  use TestTrait;

  public function regular_method(named int $value): void {}
}

// Trait with requirements
trait RequiresTrait {
  require extends BaseClass;

  public function required_method(named string $name): void {}
}

class BaseClass {
  public function base_method(int $x): void {}
}

class TraitUserWithRequire extends BaseClass {
  use RequiresTrait;
}

function test_traits(): void {
  $obj = new TraitUser();

  // OK: Using trait method with named parameters
  $obj->trait_method(1, label="test", enabled=false);
  $obj->trait_method(2, label="another");

  // OK: Using regular method with named parameters
  $obj->regular_method(value=42);

  // OK
  $obj2 = new TraitUserWithRequire();
  $obj2->required_method(name="test");

  // Error: Missing required named parameter in trait method
  $obj->trait_method(1, enabled=false);

  // Error: Unexpected named parameter in trait method
  $obj->trait_method(1, label="test", unknown=123);

  // Error: Missing required parameter in trait with requirements
  $obj2->required_method();
}
