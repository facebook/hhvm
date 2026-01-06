<?hh

class F1_TypeStructure1 {
  public static function makesTypeStruture1(): int {
    $ts = type_structure(F1_ClassWithTypeConst1::class, 'TBar');

    return 3;
  }

  // Caling this function should not get our test selected.
  public static function makesTypeStruture2(): int {
    $ts =
      type_structure(F1_ClassWithTypeConst1::class, 'TBarShouldNotBeTraversed');

    return 3;
  }
}
