<?hh

class F1_TypeStructureForAlias {
  public static function makeTypeStructure(): TypeStructure<F1_TypeAlias1> {
    return type_structure_for_alias(static::makeName());
  }

  public static function makeName(): typename<F1_TypeAlias1> {
    return nameof F1_TypeAlias1;
  }

}
