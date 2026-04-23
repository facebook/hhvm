<?hh

<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

// Regression test for find_first_constraint traversal bug.
// The type parameter T appears nested inside a type parameter constraint
// in the return type: (function<U as Box<T>>(U): void).
// check_rank sees Tvar(Tr) <: (function<U as Box<T>>(U): void) and must
// recurse into the function's tparam constraints via find_first_constraint
// to find T inside the bound Box<T>.

class Box<T> {}

class Mock<Tfun> {

  private function __construct(private HH\FunctionRef<Tfun> $fn): void {}

  final public static function mockFunction(HH\FunctionRef<Tfun> $fn): Mock<Tfun> {
    return new Mock($fn);
  }

  public function mockReturn<Tr>(Tr $v): this
  where
    Tfun super (readonly function(mixed...)[]: Tr) {
    return $this;
  }
}

function get_constrained<T>(): (function<U as Box<T>>(U): void) {
  throw new Exception();
}

function test_nested_escape_in_constraint(): void {
  Mock::mockFunction(get_constrained<>)->mockReturn(($_x) ==> {});
}
