<?hh // strict

class A {
  protected const int MAX_ARGS = 5;
}

class B extends A{
  public function get_int():int{
    return parent::MAX_ARGS; // ok
  }
}
