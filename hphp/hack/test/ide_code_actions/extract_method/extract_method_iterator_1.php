<?hh

class Klass {
  public function foo(): Iterator<int> {
    /*range-start*/
    yield 2;
    /*range-end*/
  }
}
