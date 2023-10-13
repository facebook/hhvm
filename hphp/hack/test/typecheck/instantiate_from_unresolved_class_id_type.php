<?hh
<<__ConsistentConstruct>>
abstract class AbstractBaseClass {
  abstract const type T;
  public function __construct(private this::T $req) {}
}

abstract class InstantiateTypeConstantFromClassname {
  abstract const type TConstType as AbstractBaseClass;
  public function instantiate(this::TConstType::T $request): this::TConstType {
    $ts = type_structure(static::class, 'TConstType');
    // Class is now a Tvar of Tunion.
    $class = getClassname("dummy", $ts['classname']);
    // This should not be a dynamic class.
    $instance = new $class($request);
    return $instance;
  }
}

function getClassname<T>(mixed $x, classname<T> $y): classname<T> {
  return $y;
}
