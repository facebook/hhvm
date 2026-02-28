<?hh

class :x:composable-element {
  <<__Deprecated('message')>>
  public function foo() :mixed{}
}

class A_B__C {
  <<__Deprecated('another message')>>
  public function bar() :mixed{}
}


<<__EntryPoint>>
function main_deprecated_xhp_class_method() :mixed{
$x = <x:composable-element />;
$x->foo();
$c = new A_B__C();
$c->bar();
}
