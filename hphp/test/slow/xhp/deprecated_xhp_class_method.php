<?hh // strict

class :x:composable-element {
  <<__Deprecated('message')>>
  public function foo() {}
}

class A_B__C {
  <<__Deprecated('another message')>>
  public function bar() {}
}

$x = <x:composable-element />;
$x->foo();
$c = new A_B__C();
$c->bar();
