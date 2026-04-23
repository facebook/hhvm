<?hh

// Regression test for find_locl_ty traversal bug.
// The type parameter T appears nested TWO levels deep in the return type:
// Awaitable<Box<T>>. The traversal must recurse into type arguments
// (via find_first_ty) to detect the escaping higher-rank type parameter.

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

// Return type Awaitable<Box<T>> nests T two levels deep.
// check_rank sees (Tvar(Tr) <: Awaitable<Box<T>>)
// and must find T inside Awaitable's type arg list element Box<T>.
function gen_make_box<T>(T $value): Awaitable<Box<T>> {
  throw new Exception();
}

function test_nested_escape(): void {
  Mock::mockFunction(gen_make_box<>)->mockReturn(new Box());
}
