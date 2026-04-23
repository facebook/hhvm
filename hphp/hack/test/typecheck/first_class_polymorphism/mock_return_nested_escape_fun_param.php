<?hh

// Regression test for find_first_fun_param traversal bug.
// The type parameter T appears nested inside a function parameter type
// in the return type: (function(Box<T>): void).
// check_rank sees Tvar(Tr) <: (function(Box<T>): void) and must recurse
// into the function's parameter types via find_first_fun_param to find T
// inside Box<T>.

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

function get_callback<T>(T $_x): (function(Box<T>): void) {
  throw new Exception();
}

function test_nested_escape_in_fun_param(): void {
  Mock::mockFunction(get_callback<>)->mockReturn(($_x) ==> {});
}
