<?hh // strict

class A {
  protected const int NUM_ARGS = 5;
}
class B extends A {
  public function get_num_args():int{
    return A::NUM_ARGS;        // ok
  }
}
class C {
  public function get_num_args():int{
    return A::NUM_ARGS;        // illegal
  }
}
