<?hh // strict

class A {
  public coroutine function number(): int {
    return 1;
  }
}

class B extends A {
  public function run(): int {
    // not ok - call to coroutine outside of suspend
    return $this->number();
  }
}
