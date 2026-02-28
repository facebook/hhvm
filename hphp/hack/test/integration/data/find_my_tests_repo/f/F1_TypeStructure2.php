<?hh

class F1_TypeStructure2 {
  public static function makesTypeStruture1(): int {
    $ts = type_structure(F1_ClassWithTypeConst2::class, 'TBar');

    return 3;
  }

  // Caling this function should not get our test selected.
  public static function makesTypeStruture2(): int {
    $ts =
      type_structure(F1_ClassWithTypeConst2::class, 'TBarShouldNotBeTraversed');

    return 3;
  }
}
