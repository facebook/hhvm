//// foo.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
type TShape = shape('x' => int);
type TInt = int;
type TClassAlias = Exception;
class Foo {}
class Box<T> { public function __construct(public T $v) {} }

//// bar.php
<?hh
// package pkg1

// Generic upper bound constraints, type aliases nested inside generics, and
// generic-wrapped variants. HHVM enforces only the outer generic class /
// builtin container — inner type arguments are not observed by name.

// Generic upper bound constraints: non-class aliases should error in v2.strict
function generic_as_shape<T as TShape>(T $_): void {}
function generic_as_int<T as TInt>(T $_): void {}

// Generic upper bound constraints: class aliases should NOT error
function generic_as_class_alias<T as TClassAlias>(T $_): void {}
function generic_as_class<T as Foo>(T $_): void {}

// Type aliases nested inside generics at enforceable positions: should NOT
// produce enforceable_type_alias errors (outer generic is what HHVM enforces)
function param_vec_of_shape(vec<TShape> $_): void {}
function return_vec_of_shape(): vec<TShape> { return vec[]; }
function param_dict_of_int(dict<string, TInt> $_): void {}
function return_dict_of_int(): dict<string, TInt> { return dict[]; }
function return_shape_of_shape(): shape('x' => TShape) {
  return shape('x' => shape('x' => 1));
}
function param_shape_of_int(shape('x' => TInt) $_): void {}

class Qux {
  public vec<TShape> $prop_vec_of_shape = vec[];
  public shape('x' => TInt) $prop_shape_of_int = shape('x' => 0);
}

// Tuples: HHVM enforces the outer tuple as a vec of the right arity, but does
// NOT enforce the element types. So tuple elements should NOT trigger.
function tuple_param((TShape, TInt) $_): void {}
function tuple_return(): (TShape, TInt) { return tuple(shape('x' => 1), 0); }

// Plain (non-container) generic class: HHVM enforces only the outer class
// `Box`, not the type argument. Inner `TShape` should NOT trigger.
function box_param(Box<TShape> $_): void {}
function box_return(): Box<TShape> { return new Box(shape('x' => 1)); }
class WithBoxProp {
  public ?Box<TShape> $box_of_shape = null;
}
