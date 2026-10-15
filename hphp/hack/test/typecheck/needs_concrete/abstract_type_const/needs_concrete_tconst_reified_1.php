<?hh

interface IHasAccessors {
  abstract const type TAccessors;
}

trait THasAccessors implements IHasAccessors {
  public static function getAccessor(): mixed {
    // error: abstract type constant resolved through `this` in a plain static
    // method, where `static` might refer to an abstract class at runtime
    return HH\ReifiedGenerics\get_class_from_type<this::TAccessors>();
  }

  <<__NeedsConcrete>>
  public static function getAccessorNeedsConcrete(): mixed {
    // ok: `static` points to a concrete class in a `__NeedsConcrete` method
    return HH\ReifiedGenerics\get_class_from_type<this::TAccessors>();
  }

  public function getAccessorInstance(): mixed {
    // ok: `static` points to a concrete class in an instance method
    return HH\ReifiedGenerics\get_class_from_type<this::TAccessors>();
  }
}
