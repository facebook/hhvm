//// foo.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
type TShape = shape('x' => int);
type TInt = int;
type TClassAlias = Exception;

//// bar.php
<?hh
// package pkg1

// Methods, static methods, constructors with promoted params, static
// properties, abstract methods, and interface methods. The first three are
// enforced at runtime; abstract method signatures are enforced at the
// concrete override; interface method signatures are not (only the
// implementor's method is invoked).

// Methods (instance + static), constructor with promoted params, static props,
// method generic upper bounds: all enforceable, non-class aliases should error.
class WithMethods {
  public static TShape $static_prop_shape = shape('x' => 1);
  public static TInt $static_prop_int = 0;

  public function __construct(public TShape $promoted_shape, public TInt $promoted_int) {}

  public function instance_method(TShape $_): TInt { return 0; }
  public static function static_method(TShape $_): TInt { return 0; }

  public function method_generic_as_shape<T as TShape>(T $_): void {}
  public function method_generic_as_int<T as TInt>(T $_): void {}
  // Class alias in method generic: should NOT error
  public function method_generic_as_class<T as TClassAlias>(T $_): void {}
}

// Abstract method signatures are enforceable: the concrete override inherits
// the hint and is invoked at runtime.
abstract class WithAbstract {
  abstract public function abstract_method(TShape $_): TInt;
}

// Interface method signatures are NOT directly invoked (only the implementor's
// method is), so type aliases in interface signatures should NOT error.
interface WithInterface {
  public function interface_method(TShape $_): TInt;
  public function interface_generic<T as TShape>(T $_): void;
}
