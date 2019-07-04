<?hh // strict

class A {
  private const int MAX_ARGS = 5;
}

final class B extends A {
  private const int MAX_ARGS = 6;
  public function get_max_args():int{
    return static::MAX_ARGS;  // ok
  }
}
