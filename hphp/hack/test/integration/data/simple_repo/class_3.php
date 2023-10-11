<?hh

class UsesA {
  public function test() : int {
    return A::foo();
  }

}
