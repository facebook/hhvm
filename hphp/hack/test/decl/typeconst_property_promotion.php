<?hh

class C {
  const type T = int;
  public function __construct(
    private this::T $a,
  ): void {
  }
}

// Using `this::T` with parameter promotion requires the decl parser to clone
// the representation of `this::T` between the property and the parameter.
// Because we use internal mutability in the TypeconstAccess node, it is
// tempting to use `RefCell::replace` to take the contents of our typeconst
// names Vec when converting to a type, but it is incorrect to do so because of
// this clone.
