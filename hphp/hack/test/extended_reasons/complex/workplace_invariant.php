<?hh

interface bar<T as num> {
  public function get(): T;
}


final class foo implements bar<int> {
  public function get(): int {
    return 1;
  }
}


function getClass<Tu as num>(): classname<bar<Tu>> {
    return foo::class;
}
