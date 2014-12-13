<?hh // strict

class Parent {
  protected type const ID = arraykey;
}

class Child extends Parent {
  protected type const ID = int;
}
