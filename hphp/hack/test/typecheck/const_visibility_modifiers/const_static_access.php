<?hh // strict

class A {
  private const int MAX_ARGS = 5;
  public function get_max_args():int{
    return static::MAX_ARGS;
  }
}

class B extends A {
  private const int MAX_ARGS = 6;
}
