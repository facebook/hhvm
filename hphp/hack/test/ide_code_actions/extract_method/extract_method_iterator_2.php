<?hh

class Klass {
  public function foo(): KeyedIterator<int, string> {
    /*range-start*/
    yield 2 => "";
    /*range-end*/
  }
}
