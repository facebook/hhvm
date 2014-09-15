<?hh

class A<T as int> {
  public function foo(T $a): int {
    return $a + 10;
  }
}
