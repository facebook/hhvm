<?hh

trait Trait_1 {
  int $prop_from_Trait_1 = 0;

  static int $static_prop_from_Trait_1 = 1;

  function method_from_trait_1(): void {}

  static function static_method_from_Trait_1(): void {}
}

trait Trait_2 {
  use Trait_1;

  int $prop_from_Trait_2 = 0;

  static int $static_prop_from_Trait_2 = 1;

  function method_from_trait_2(): void {}

  static function static_method_from_Trait_2(): void {}
}

trait Trait_3 {
  use Trait_2;
  //    ^ type-hierarchy-at-caret
}
