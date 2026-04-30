//// foo.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
type TShape = shape('x' => int);
type TInt = int;
type TClassAlias = Exception;
class Foo {}

//// bar.php
<?hh
// package pkg1

// Free-function and property signatures: params, returns, instance/static
// properties. Plus parameter modifiers (inout / readonly / variadic) and
// nullable wrappers, which all share the same signature-level enforcement.

// Enforceable positions with non-class type aliases: should error in v2.strict
function param_shape(TShape $_): void {}
function param_int(TInt $_): void {}
function return_shape(): TShape { return shape('x' => 1); }
function return_int(): TInt { return 42; }

class Bar {
  public TShape $prop_shape = shape('x' => 1);
  public TInt $prop_int = 42;
}

// Enforceable positions with class type aliases: should NOT error
function param_class_alias(TClassAlias $_): void {}
function param_class(Foo $_): void {}
function return_class_alias(): TClassAlias { throw new Exception(); }
function return_class(): Foo { return new Foo(); }

class Baz {
  public TClassAlias $prop_class_alias;
  public Foo $prop_class;
  public function __construct() {
    $this->prop_class_alias = new Exception();
    $this->prop_class = new Foo();
  }
}

// Parameter modifiers: inout and readonly are enforced; variadic is NOT
// enforced at runtime, so variadic params should not trigger.
function inout_param(inout TShape $_): void {}
function readonly_param(readonly TShape $_): void {}
function variadic_param(TShape ...$_): void {}

// Nullable `?T`: still enforced as the same type at runtime, so should error
// when used at an enforceable position.
function nullable_param_shape(?TShape $_): void {}
function nullable_return_shape(): ?TShape { return null; }
