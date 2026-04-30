//// foo.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
type TShape = shape('x' => int);
type TInt = int;

//// bar.php
<?hh
// package pkg1

// `__RequirePackage("pkg2")` grants the annotated function/method
// temporary access to symbols defined in pkg2. References to non-class
// type aliases at enforceable positions inside such a function should NOT
// fire `Enforceable_type_alias`.

// Free functions
<<__RequirePackage("pkg2")>>
function require_param(TShape $_): void {}

<<__RequirePackage("pkg2")>>
function require_return(): TInt {
  return 0;
}

<<__RequirePackage("pkg2")>>
function require_generic_as<T as TShape>(T $_): void {}

// Methods (instance + static): the wellformedness pass applies
// per-method __RequirePackage before checking signatures, so these are
// allowed for the same reason.
class Bar {
  <<__RequirePackage("pkg2")>>
  public function method_param(TShape $_): TInt {
    return 0;
  }

  <<__RequirePackage("pkg2")>>
  public static function static_method_generic_as<T as TInt>(T $_): void {}
}
